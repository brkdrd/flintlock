#include "pin_joint_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

PinJoint4D::PinJoint4D() {}
PinJoint4D::~PinJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void PinJoint4D::set_damping(float p_damping) {
	_damping = p_damping;
}

float PinJoint4D::get_damping() const {
	return _damping;
}

void PinJoint4D::set_impulse_clamp(float p_clamp) {
	_impulse_clamp = p_clamp;
}

float PinJoint4D::get_impulse_clamp() const {
	return _impulse_clamp;
}

void PinJoint4D::set_bias(float p_bias) {
	_bias = p_bias;
}

float PinJoint4D::get_bias() const {
	return _bias;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void PinJoint4D::_configure_joint() {
	// Pin joint creation would call PhysicsServer4D when a joint creation API
	// is available. The base Joint4D stores _joint_rid for future use.
	// Parameters _damping, _impulse_clamp, _bias would be passed to the server.
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void PinJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_damping", "damping"), &PinJoint4D::set_damping);
	ClassDB::bind_method(D_METHOD("get_damping"), &PinJoint4D::get_damping);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damping", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater"),
		"set_damping", "get_damping");

	ClassDB::bind_method(D_METHOD("set_impulse_clamp", "clamp"), &PinJoint4D::set_impulse_clamp);
	ClassDB::bind_method(D_METHOD("get_impulse_clamp"), &PinJoint4D::get_impulse_clamp);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "impulse_clamp", PROPERTY_HINT_RANGE, "0,64,0.001,or_greater"),
		"set_impulse_clamp", "get_impulse_clamp");

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &PinJoint4D::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &PinJoint4D::get_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bias", PROPERTY_HINT_RANGE, "0.01,0.99,0.001"),
		"set_bias", "get_bias");
}
