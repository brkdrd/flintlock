#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/object_id.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include "node_4d.h"
#include "../servers/physics/physics_server_4d.h"

using namespace godot;

class CollisionObject4D : public Node4D {
	GDCLASS(CollisionObject4D, Node4D);

protected:
	RID _rid;
	uint32_t _collision_layer   = 1;
	uint32_t _collision_mask    = 1;
	float    _collision_priority = 1.0f;

	// Called by subclasses to create the actual physics object (body or area).
	// Implementations must store the created RID in _rid.
	virtual void _create_physics_body() = 0;

	// Build a 20-float PackedFloat32Array from the current global Transform4D.
	PackedFloat32Array _get_transform_array() const;

	void _notification(int p_what);
	static void _bind_methods();

public:
	CollisionObject4D();
	virtual ~CollisionObject4D();

	RID get_rid() const { return _rid; }

	// Collision layer / mask / priority
	void     set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void     set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void  set_collision_priority(float p_priority);
	float get_collision_priority() const;

	// Shape owner helpers (thin wrappers around the server's shape API)
	void shape_owner_add_shape(const RID &p_shape);
	int  shape_owner_get_count() const;
	void shape_owner_clear();
};
