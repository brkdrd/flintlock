#include "joint_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

Joint4D::Joint4D() {}
Joint4D::~Joint4D() {
	if (_joint_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) {
			ps->free_rid(_joint_rid);
		}
		_joint_rid = RID();
	}
}

// ─── Properties ───────────────────────────────────────────────────────────────

void Joint4D::set_node_a(const NodePath &p_node_a) {
	_node_a = p_node_a;
	_update_joint();
}

NodePath Joint4D::get_node_a() const {
	return _node_a;
}

void Joint4D::set_node_b(const NodePath &p_node_b) {
	_node_b = p_node_b;
	_update_joint();
}

NodePath Joint4D::get_node_b() const {
	return _node_b;
}

void Joint4D::set_solver_priority(int p_priority) {
	_solver_priority = p_priority;
}

int Joint4D::get_solver_priority() const {
	return _solver_priority;
}

void Joint4D::set_exclude_nodes_from_collision(bool p_exclude) {
	_exclude_nodes_from_collision = p_exclude;
}

bool Joint4D::get_exclude_nodes_from_collision() const {
	return _exclude_nodes_from_collision;
}

RID Joint4D::get_joint_rid() const {
	return _joint_rid;
}

// ─── Internal ─────────────────────────────────────────────────────────────────

void Joint4D::_update_joint() {
	if (!is_inside_tree()) {
		return;
	}
	// Free the existing joint if present
	if (_joint_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) {
			ps->free_rid(_joint_rid);
		}
		_joint_rid = RID();
	}
	// Delegate joint creation to the subclass
	_configure_joint();
}

void Joint4D::_configure_joint() {
	// Base implementation does nothing; subclasses override.
}

// ─── Notification ─────────────────────────────────────────────────────────────

void Joint4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_update_joint();
			break;
		case NOTIFICATION_EXIT_TREE:
			if (_joint_rid.is_valid()) {
				PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
				if (ps) {
					ps->free_rid(_joint_rid);
				}
				_joint_rid = RID();
			}
			break;
		default:
			break;
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void Joint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_node_a", "node"), &Joint4D::set_node_a);
	ClassDB::bind_method(D_METHOD("get_node_a"), &Joint4D::get_node_a);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_a", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody4D"),
		"set_node_a", "get_node_a");

	ClassDB::bind_method(D_METHOD("set_node_b", "node"), &Joint4D::set_node_b);
	ClassDB::bind_method(D_METHOD("get_node_b"), &Joint4D::get_node_b);
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "node_b", PROPERTY_HINT_NODE_PATH_VALID_TYPES, "PhysicsBody4D"),
		"set_node_b", "get_node_b");

	ClassDB::bind_method(D_METHOD("set_solver_priority", "priority"), &Joint4D::set_solver_priority);
	ClassDB::bind_method(D_METHOD("get_solver_priority"), &Joint4D::get_solver_priority);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "solver_priority", PROPERTY_HINT_RANGE, "1,8,1"),
		"set_solver_priority", "get_solver_priority");

	ClassDB::bind_method(D_METHOD("set_exclude_nodes_from_collision", "exclude"), &Joint4D::set_exclude_nodes_from_collision);
	ClassDB::bind_method(D_METHOD("get_exclude_nodes_from_collision"), &Joint4D::get_exclude_nodes_from_collision);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exclude_nodes_from_collision"),
		"set_exclude_nodes_from_collision", "get_exclude_nodes_from_collision");

	ClassDB::bind_method(D_METHOD("get_joint_rid"), &Joint4D::get_joint_rid);
}
