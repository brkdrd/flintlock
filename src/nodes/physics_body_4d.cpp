#include "physics_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsBody4D::PhysicsBody4D() {}
PhysicsBody4D::~PhysicsBody4D() {}

RID PhysicsBody4D::get_rid() const {
	return base_rid;
}

void PhysicsBody4D::_ready() {
	// Base implementation — derived classes override.
}

void PhysicsBody4D::_exit_tree() {
	// Base implementation — derived classes override.
}

void PhysicsBody4D::_bind_methods() {
	// Collision layer/mask methods are inherited from CollisionObject4D.
	// Derived classes bind their own specific methods.
}
