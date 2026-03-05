#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include "node_4d.h"

using namespace godot;

class RayCast4D : public Node4D {
	GDCLASS(RayCast4D, Node4D);

public:
	RayCast4D();
	~RayCast4D();

	// ─── Properties ───────────────────────────────────────────────────────────

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void set_target_position(const Vector4 &p_pos);
	Vector4 get_target_position() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_exclude_parent(bool p_exclude);
	bool get_exclude_parent() const;

	void set_hit_from_inside(bool p_hit);
	bool get_hit_from_inside() const;

	void set_collide_with_areas(bool p_enable);
	bool is_collide_with_areas_enabled() const;

	void set_collide_with_bodies(bool p_enable);
	bool is_collide_with_bodies_enabled() const;

	// ─── Query Results ────────────────────────────────────────────────────────

	bool is_colliding() const;
	Object *get_collider() const;
	Vector4 get_collision_point() const;
	Vector4 get_collision_normal() const;

	// ─── Control ──────────────────────────────────────────────────────────────

	void force_raycast_update();

	void add_exception(Object *p_obj);
	void remove_exception(Object *p_obj);
	void clear_exceptions();

	void _notification(int p_what);
	void _physics_process(double p_delta);

protected:
	static void _bind_methods();

private:
	bool _enabled = true;
	Vector4 _target_position = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	uint32_t _collision_mask = 1;
	bool _exclude_parent = true;
	bool _hit_from_inside = false;
	bool _collide_with_areas = false;
	bool _collide_with_bodies = true;

	// Cached result from last raycast
	bool _is_colliding = false;
	Vector4 _collision_point;
	Vector4 _collision_normal;
	uint64_t _collider_id = 0;
	Object *_collider = nullptr;

	// RIDs excluded from the ray query
	TypedArray<RID> _exclude_rids;

	void _perform_raycast();
};
