#include "rigid_body_4d.h"
#include "server/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

RigidBody4D::RigidBody4D() {
}

RigidBody4D::~RigidBody4D() {
}

void RigidBody4D::_ready() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) {
		return;
	}

	// Create physics body in the server
	body_rid = server->body_create();
	server->body_set_mode(body_rid, PhysicsServer4D::BODY_MODE_RIGID);

	// Set initial properties
	server->body_set_param(body_rid, PhysicsServer4D::BODY_PARAM_MASS, mass);
	server->body_set_param(body_rid, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, linear_damp);

	// Set initial transform from Node4D's transform_4d
	server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, transform_4d);

	server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, linear_velocity);

	// TODO: Add to space (for now, bodies aren't in a space automatically)
}

void RigidBody4D::_physics_process(double p_delta) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server || !body_rid.is_valid()) {
		return;
	}

	// Read back the physics state
	transform_4d = server->body_get_state(body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM);

	linear_velocity = server->body_get_state(body_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);

	// Update 3D position for rendering (inherited from Node4D)
	_update_3d_position();
}

void RigidBody4D::_exit_tree() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->free_rid(body_rid);
		body_rid = flintlock::PhysicsRID();
	}
}

// Mass
void RigidBody4D::set_mass(real_t p_mass) {
	mass = p_mass;
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->body_set_param(body_rid, PhysicsServer4D::BODY_PARAM_MASS, mass);
	}
}

real_t RigidBody4D::get_mass() const {
	return mass;
}

// Damping
void RigidBody4D::set_linear_damp(real_t p_damp) {
	linear_damp = p_damp;
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->body_set_param(body_rid, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, linear_damp);
	}
}

real_t RigidBody4D::get_linear_damp() const {
	return linear_damp;
}

// Velocity
void RigidBody4D::set_linear_velocity(const Vector4 &p_velocity) {
	linear_velocity = p_velocity;
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, linear_velocity);
	}
}

Vector4 RigidBody4D::get_linear_velocity() const {
	return linear_velocity;
}

// Forces
void RigidBody4D::apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->body_apply_impulse(body_rid, p_impulse, p_position);
	}
}

void RigidBody4D::apply_force(const Vector4 &p_force, const Vector4 &p_position) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->body_apply_force(body_rid, p_force, p_position);
	}
}

flintlock::PhysicsRID RigidBody4D::get_rid() const {
	return body_rid;
}

// Bind methods for GDScript exposure
void RigidBody4D::_bind_methods() {
	// Mass
	ClassDB::bind_method(D_METHOD("set_mass", "mass"), &RigidBody4D::set_mass);
	ClassDB::bind_method(D_METHOD("get_mass"), &RigidBody4D::get_mass);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");

	// Damping
	ClassDB::bind_method(D_METHOD("set_linear_damp", "damp"), &RigidBody4D::set_linear_damp);
	ClassDB::bind_method(D_METHOD("get_linear_damp"), &RigidBody4D::get_linear_damp);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_damp"), "set_linear_damp", "get_linear_damp");

	// Note: position_4d is inherited from Node4D

	// Velocity
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "velocity"), &RigidBody4D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &RigidBody4D::get_linear_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity"), "set_linear_velocity", "get_linear_velocity");

	// Forces
	ClassDB::bind_method(D_METHOD("apply_impulse", "impulse", "position"), &RigidBody4D::apply_impulse, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("apply_force", "force", "position"), &RigidBody4D::apply_force, DEFVAL(Vector4()));

	// Note: get_rid() is not bound - it's for internal C++ use only
}
