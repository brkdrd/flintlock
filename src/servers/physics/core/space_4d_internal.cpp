#include "space_4d_internal.h"
#include "gjk_epa_4d.h"
#include "constraint_solver_4d.h"
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/variant.hpp>
#include <algorithm>
#include <set>

using namespace godot;

// ============================================================================
// Space4DInternal - Full physics pipeline implementation
// ============================================================================

void Space4DInternal::step(float dt) {
	last_dt = dt;

	// 1. Integrate forces (gravity + applied forces -> velocity changes)
	integrate_forces(dt);

	// 2. Update broadphase AABBs
	update_broadphase();

	// 3. Narrowphase collision detection
	detect_collisions();

	// 4. Solve contact + joint constraints
	solve_constraints(dt);

	// 5. Integrate velocities (velocity -> position/rotation changes)
	integrate_velocities(dt);

	// 6. Update sleep states
	update_sleep_states(dt);

	// 7. Area overlap detection
	detect_area_overlaps();

	// 8. Fire callbacks (state sync, area monitors)
	fire_callbacks();
}

void Space4DInternal::integrate_forces(float dt) {
	for (auto &body : bodies) {
		if (!body.is_dynamic() || body.sleeping) continue;
		Vector4 gravity = compute_gravity_for_body(body);
		body.integrate_forces(dt, gravity);
	}
}

void Space4DInternal::update_broadphase() {
	for (auto &body : bodies) {
		if (body.shapes.empty()) continue;
		AABB4 aabb = body.get_world_aabb();
		if (body.broadphase_proxy < 0) {
			body.broadphase_proxy = broadphase.insert(aabb, body.id);
		} else {
			broadphase.update(body.broadphase_proxy, aabb);
		}
	}
	for (auto &area : areas) {
		if (area.shapes.empty()) continue;
		AABB4 aabb = area.get_world_aabb();
		if (area.broadphase_proxy < 0) {
			// Use negative IDs for areas to distinguish from bodies
			area.broadphase_proxy = broadphase.insert(aabb, -(area.id + 1));
		} else {
			broadphase.update(area.broadphase_proxy, aabb);
		}
	}
}

void Space4DInternal::detect_collisions() {
	// Get broadphase pairs
	std::vector<std::pair<int, int>> broad_pairs;
	broadphase.query_pairs(broad_pairs);

	// Remove duplicate pairs
	std::set<std::pair<int, int>> unique_pairs(broad_pairs.begin(), broad_pairs.end());

	// Clear old manifolds (but preserve warm-start data)
	std::vector<ContactManifold4D> old_manifolds = std::move(contact_manifolds);
	contact_manifolds.clear();

	for (const auto &pair : unique_pairs) {
		int id_a = pair.first;
		int id_b = pair.second;

		// Skip area-area pairs for collision (handled in detect_area_overlaps)
		if (id_a < 0 && id_b < 0) continue;
		// Skip area-body pairs for collision (handled in detect_area_overlaps)
		if (id_a < 0 || id_b < 0) continue;

		// Find the bodies
		auto it_a = body_index_map.find(id_a);
		auto it_b = body_index_map.find(id_b);
		if (it_a == body_index_map.end() || it_b == body_index_map.end()) continue;

		int idx_a = it_a->second;
		int idx_b = it_b->second;
		RigidBody4DInternal &body_a = bodies[idx_a];
		RigidBody4DInternal &body_b = bodies[idx_b];

		// Skip if both static/kinematic
		if (!body_a.is_dynamic() && !body_b.is_dynamic()) continue;

		// Skip if both sleeping
		if (body_a.sleeping && body_b.sleeping) continue;

		// Collision layer/mask filter
		if (!(body_a.collision_layer & body_b.collision_mask) &&
			!(body_b.collision_layer & body_a.collision_mask)) continue;

		// Check joint exclusions
		bool excluded = false;
		for (const auto &joint : joints) {
			if (!joint->exclude_collision) continue;
			if ((joint->body_a_index == idx_a && joint->body_b_index == idx_b) ||
				(joint->body_a_index == idx_b && joint->body_b_index == idx_a)) {
				excluded = true;
				break;
			}
		}
		if (excluded) continue;

		// Narrowphase: test each shape pair
		for (int sa = 0; sa < (int)body_a.shapes.size(); sa++) {
			if (body_a.shapes[sa].disabled || !body_a.shapes[sa].shape) continue;

			for (int sb = 0; sb < (int)body_b.shapes.size(); sb++) {
				if (body_b.shapes[sb].disabled || !body_b.shapes[sb].shape) continue;

				Xform4 xf_a = body_a.transform * body_a.shapes[sa].local_transform;
				Xform4 xf_b = body_b.transform * body_b.shapes[sb].local_transform;

				CollisionResult collision = collide_shapes_4d(
					body_a.shapes[sa].shape.get(), xf_a,
					body_b.shapes[sb].shape.get(), xf_b
				);

				if (collision.colliding && collision.depth > 0) {
					// Wake both bodies
					body_a.wake_up();
					body_b.wake_up();

					// Find or create manifold
					ContactManifold4D manifold;
					manifold.body_a_index = idx_a;
					manifold.body_b_index = idx_b;
					manifold.shape_a_index = sa;
					manifold.shape_b_index = sb;

					// Check for existing manifold to preserve warm-start data
					for (const auto &old_m : old_manifolds) {
						if (old_m.body_a_index == idx_a && old_m.body_b_index == idx_b &&
							old_m.shape_a_index == sa && old_m.shape_b_index == sb) {
							manifold = old_m;
							manifold.update(body_a.transform, body_b.transform);
							break;
						}
					}

					ContactPoint4D cp;
					cp.normal = collision.normal;
					cp.depth = collision.depth;
					cp.position_on_a = collision.point_on_a;
					cp.position_on_b = collision.point_on_b;
					cp.local_a = body_a.transform.xform_inv(collision.point_on_a);
					cp.local_b = body_b.transform.xform_inv(collision.point_on_b);

					manifold.add_point(cp);
					contact_manifolds.push_back(manifold);
				}
			}
		}
	}

	// Update contact counts on bodies
	for (auto &body : bodies) body.contact_count = 0;
	for (const auto &m : contact_manifolds) {
		bodies[m.body_a_index].contact_count += m.point_count;
		bodies[m.body_b_index].contact_count += m.point_count;
	}
}

void Space4DInternal::solve_constraints(float dt) {
	// Solve contacts
	solve_contacts_4d(contact_manifolds, bodies, dt);

	// Solve joints
	if (!joints.empty()) {
		solve_joints_4d(joints, bodies, dt, solver_iterations);
	}
}

void Space4DInternal::integrate_velocities(float dt) {
	for (auto &body : bodies) {
		body.integrate_velocity(dt);
	}
}

void Space4DInternal::update_sleep_states(float dt) {
	for (auto &body : bodies) {
		body.update_sleep(dt);
	}
}

void Space4DInternal::detect_area_overlaps() {
	for (auto &area : areas) {
		if (!area.monitoring || area.shapes.empty()) continue;

		// Query broadphase for overlaps
		AABB4 area_aabb = area.get_world_aabb();
		std::vector<int> overlaps;
		broadphase.query_aabb(area_aabb, overlaps);

		std::set<int> current_bodies;
		std::set<int> current_areas;

		for (int overlap_id : overlaps) {
			if (overlap_id >= 0) {
				// It's a body
				auto it = body_index_map.find(overlap_id);
				if (it == body_index_map.end()) continue;
				int body_idx = it->second;

				// Collision layer/mask check
				RigidBody4DInternal &body = bodies[body_idx];
				if (!(area.collision_mask & body.collision_layer)) continue;

				// Narrowphase check
				bool touching = false;
				for (const auto &area_shape : area.shapes) {
					if (area_shape.disabled || !area_shape.shape) continue;
					for (const auto &body_shape : body.shapes) {
						if (body_shape.disabled || !body_shape.shape) continue;
						Xform4 xf_a = area.transform * area_shape.local_transform;
						Xform4 xf_b = body.transform * body_shape.local_transform;
						CollisionResult cr = collide_shapes_4d(
							area_shape.shape.get(), xf_a,
							body_shape.shape.get(), xf_b
						);
						if (cr.colliding) {
							touching = true;
							break;
						}
					}
					if (touching) break;
				}

				if (touching) {
					current_bodies.insert(body_idx);
				}
			} else {
				// It's another area (overlap_id = -(area_id + 1))
				int other_area_id = -(overlap_id + 1);
				auto it = area_index_map.find(other_area_id);
				if (it == area_index_map.end()) continue;
				int other_idx = it->second;
				if (other_idx == area.id) continue; // Don't self-overlap

				// Narrowphase check (simplified - just AABB for areas)
				Area4DInternal &other = areas[other_idx];
				if (!other.monitorable) continue;
				AABB4 other_aabb = other.get_world_aabb();
				if (area_aabb.intersects(other_aabb)) {
					current_areas.insert(other_idx);
				}
			}
		}

		// Detect enter/exit events for bodies
		for (int bi : current_bodies) {
			if (area.overlapping_bodies.find(bi) == area.overlapping_bodies.end()) {
				// New overlap - body entered
				if (area.body_monitor_callback.is_valid()) {
					area.body_monitor_callback.call(1, RID(), (int64_t)bodies[bi].object_instance_id, 0, 0);
				}
			}
		}
		for (int bi : area.overlapping_bodies) {
			if (current_bodies.find(bi) == current_bodies.end()) {
				// Lost overlap - body exited
				if (area.body_monitor_callback.is_valid()) {
					area.body_monitor_callback.call(0, RID(), (int64_t)bodies[bi].object_instance_id, 0, 0);
				}
			}
		}
		area.overlapping_bodies = current_bodies;

		// Detect enter/exit events for areas
		for (int ai : current_areas) {
			if (area.overlapping_areas.find(ai) == area.overlapping_areas.end()) {
				if (area.area_monitor_callback.is_valid()) {
					area.area_monitor_callback.call(1, RID(), (int64_t)areas[ai].object_instance_id, 0, 0);
				}
			}
		}
		for (int ai : area.overlapping_areas) {
			if (current_areas.find(ai) == current_areas.end()) {
				if (area.area_monitor_callback.is_valid()) {
					area.area_monitor_callback.call(0, RID(), (int64_t)areas[ai].object_instance_id, 0, 0);
				}
			}
		}
		area.overlapping_areas = current_areas;
	}
}

void Space4DInternal::fire_callbacks() {
	for (auto &body : bodies) {
		if (!body.is_dynamic()) continue;
		if (!body.state_sync_callback.is_valid()) continue;
		// The callback will be fired from the PhysicsServer4D level
		// since it needs to create PhysicsDirectBodyState4D objects.
	}
}

Vector4 Space4DInternal::compute_gravity_for_body(const RigidBody4DInternal &body) const {
	Vector4 gravity = gravity_direction * gravity_magnitude;

	// Check area overrides
	for (const auto &area : areas) {
		if (area.gravity_override_mode == 0) continue; // Disabled

		// Check if body overlaps this area
		if (area.overlapping_bodies.find(body.id) == area.overlapping_bodies.end()) continue;

		Vector4 area_gravity = area.compute_gravity(body.transform.origin);

		if (area.gravity_override_mode == 2) {
			// Replace
			gravity = area_gravity;
		} else {
			// Combine
			gravity += area_gravity;
		}
	}

	return gravity;
}

// ============================================================================
// Ray casting
// ============================================================================

Space4DInternal::RayResult Space4DInternal::cast_ray(
	const Vector4 &from, const Vector4 &to,
	uint32_t collision_mask, bool collide_bodies, bool collide_areas,
	const std::vector<int> &exclude_bodies
) const {
	RayResult result;
	Vector4 dir = to - from;
	float ray_length = dir.length();
	if (ray_length < PHYSICS_EPSILON) return result;
	dir /= ray_length;

	float best_t = ray_length;

	auto check_hit = [&](int user_data, float t_min_aabb, float t_max_aabb) -> bool {
		if (t_min_aabb > best_t) return true; // Can't improve

		if (user_data >= 0 && collide_bodies) {
			auto it = body_index_map.find(user_data);
			if (it == body_index_map.end()) return true;
			int idx = it->second;
			const RigidBody4DInternal &body = bodies[idx];

			// Check exclusion
			for (int ex : exclude_bodies) {
				if (ex == idx) return true;
			}

			// Check layer/mask
			if (!(body.collision_layer & collision_mask)) return true;

			// Test each shape
			for (int si = 0; si < (int)body.shapes.size(); si++) {
				const auto &shape_inst = body.shapes[si];
				if (shape_inst.disabled || !shape_inst.shape) continue;

				Xform4 xf = body.transform * shape_inst.local_transform;
				RayCastResult4D rc = ray_cast_shape_4d(from, dir, ray_length, shape_inst.shape.get(), xf);
				if (rc.hit) {
					float t = rc.t * ray_length;
					if (t < best_t) {
						best_t = t;
						result.hit = true;
						result.point = rc.point;
						result.normal = rc.normal;
						result.body_index = idx;
						result.shape_index = si;
						result.t = t / ray_length;
					}
				}
			}
		} else if (user_data < 0 && collide_areas) {
			int area_id = -(user_data + 1);
			auto it = area_index_map.find(area_id);
			if (it == area_index_map.end()) return true;
			int idx = it->second;
			const Area4DInternal &area = areas[idx];

			if (!(area.collision_layer & collision_mask)) return true;

			for (int si = 0; si < (int)area.shapes.size(); si++) {
				const auto &shape_inst = area.shapes[si];
				if (shape_inst.disabled || !shape_inst.shape) continue;

				Xform4 xf = area.transform * shape_inst.local_transform;
				RayCastResult4D rc = ray_cast_shape_4d(from, dir, ray_length, shape_inst.shape.get(), xf);
				if (rc.hit) {
					float t = rc.t * ray_length;
					if (t < best_t) {
						best_t = t;
						result.hit = true;
						result.point = rc.point;
						result.normal = rc.normal;
						result.body_index = -(idx + 1); // Negative for area
						result.shape_index = si;
						result.t = t / ray_length;
					}
				}
			}
		}
		return true;
	};

	const_cast<BroadPhase4D&>(broadphase).query_ray(from, dir, ray_length, check_hit);

	// If broadphase is empty, check all bodies directly
	if (broadphase.get_root() == BVH_NULL_NODE) {
		if (collide_bodies) {
			for (int i = 0; i < (int)bodies.size(); i++) {
				check_hit(bodies[i].id, 0, ray_length);
			}
		}
	}

	return result;
}

// ============================================================================
// Point query
// ============================================================================

std::vector<Space4DInternal::PointResult> Space4DInternal::query_point(
	const Vector4 &point, int max_results,
	uint32_t collision_mask, bool collide_bodies, bool collide_areas
) const {
	std::vector<PointResult> results;

	auto check = [&](const std::vector<ShapeInstance4D> &shapes, const Xform4 &body_xf,
					  uint32_t layer, int index, uint64_t obj_id, bool is_body) {
		if (!(layer & collision_mask)) return;
		for (int si = 0; si < (int)shapes.size(); si++) {
			if (shapes[si].disabled || !shapes[si].shape) continue;
			Xform4 xf = body_xf * shapes[si].local_transform;
			if (point_in_shape_4d(point, shapes[si].shape.get(), xf)) {
				PointResult pr;
				pr.body_index = index;
				pr.shape_index = si;
				pr.object_instance_id = obj_id;
				results.push_back(pr);
				if ((int)results.size() >= max_results) return;
			}
		}
	};

	if (collide_bodies) {
		for (int i = 0; i < (int)bodies.size(); i++) {
			if ((int)results.size() >= max_results) break;
			check(bodies[i].shapes, bodies[i].transform,
				  bodies[i].collision_layer, i, bodies[i].object_instance_id, true);
		}
	}

	if (collide_areas) {
		for (int i = 0; i < (int)areas.size(); i++) {
			if ((int)results.size() >= max_results) break;
			check(areas[i].shapes, areas[i].transform,
				  areas[i].collision_layer, -(i+1), areas[i].object_instance_id, false);
		}
	}

	return results;
}

// ============================================================================
// Shape cast (for move_and_slide)
// ============================================================================

Space4DInternal::ShapeCastResult Space4DInternal::cast_shape(
	const Shape4DInternal *shape, const Xform4 &from,
	const Vector4 &motion, float margin, uint32_t collision_mask,
	const std::vector<int> &exclude
) const {
	ShapeCastResult result;
	result.safe_fraction = 1.0f;
	result.unsafe_fraction = 1.0f;

	if (motion.length_squared() < PHYSICS_EPSILON * PHYSICS_EPSILON) return result;

	float motion_length = motion.length();
	Vector4 motion_dir = motion / motion_length;

	// Binary search along the motion for earliest collision
	float lo = 0.0f, hi = 1.0f;
	bool found = false;

	for (int iter = 0; iter < 16; iter++) {
		float mid = (lo + hi) * 0.5f;
		Xform4 test_xf = from;
		test_xf.origin += motion * mid;

		bool colliding = false;

		for (int bi = 0; bi < (int)bodies.size(); bi++) {
			bool skip = false;
			for (int ex : exclude) if (ex == bi) { skip = true; break; }
			if (skip) continue;

			const RigidBody4DInternal &body = bodies[bi];
			if (!(body.collision_layer & collision_mask)) continue;

			for (const auto &bs : body.shapes) {
				if (bs.disabled || !bs.shape) continue;
				Xform4 body_shape_xf = body.transform * bs.local_transform;

				CollisionResult cr = collide_shapes_4d(shape, test_xf, bs.shape.get(), body_shape_xf);
				if (cr.colliding && cr.depth > margin * 0.1f) {
					colliding = true;
					result.collision_point = cr.point_on_b;
					result.collision_normal = cr.normal;
					result.collider_index = bi;
					break;
				}
			}
			if (colliding) break;
		}

		if (colliding) {
			hi = mid;
			found = true;
		} else {
			lo = mid;
		}
	}

	if (found) {
		result.colliding = true;
		result.safe_fraction = lo;
		result.unsafe_fraction = hi;
	}

	return result;
}

// ============================================================================
// Body motion test
// ============================================================================

Space4DInternal::MotionResult Space4DInternal::test_body_motion(
	int body_index, const Vector4 &motion, float margin
) const {
	MotionResult result;
	result.travel = motion;

	if (body_index < 0 || body_index >= (int)bodies.size()) return result;
	const RigidBody4DInternal &body = bodies[body_index];
	if (body.shapes.empty()) return result;

	// Use the first shape for the motion test
	const auto &shape_inst = body.shapes[0];
	if (!shape_inst.shape) return result;

	Xform4 from = body.transform * shape_inst.local_transform;

	std::vector<int> exclude = {body_index};
	ShapeCastResult cast = cast_shape(shape_inst.shape.get(), from, motion, margin,
									   body.collision_mask, exclude);

	if (cast.colliding) {
		result.colliding = true;
		result.safe_fraction = cast.safe_fraction;
		result.travel = motion * cast.safe_fraction;
		result.remainder = motion * (1.0f - cast.safe_fraction);
		result.collision_point = cast.collision_point;
		result.collision_normal = cast.collision_normal;
		result.collider_index = cast.collider_index;

		// Compute depth
		Xform4 test_xf = from;
		test_xf.origin += motion * cast.unsafe_fraction;
		for (const auto &other_body : bodies) {
			if (other_body.id == body.id) continue;
			for (const auto &obs : other_body.shapes) {
				if (obs.disabled || !obs.shape) continue;
				Xform4 obs_xf = other_body.transform * obs.local_transform;
				CollisionResult cr = collide_shapes_4d(
					shape_inst.shape.get(), test_xf, obs.shape.get(), obs_xf);
				if (cr.colliding) {
					result.collision_depth = cr.depth;
					break;
				}
			}
		}
	}

	return result;
}
