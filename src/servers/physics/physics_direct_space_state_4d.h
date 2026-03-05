#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

class PhysicsServer4D;

// PhysicsDirectSpaceState4D - direct query interface for a physics space.
// Analog of PhysicsDirectSpaceState3D.
class PhysicsDirectSpaceState4D : public RefCounted {
	GDCLASS(PhysicsDirectSpaceState4D, RefCounted);

public:
	RID _space_rid;
	PhysicsServer4D *_server = nullptr;

protected:
	static void _bind_methods();

public:
	// Intersect a ray in 4D space
	// Returns dictionary with: position (Vector4), normal (Vector4), collider (Object),
	// collider_id (int), rid (RID), shape (int)
	Dictionary intersect_ray(const Vector4 &p_from, const Vector4 &p_to,
		const TypedArray<RID> &p_exclude = TypedArray<RID>(),
		uint32_t p_collision_mask = 0xFFFFFFFF,
		bool p_collide_with_bodies = true,
		bool p_collide_with_areas = false,
		bool p_hit_from_inside = false,
		bool p_hit_back_faces = true);

	// Intersect a point in 4D space
	Array intersect_point(const Vector4 &p_point,
		int p_max_results = 32,
		const TypedArray<RID> &p_exclude = TypedArray<RID>(),
		uint32_t p_collision_mask = 0xFFFFFFFF,
		bool p_collide_with_bodies = true,
		bool p_collide_with_areas = false);
};
