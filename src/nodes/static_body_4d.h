#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "physics_body_4d.h"

using namespace godot;

class StaticBody4D : public PhysicsBody4D {
	GDCLASS(StaticBody4D, PhysicsBody4D);

protected:
	Vector4 _constant_linear_velocity;

	virtual void _create_physics_body() override;

	static void _bind_methods();

public:
	StaticBody4D();
	virtual ~StaticBody4D();

	void    set_constant_linear_velocity(const Vector4 &p_velocity);
	Vector4 get_constant_linear_velocity() const;
};
