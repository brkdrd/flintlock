#include "cone_twist_joint_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

ConeTwistJoint4D::ConeTwistJoint4D() {}
ConeTwistJoint4D::~ConeTwistJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void ConeTwistJoint4D::set_swing_span(float p_swing_span) {
	_swing_span = p_swing_span;
}

float ConeTwistJoint4D::get_swing_span() const {
	return _swing_span;
}

void ConeTwistJoint4D::set_twist_span(float p_twist_span) {
	_twist_span = p_twist_span;
}

float ConeTwistJoint4D::get_twist_span() const {
	return _twist_span;
}

void ConeTwistJoint4D::set_bias(float p_bias) {
	_bias = p_bias;
}

float ConeTwistJoint4D::get_bias() const {
	return _bias;
}

void ConeTwistJoint4D::set_softness(float p_softness) {
	_softness = p_softness;
}

float ConeTwistJoint4D::get_softness() const {
	return _softness;
}

void ConeTwistJoint4D::set_relaxation(float p_relaxation) {
	_relaxation = p_relaxation;
}

float ConeTwistJoint4D::get_relaxation() const {
	return _relaxation;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void ConeTwistJoint4D::_configure_joint() {
	// Cone-twist joint creation on PhysicsServer4D when joint API is available.
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void ConeTwistJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_swing_span", "swing_span"), &ConeTwistJoint4D::set_swing_span);
	ClassDB::bind_method(D_METHOD("get_swing_span"), &ConeTwistJoint4D::get_swing_span);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "swing_span", PROPERTY_HINT_RANGE, "0,6.2832,0.001,radians"),
		"set_swing_span", "get_swing_span");

	ClassDB::bind_method(D_METHOD("set_twist_span", "twist_span"), &ConeTwistJoint4D::set_twist_span);
	ClassDB::bind_method(D_METHOD("get_twist_span"), &ConeTwistJoint4D::get_twist_span);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "twist_span", PROPERTY_HINT_RANGE, "0,6.2832,0.001,radians"),
		"set_twist_span", "get_twist_span");

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &ConeTwistJoint4D::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &ConeTwistJoint4D::get_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bias", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_bias", "get_bias");

	ClassDB::bind_method(D_METHOD("set_softness", "softness"), &ConeTwistJoint4D::set_softness);
	ClassDB::bind_method(D_METHOD("get_softness"), &ConeTwistJoint4D::get_softness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "softness", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_softness", "get_softness");

	ClassDB::bind_method(D_METHOD("set_relaxation", "relaxation"), &ConeTwistJoint4D::set_relaxation);
	ClassDB::bind_method(D_METHOD("get_relaxation"), &ConeTwistJoint4D::get_relaxation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "relaxation", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_relaxation", "get_relaxation");
}
