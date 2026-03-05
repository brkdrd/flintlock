#include "physics_direct_space_state_4d.h"

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
	// Stub: no collision detection implemented yet
	return Dictionary();
}

Array PhysicsDirectSpaceState4D::intersect_point(const Vector4 &p_point,
	int p_max_results,
	const TypedArray<RID> &p_exclude,
	uint32_t p_collision_mask,
	bool p_collide_with_bodies,
	bool p_collide_with_areas) {
	// Stub: no collision detection implemented yet
	return Array();
}
