#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// RigidBody4D — A physics body in 4D space that responds to forces.
///
/// This is the Godot-facing wrapper around our internal physics implementation.
/// It integrates with Godot's scene tree and can be controlled from GDScript.
class RigidBody4D : public Node3D {
	GDCLASS(RigidBody4D, Node3D)

private:
	flintlock::PhysicsRID body_rid;
	real_t mass = 1.0;
	real_t linear_damp = 0.0;
	Vector4 linear_velocity = Vector4(0, 0, 0, 0);
	Vector4 position_4d = Vector4(0, 0, 0, 0);

public:
	RigidBody4D();
	~RigidBody4D();

	// Mass
	void set_mass(real_t p_mass);
	real_t get_mass() const;

	// Damping
	void set_linear_damp(real_t p_damp);
	real_t get_linear_damp() const;

	// Position in 4D
	void set_position_4d(const Vector4 &p_position);
	Vector4 get_position_4d() const;

	// Velocity
	void set_linear_velocity(const Vector4 &p_velocity);
	Vector4 get_linear_velocity() const;

	// Forces and impulses
	void apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position = Vector4());
	void apply_force(const Vector4 &p_force, const Vector4 &p_position = Vector4());

	// Godot lifecycle
	void _ready() override;
	void _physics_process(double p_delta) override;
	void _exit_tree() override;

	// Get the internal RID (for advanced use)
	flintlock::PhysicsRID get_rid() const;

protected:
	static void _bind_methods();
};
