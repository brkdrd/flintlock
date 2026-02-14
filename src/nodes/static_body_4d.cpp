#include "static_body_4d.h"
#include "server/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

StaticBody4D::StaticBody4D() {
}

StaticBody4D::~StaticBody4D() {
}

void StaticBody4D::_ready() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) {
		return;
	}

	// Create physics body in the server
	body_rid = server->body_create();
	server->body_set_mode(body_rid, PhysicsServer4D::BODY_MODE_STATIC);
	server->body_set_param(body_rid, PhysicsServer4D::BODY_PARAM_MASS, 0.0);

	// Set initial transform
	Transform4D transform;
	transform.origin = position_4d;
	server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, transform);

	// Update 3D position for rendering
	set_position(Vector3(position_4d.x, position_4d.y, position_4d.z));
}

void StaticBody4D::_exit_tree() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->free_rid(body_rid);
		body_rid = flintlock::PhysicsRID();
	}
}

void StaticBody4D::set_position_4d(const Vector4 &p_position) {
	position_4d = p_position;
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		Transform4D transform;
		transform.origin = position_4d;
		server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, transform);
	}
	set_position(Vector3(position_4d.x, position_4d.y, position_4d.z));
}

Vector4 StaticBody4D::get_position_4d() const {
	return position_4d;
}

flintlock::PhysicsRID StaticBody4D::get_rid() const {
	return body_rid;
}

void StaticBody4D::_bind_methods() {
	// Position
	ClassDB::bind_method(D_METHOD("set_position_4d", "position"), &StaticBody4D::set_position_4d);
	ClassDB::bind_method(D_METHOD("get_position_4d"), &StaticBody4D::get_position_4d);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position_4d"), "set_position_4d", "get_position_4d");

	// Note: get_rid() is not bound - it's for internal C++ use only
}
