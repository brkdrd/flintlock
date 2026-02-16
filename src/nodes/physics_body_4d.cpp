#include "physics_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsBody4D::PhysicsBody4D() {
}

PhysicsBody4D::~PhysicsBody4D() {
}

// -- Collision layers and masks ----------------------------------------------

void PhysicsBody4D::set_collision_layer(uint32_t p_layer) {
	collision_layer = p_layer;
	// TODO: Update physics server when collision system is implemented
}

uint32_t PhysicsBody4D::get_collision_layer() const {
	return collision_layer;
}

void PhysicsBody4D::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;
	// TODO: Update physics server when collision system is implemented
}

uint32_t PhysicsBody4D::get_collision_mask() const {
	return collision_mask;
}

void PhysicsBody4D::set_collision_layer_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
	uint32_t bit = 1 << (p_layer_number - 1);
	if (p_value) {
		collision_layer |= bit;
	} else {
		collision_layer &= ~bit;
	}
	// TODO: Update physics server when collision system is implemented
}

bool PhysicsBody4D::get_collision_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return collision_layer & (1 << (p_layer_number - 1));
}

void PhysicsBody4D::set_collision_mask_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
	uint32_t bit = 1 << (p_layer_number - 1);
	if (p_value) {
		collision_mask |= bit;
	} else {
		collision_mask &= ~bit;
	}
	// TODO: Update physics server when collision system is implemented
}

bool PhysicsBody4D::get_collision_mask_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return collision_mask & (1 << (p_layer_number - 1));
}

// -- Physics server interaction ----------------------------------------------

RID PhysicsBody4D::get_rid() const {
	return body_rid;
}

// -- Godot lifecycle ---------------------------------------------------------

void PhysicsBody4D::_ready() {
	// Base implementation - derived classes will override
}

void PhysicsBody4D::_exit_tree() {
	// Base implementation - derived classes will override
}

// -- Bind methods ------------------------------------------------------------

void PhysicsBody4D::_bind_methods() {
	// Collision layers
	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &PhysicsBody4D::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &PhysicsBody4D::get_collision_layer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &PhysicsBody4D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &PhysicsBody4D::get_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

	// Individual layer/mask bit access
	ClassDB::bind_method(D_METHOD("set_collision_layer_value", "layer_number", "value"), &PhysicsBody4D::set_collision_layer_value);
	ClassDB::bind_method(D_METHOD("get_collision_layer_value", "layer_number"), &PhysicsBody4D::get_collision_layer_value);

	ClassDB::bind_method(D_METHOD("set_collision_mask_value", "layer_number", "value"), &PhysicsBody4D::set_collision_mask_value);
	ClassDB::bind_method(D_METHOD("get_collision_mask_value", "layer_number"), &PhysicsBody4D::get_collision_mask_value);

	// Note: get_rid() is not bound - it's for internal C++ use only
}
