#pragma once

#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// Area4D — A 4D area for overlap detection and spatial queries.
///
/// Detects when other physics bodies enter or exit its space.
/// Can be used for triggers, sensors, gravity wells, etc.
class Area4D : public Node4D {
	GDCLASS(Area4D, Node4D)

private:
	RID area_rid;
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;
	real_t gravity = 0.0;
	Vector4 gravity_direction = Vector4(0, 0, 0, -1);
	bool monitoring = true;
	bool monitorable = true;

public:
	Area4D();
	~Area4D();

	// -- Collision layers and masks ------------------------------------------

	void set_collision_layer(uint32_t p_layer);
	uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t p_mask);
	uint32_t get_collision_mask() const;

	void set_collision_layer_value(int p_layer_number, bool p_value);
	bool get_collision_layer_value(int p_layer_number) const;

	void set_collision_mask_value(int p_layer_number, bool p_value);
	bool get_collision_mask_value(int p_layer_number) const;

	// -- Gravity override ----------------------------------------------------

	void set_gravity(real_t p_gravity);
	real_t get_gravity() const;

	void set_gravity_direction(const Vector4 &p_direction);
	Vector4 get_gravity_direction() const;

	// -- Monitoring ----------------------------------------------------------

	void set_monitoring(bool p_enable);
	bool is_monitoring() const;

	void set_monitorable(bool p_enable);
	bool is_monitorable() const;

	// -- Physics server interaction ------------------------------------------

	RID get_rid() const;

	// -- Godot lifecycle -----------------------------------------------------

	void _ready() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();
};
