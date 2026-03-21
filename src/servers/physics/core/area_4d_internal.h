#pragma once

#include "physics_types_4d.h"
#include "shape_4d_internal.h"
#include <vector>
#include <memory>
#include <set>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

// ============================================================================
// Internal area representation for the 4D physics engine.
// Areas detect overlaps and can override gravity/damping.
// ============================================================================

struct Area4DInternal {
	int id = -1;
	uint64_t object_instance_id = 0;
	int space_index = -1;

	Xform4 transform;

	// Shapes
	std::vector<ShapeInstance4D> shapes;

	// Gravity override
	float gravity = 9.8f;
	Vector4 gravity_direction = Vector4(0, -1, 0, 0);
	bool gravity_is_point = false;
	float gravity_point_unit_distance = 1.0f;
	int gravity_override_mode = 0; // 0=disabled, 1=combine, 2=replace

	// Damping override
	float linear_damp = 0.0f;
	int linear_damp_override_mode = 0;
	float angular_damp = 0.0f;
	int angular_damp_override_mode = 0;

	int priority = 0;
	bool monitoring = true;
	bool monitorable = true;

	// Collision filtering
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;

	// Callbacks
	Callable body_monitor_callback;
	Callable area_monitor_callback;

	// Broadphase proxy
	int broadphase_proxy = -1;

	// Overlap tracking
	std::set<int> overlapping_bodies;  // body indices
	std::set<int> overlapping_areas;   // area indices

	AABB4 get_world_aabb() const {
		AABB4 result;
		bool first = true;
		for (const auto &si : shapes) {
			if (si.disabled || !si.shape) continue;
			Xform4 world_shape_xf = transform * si.local_transform;
			AABB4 shape_aabb = compute_world_aabb(si.shape.get(), world_shape_xf);
			if (first) {
				result = shape_aabb;
				first = false;
			} else {
				result = result.merged(shape_aabb);
			}
		}
		if (first) {
			result = AABB4(transform.origin, transform.origin);
		}
		return result;
	}

	Vector4 compute_gravity(const Vector4 &body_position) const {
		if (gravity_is_point) {
			Vector4 dir = transform.origin - body_position;
			float dist = dir.length();
			if (dist < PHYSICS_EPSILON) return Vector4(0, 0, 0, 0);
			dir /= dist;
			float factor = gravity;
			if (gravity_point_unit_distance > PHYSICS_EPSILON) {
				float ratio = gravity_point_unit_distance / dist;
				factor *= ratio * ratio;
			}
			return dir * factor;
		} else {
			return gravity_direction * gravity;
		}
	}
};
