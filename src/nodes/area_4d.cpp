#include "area_4d.h"
#include <godot_cpp/core/class_db.hpp>

Area4D::Area4D() {
}

Area4D::~Area4D() {
}

// -- Collision layers and masks ----------------------------------------------

void Area4D::set_collision_layer(uint32_t p_layer) {
	collision_layer = p_layer;
	// TODO: Update physics server when area system is implemented
}

uint32_t Area4D::get_collision_layer() const {
	return collision_layer;
}

void Area4D::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;
	// TODO: Update physics server when area system is implemented
}

uint32_t Area4D::get_collision_mask() const {
	return collision_mask;
}

void Area4D::set_collision_layer_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
	uint32_t bit = 1 << (p_layer_number - 1);
	if (p_value) {
		collision_layer |= bit;
	} else {
		collision_layer &= ~bit;
	}
}

bool Area4D::get_collision_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return collision_layer & (1 << (p_layer_number - 1));
}

void Area4D::set_collision_mask_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32, "Collision layer number must be between 1 and 32 inclusive.");
	uint32_t bit = 1 << (p_layer_number - 1);
	if (p_value) {
		collision_mask |= bit;
	} else {
		collision_mask &= ~bit;
	}
}

bool Area4D::get_collision_mask_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false, "Collision layer number must be between 1 and 32 inclusive.");
	return collision_mask & (1 << (p_layer_number - 1));
}

// -- Gravity override --------------------------------------------------------

void Area4D::set_gravity(real_t p_gravity) {
	gravity = p_gravity;
	// TODO: Update physics server when area system is implemented
}

real_t Area4D::get_gravity() const {
	return gravity;
}

void Area4D::set_gravity_direction(const Vector4 &p_direction) {
	gravity_direction = p_direction;
	// TODO: Update physics server when area system is implemented
}

Vector4 Area4D::get_gravity_direction() const {
	return gravity_direction;
}

// -- Monitoring --------------------------------------------------------------

void Area4D::set_monitoring(bool p_enable) {
	monitoring = p_enable;
	// TODO: Update physics server when area system is implemented
}

bool Area4D::is_monitoring() const {
	return monitoring;
}

void Area4D::set_monitorable(bool p_enable) {
	monitorable = p_enable;
	// TODO: Update physics server when area system is implemented
}

bool Area4D::is_monitorable() const {
	return monitorable;
}

// -- Physics server interaction ----------------------------------------------

RID Area4D::get_rid() const {
	return area_rid;
}

// -- Godot lifecycle ---------------------------------------------------------

void Area4D::_ready() {
	// TODO: Create area in PhysicsServer4D when area support is added
	// For now, just a placeholder
}

void Area4D::_exit_tree() {
	// TODO: Free area RID when area support is added
}

// -- Bind methods ------------------------------------------------------------

void Area4D::_bind_methods() {
	// Collision layers
	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &Area4D::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &Area4D::get_collision_layer);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_layer", "get_collision_layer");

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &Area4D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &Area4D::get_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS), "set_collision_mask", "get_collision_mask");

	// Individual layer/mask bit access
	ClassDB::bind_method(D_METHOD("set_collision_layer_value", "layer_number", "value"), &Area4D::set_collision_layer_value);
	ClassDB::bind_method(D_METHOD("get_collision_layer_value", "layer_number"), &Area4D::get_collision_layer_value);

	ClassDB::bind_method(D_METHOD("set_collision_mask_value", "layer_number", "value"), &Area4D::set_collision_mask_value);
	ClassDB::bind_method(D_METHOD("get_collision_mask_value", "layer_number"), &Area4D::get_collision_mask_value);

	// Gravity
	ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &Area4D::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &Area4D::get_gravity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

	ClassDB::bind_method(D_METHOD("set_gravity_direction", "direction"), &Area4D::set_gravity_direction);
	ClassDB::bind_method(D_METHOD("get_gravity_direction"), &Area4D::get_gravity_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "gravity_direction"), "set_gravity_direction", "get_gravity_direction");

	// Monitoring
	ClassDB::bind_method(D_METHOD("set_monitoring", "enable"), &Area4D::set_monitoring);
	ClassDB::bind_method(D_METHOD("is_monitoring"), &Area4D::is_monitoring);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitoring"), "set_monitoring", "is_monitoring");

	ClassDB::bind_method(D_METHOD("set_monitorable", "enable"), &Area4D::set_monitorable);
	ClassDB::bind_method(D_METHOD("is_monitorable"), &Area4D::is_monitorable);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitorable"), "set_monitorable", "is_monitorable");

	// TODO: Add signals for body_entered, body_exited, area_entered, area_exited
}
