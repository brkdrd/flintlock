#pragma once

#include "collision_object_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>

using namespace godot;

/// Area4D — A 4D area for overlap detection and spatial queries.
///
/// Detects when other physics bodies enter or exit its space.
/// Inherits from CollisionObject4D.
class Area4D : public CollisionObject4D {
	GDCLASS(Area4D, CollisionObject4D)

private:
	real_t gravity = 0.0;
	Vector4 gravity_direction = Vector4(0, 0, 0, -1);
	bool monitoring = true;
	bool monitorable = true;

public:
	Area4D();
	~Area4D();

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

	// -- Godot lifecycle -----------------------------------------------------

	void _ready() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();
};
