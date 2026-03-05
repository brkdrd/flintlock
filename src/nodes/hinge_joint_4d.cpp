#include "hinge_joint_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

HingeJoint4D::HingeJoint4D() {}
HingeJoint4D::~HingeJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void HingeJoint4D::set_angular_limit_enabled(bool p_enabled) {
	_angular_limit_enabled = p_enabled;
}

bool HingeJoint4D::is_angular_limit_enabled() const {
	return _angular_limit_enabled;
}

void HingeJoint4D::set_angular_limit_upper(float p_upper) {
	_angular_limit_upper = p_upper;
}

float HingeJoint4D::get_angular_limit_upper() const {
	return _angular_limit_upper;
}

void HingeJoint4D::set_angular_limit_lower(float p_lower) {
	_angular_limit_lower = p_lower;
}

float HingeJoint4D::get_angular_limit_lower() const {
	return _angular_limit_lower;
}

void HingeJoint4D::set_motor_enabled(bool p_enabled) {
	_motor_enabled = p_enabled;
}

bool HingeJoint4D::is_motor_enabled() const {
	return _motor_enabled;
}

void HingeJoint4D::set_motor_target_velocity(float p_velocity) {
	_motor_target_velocity = p_velocity;
}

float HingeJoint4D::get_motor_target_velocity() const {
	return _motor_target_velocity;
}

void HingeJoint4D::set_motor_max_impulse(float p_impulse) {
	_motor_max_impulse = p_impulse;
}

float HingeJoint4D::get_motor_max_impulse() const {
	return _motor_max_impulse;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void HingeJoint4D::_configure_joint() {
	// Hinge joint creation would use PhysicsServer4D when a joint API is available.
	// Parameters would include angular limits and motor settings.
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void HingeJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_angular_limit_enabled", "enabled"), &HingeJoint4D::set_angular_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_limit_enabled"), &HingeJoint4D::is_angular_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_limit_enabled"),
		"set_angular_limit_enabled", "is_angular_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_limit_upper", "angle"), &HingeJoint4D::set_angular_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_limit_upper"), &HingeJoint4D::get_angular_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_limit_upper", PROPERTY_HINT_RANGE, "-6.2832,6.2832,0.001,radians"),
		"set_angular_limit_upper", "get_angular_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_limit_lower", "angle"), &HingeJoint4D::set_angular_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_limit_lower"), &HingeJoint4D::get_angular_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_limit_lower", PROPERTY_HINT_RANGE, "-6.2832,6.2832,0.001,radians"),
		"set_angular_limit_lower", "get_angular_limit_lower");

	ClassDB::bind_method(D_METHOD("set_motor_enabled", "enabled"), &HingeJoint4D::set_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_motor_enabled"), &HingeJoint4D::is_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "motor_enabled"), "set_motor_enabled", "is_motor_enabled");

	ClassDB::bind_method(D_METHOD("set_motor_target_velocity", "velocity"), &HingeJoint4D::set_motor_target_velocity);
	ClassDB::bind_method(D_METHOD("get_motor_target_velocity"), &HingeJoint4D::get_motor_target_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_target_velocity", PROPERTY_HINT_RANGE, "-1000,1000,0.01,or_lesser,or_greater"),
		"set_motor_target_velocity", "get_motor_target_velocity");

	ClassDB::bind_method(D_METHOD("set_motor_max_impulse", "impulse"), &HingeJoint4D::set_motor_max_impulse);
	ClassDB::bind_method(D_METHOD("get_motor_max_impulse"), &HingeJoint4D::get_motor_max_impulse);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "motor_max_impulse", PROPERTY_HINT_RANGE, "0,1000,0.01,or_greater"),
		"set_motor_max_impulse", "get_motor_max_impulse");
}
