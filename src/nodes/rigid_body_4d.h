#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "physics_body_4d.h"
#include "../servers/physics/physics_direct_body_state_4d.h"

using namespace godot;

class RigidBody4D : public PhysicsBody4D {
	GDCLASS(RigidBody4D, PhysicsBody4D);

protected:
	float   _mass           = 1.0f;
	float   _gravity_scale  = 1.0f;
	float   _linear_damp    = 0.0f;
	float   _angular_damp   = 0.0f;
	bool    _sleeping       = false;
	bool    _can_sleep      = true;
	bool    _freeze         = false;
	Vector4 _linear_velocity;
	Vector4 _constant_force;

	virtual void _create_physics_body() override;
	virtual void _configure_physics_body() override;

	void _notification(int p_what);
	static void _bind_methods();

	// Called each physics step by the server to sync state back to the node.
	void _state_sync_callback(PhysicsDirectBodyState4D *p_state);

public:
	RigidBody4D();
	virtual ~RigidBody4D();

	void  set_mass(float p_mass);
	float get_mass() const;

	void  set_gravity_scale(float p_scale);
	float get_gravity_scale() const;

	void  set_linear_damp(float p_damp);
	float get_linear_damp() const;

	void  set_angular_damp(float p_damp);
	float get_angular_damp() const;

	void set_sleeping(bool p_sleeping);
	bool get_sleeping() const;

	void set_can_sleep(bool p_can_sleep);
	bool get_can_sleep() const;

	void set_freeze(bool p_freeze);
	bool get_freeze() const;

	void    set_linear_velocity(const Vector4 &p_velocity);
	Vector4 get_linear_velocity() const;

	void    set_constant_force(const Vector4 &p_force);
	Vector4 get_constant_force() const;

	void apply_central_force(const Vector4 &p_force);
	void apply_force(const Vector4 &p_force, const Vector4 &p_position);
	void apply_central_impulse(const Vector4 &p_impulse);
	void apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position);
};
