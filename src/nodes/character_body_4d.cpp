#include "character_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

CharacterBody4D::CharacterBody4D() {
}

CharacterBody4D::~CharacterBody4D() {
}

// -- Velocity ----------------------------------------------------------------

void CharacterBody4D::set_velocity(const Vector4 &p_velocity) {
	velocity = p_velocity;
}

Vector4 CharacterBody4D::get_velocity() const {
	return velocity;
}

// -- Movement ----------------------------------------------------------------

void CharacterBody4D::set_up_direction(const Vector4 &p_direction) {
	up_direction = p_direction;
}

Vector4 CharacterBody4D::get_up_direction() const {
	return up_direction;
}

void CharacterBody4D::set_floor_max_angle(real_t p_angle) {
	floor_max_angle = p_angle;
}

real_t CharacterBody4D::get_floor_max_angle() const {
	return floor_max_angle;
}

void CharacterBody4D::set_floor_stop_on_slope(bool p_enabled) {
	floor_stop_on_slope = p_enabled;
}

bool CharacterBody4D::is_floor_stop_on_slope_enabled() const {
	return floor_stop_on_slope;
}

void CharacterBody4D::set_floor_constant_speed(bool p_enabled) {
	floor_constant_speed = p_enabled;
}

bool CharacterBody4D::is_floor_constant_speed_enabled() const {
	return floor_constant_speed;
}

void CharacterBody4D::set_floor_block_on_wall(bool p_enabled) {
	floor_block_on_wall = p_enabled;
}

bool CharacterBody4D::is_floor_block_on_wall_enabled() const {
	return floor_block_on_wall;
}

void CharacterBody4D::set_max_slides(int p_max_slides) {
	max_slides = p_max_slides;
}

int CharacterBody4D::get_max_slides() const {
	return max_slides;
}

void CharacterBody4D::set_safe_margin(real_t p_margin) {
	safe_margin = p_margin;
}

real_t CharacterBody4D::get_safe_margin() const {
	return safe_margin;
}

// -- Kinematic motion --------------------------------------------------------

void CharacterBody4D::move_and_slide() {
	// TODO: Implement move_and_slide with 4D collision resolution
	// For now, just update position based on velocity
	Vector4 current_pos = get_position_4d();
	set_position_4d(current_pos + velocity);
}

bool CharacterBody4D::is_on_floor() const {
	// TODO: Implement floor detection
	return false;
}

bool CharacterBody4D::is_on_wall() const {
	// TODO: Implement wall detection
	return false;
}

bool CharacterBody4D::is_on_ceiling() const {
	// TODO: Implement ceiling detection
	return false;
}

Vector4 CharacterBody4D::get_floor_normal() const {
	// TODO: Return actual floor normal from collision
	return Vector4(0, 0, 0, 1);
}

Vector4 CharacterBody4D::get_wall_normal() const {
	// TODO: Return actual wall normal from collision
	return Vector4(0, 0, 0, 0);
}

// -- Godot lifecycle ---------------------------------------------------------

void CharacterBody4D::_ready() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) {
		return;
	}

	// Create physics body in the server
	body_rid = server->body_create();
	server->body_set_mode(body_rid, PhysicsServer4D::BODY_MODE_KINEMATIC);

	// Set initial transform
	server->body_set_state(body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, transform_4d);

	// TODO: Add to space when implemented
}

void CharacterBody4D::_exit_tree() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (server && body_rid.is_valid()) {
		server->free_rid(body_rid);
		body_rid = RID();
	}
}

// -- Bind methods ------------------------------------------------------------

void CharacterBody4D::_bind_methods() {
	// Velocity
	ClassDB::bind_method(D_METHOD("set_velocity", "velocity"), &CharacterBody4D::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &CharacterBody4D::get_velocity);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "velocity"), "set_velocity", "get_velocity");

	// Up direction
	ClassDB::bind_method(D_METHOD("set_up_direction", "direction"), &CharacterBody4D::set_up_direction);
	ClassDB::bind_method(D_METHOD("get_up_direction"), &CharacterBody4D::get_up_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "up_direction"), "set_up_direction", "get_up_direction");

	// Floor settings
	ClassDB::bind_method(D_METHOD("set_floor_max_angle", "angle"), &CharacterBody4D::set_floor_max_angle);
	ClassDB::bind_method(D_METHOD("get_floor_max_angle"), &CharacterBody4D::get_floor_max_angle);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "floor_max_angle"), "set_floor_max_angle", "get_floor_max_angle");

	ClassDB::bind_method(D_METHOD("set_floor_stop_on_slope", "enabled"), &CharacterBody4D::set_floor_stop_on_slope);
	ClassDB::bind_method(D_METHOD("is_floor_stop_on_slope_enabled"), &CharacterBody4D::is_floor_stop_on_slope_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "floor_stop_on_slope"), "set_floor_stop_on_slope", "is_floor_stop_on_slope_enabled");

	ClassDB::bind_method(D_METHOD("set_floor_constant_speed", "enabled"), &CharacterBody4D::set_floor_constant_speed);
	ClassDB::bind_method(D_METHOD("is_floor_constant_speed_enabled"), &CharacterBody4D::is_floor_constant_speed_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "floor_constant_speed"), "set_floor_constant_speed", "is_floor_constant_speed_enabled");

	ClassDB::bind_method(D_METHOD("set_floor_block_on_wall", "enabled"), &CharacterBody4D::set_floor_block_on_wall);
	ClassDB::bind_method(D_METHOD("is_floor_block_on_wall_enabled"), &CharacterBody4D::is_floor_block_on_wall_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "floor_block_on_wall"), "set_floor_block_on_wall", "is_floor_block_on_wall_enabled");

	// Slide settings
	ClassDB::bind_method(D_METHOD("set_max_slides", "max_slides"), &CharacterBody4D::set_max_slides);
	ClassDB::bind_method(D_METHOD("get_max_slides"), &CharacterBody4D::get_max_slides);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_slides"), "set_max_slides", "get_max_slides");

	ClassDB::bind_method(D_METHOD("set_safe_margin", "margin"), &CharacterBody4D::set_safe_margin);
	ClassDB::bind_method(D_METHOD("get_safe_margin"), &CharacterBody4D::get_safe_margin);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "safe_margin"), "set_safe_margin", "get_safe_margin");

	// Movement methods
	ClassDB::bind_method(D_METHOD("move_and_slide"), &CharacterBody4D::move_and_slide);
	ClassDB::bind_method(D_METHOD("is_on_floor"), &CharacterBody4D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &CharacterBody4D::is_on_wall);
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &CharacterBody4D::is_on_ceiling);
	ClassDB::bind_method(D_METHOD("get_floor_normal"), &CharacterBody4D::get_floor_normal);
	ClassDB::bind_method(D_METHOD("get_wall_normal"), &CharacterBody4D::get_wall_normal);
}
