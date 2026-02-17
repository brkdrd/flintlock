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
	base_rid = server->body_create();
	server->body_set_mode(base_rid, PhysicsServer4D::BODY_MODE_STATIC);
	server->body_set_param(base_rid, PhysicsServer4D::BODY_PARAM_MASS, 0.0);

	// Set initial transform from Node4D's transform_4d
	server->body_set_state(base_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, transform_4d);

	// Note: 3D visualization should be handled by a separate visualization system or child Node3D
}

void StaticBody4D::_exit_tree() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && base_rid.is_valid()) {
		server->free_rid(base_rid);
		base_rid = RID();
	}
}

void StaticBody4D::_bind_methods() {
	// Note: position_4d is inherited from Node4D
	// Note: get_rid() is not bound - it's for internal C++ use only
}
