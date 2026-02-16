#pragma once

#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// PhysicsBody4D — Abstract base class for all physics-enabled 4D bodies.
///
/// Provides common functionality for collision layers, masks, and physics server interaction.
/// Similar to PhysicsBody2D and PhysicsBody3D in Godot.
class PhysicsBody4D : public Node4D {
	GDCLASS(PhysicsBody4D, Node4D)

protected:
	RID body_rid;
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;

public:
	PhysicsBody4D();
	virtual ~PhysicsBody4D();

	// -- Collision layers and masks ------------------------------------------

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_collision_layer_value(int p_layer_number, bool p_value);
	bool get_collision_layer_value(int p_layer_number) const;

	void set_collision_mask_value(int p_layer_number, bool p_value);
	bool get_collision_mask_value(int p_layer_number) const;

	// -- Physics server interaction ------------------------------------------

	RID get_rid() const;

	// -- Godot lifecycle -----------------------------------------------------

	virtual void _ready() override;
	virtual void _exit_tree() override;

protected:
	static void _bind_methods();
};
