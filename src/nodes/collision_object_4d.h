#pragma once

#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

/// CollisionObject4D — Base class for all collision-aware 4D nodes.
///
/// Mirrors CollisionObject3D: provides collision layers, masks, priority,
/// and shape owner management. Both PhysicsBody4D and Area4D inherit from this.
class CollisionObject4D : public Node4D {
	GDCLASS(CollisionObject4D, Node4D)

protected:
	RID base_rid;
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;
	real_t collision_priority = 1.0;

public:
	CollisionObject4D();
	virtual ~CollisionObject4D();

	// -- Physics server RID --------------------------------------------------

	virtual RID get_rid() const;

	// -- Collision layers and masks ------------------------------------------

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_collision_layer_value(int p_layer_number, bool p_value);
	bool get_collision_layer_value(int p_layer_number) const;

	void set_collision_mask_value(int p_layer_number, bool p_value);
	bool get_collision_mask_value(int p_layer_number) const;

	// -- Collision priority --------------------------------------------------

	void set_collision_priority(real_t p_priority);
	real_t get_collision_priority() const;

protected:
	static void _bind_methods();
};
