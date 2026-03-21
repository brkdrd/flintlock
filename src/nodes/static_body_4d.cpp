#include "static_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

StaticBody4D::StaticBody4D() {}
StaticBody4D::~StaticBody4D() {}

void StaticBody4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		_rid = ps->body_create();
	}
}

void StaticBody4D::_configure_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_rid.is_valid()) return;

	ps->body_set_mode(_rid, PhysicsServer4D::BODY_MODE_STATIC);
}

void StaticBody4D::set_constant_linear_velocity(const Vector4 &p_velocity) {
	_constant_linear_velocity = p_velocity;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) {
			ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, p_velocity);
		}
	}
}

Vector4 StaticBody4D::get_constant_linear_velocity() const {
	return _constant_linear_velocity;
}

void StaticBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_constant_linear_velocity", "velocity"), &StaticBody4D::set_constant_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_constant_linear_velocity"), &StaticBody4D::get_constant_linear_velocity);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "constant_linear_velocity"), "set_constant_linear_velocity", "get_constant_linear_velocity");
}
