#include "physics_direct_space_state_4d.h"
#include "physics_server_4d.h"
#include "core/space_4d_internal.h"
#include "core/rigid_body_4d_internal.h"
#include <godot_cpp/classes/object.hpp>

void PhysicsDirectSpaceState4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("intersect_ray", "from", "to", "exclude", "collision_mask", "collide_with_bodies", "collide_with_areas", "hit_from_inside", "hit_back_faces"),
		&PhysicsDirectSpaceState4D::intersect_ray,
		DEFVAL(TypedArray<RID>()), DEFVAL(0xFFFFFFFF), DEFVAL(true), DEFVAL(false), DEFVAL(false), DEFVAL(true));
	ClassDB::bind_method(D_METHOD("intersect_point", "point", "max_results", "exclude", "collision_mask", "collide_with_bodies", "collide_with_areas"),
		&PhysicsDirectSpaceState4D::intersect_point,
		DEFVAL(32), DEFVAL(TypedArray<RID>()), DEFVAL(0xFFFFFFFF), DEFVAL(true), DEFVAL(false));
}

Dictionary PhysicsDirectSpaceState4D::intersect_ray(const Vector4 &p_from, const Vector4 &p_to,
	const TypedArray<RID> &p_exclude,
	uint32_t p_collision_mask,
	bool p_collide_with_bodies,
	bool p_collide_with_areas,
	bool p_hit_from_inside,
	bool p_hit_back_faces) {

	if (!_server) return Dictionary();

	Space4DInternal *space = _server->get_space_internal(_space_rid);
	if (!space) return Dictionary();

	// Build exclude list (body internal IDs)
	std::vector<int> exclude_bodies;
	for (int i = 0; i < p_exclude.size(); i++) {
		RID rid = p_exclude[i];
		RigidBody4DInternal *body = _server->get_body_internal(rid);
		if (body) {
			exclude_bodies.push_back(body->id);
		}
	}

	Space4DInternal::RayResult result = space->cast_ray(
		p_from, p_to, p_collision_mask,
		p_collide_with_bodies, p_collide_with_areas,
		exclude_bodies
	);

	if (!result.hit) return Dictionary();

	Dictionary dict;
	dict["position"] = result.point;
	dict["normal"] = result.normal;
	dict["shape"] = result.shape_index;

	// Resolve the body index back to an object instance ID
	if (result.body_index >= 0 && result.body_index < (int)space->bodies.size()) {
		uint64_t oid = space->bodies[result.body_index].object_instance_id;
		if (oid != 0) {
			dict["collider_id"] = (int64_t)oid;
			Object *obj = ObjectDB::get_instance(ObjectID(oid));
			if (obj) {
				dict["collider"] = obj;
			}
		}
	}

	return dict;
}

Array PhysicsDirectSpaceState4D::intersect_point(const Vector4 &p_point,
	int p_max_results,
	const TypedArray<RID> &p_exclude,
	uint32_t p_collision_mask,
	bool p_collide_with_bodies,
	bool p_collide_with_areas) {

	if (!_server) return Array();

	Space4DInternal *space = _server->get_space_internal(_space_rid);
	if (!space) return Array();

	std::vector<Space4DInternal::PointResult> results = space->query_point(
		p_point, p_max_results, p_collision_mask,
		p_collide_with_bodies, p_collide_with_areas
	);

	Array arr;
	for (const auto &pr : results) {
		Dictionary dict;
		dict["shape"] = pr.shape_index;

		if (pr.body_index >= 0 && pr.body_index < (int)space->bodies.size()) {
			uint64_t oid = space->bodies[pr.body_index].object_instance_id;
			if (oid != 0) {
				dict["collider_id"] = (int64_t)oid;
				Object *obj = ObjectDB::get_instance(ObjectID(oid));
				if (obj) {
					dict["collider"] = obj;
				}
			}
		}

		arr.push_back(dict);
	}

	return arr;
}
