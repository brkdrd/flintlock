#include "physics_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

PhysicsBody4D::PhysicsBody4D() {}
PhysicsBody4D::~PhysicsBody4D() {}

// ---------------------------------------------------------------------------
// Create physics body (base implementation — subclasses may override to set mode)
// ---------------------------------------------------------------------------

void PhysicsBody4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		_rid = ps->body_create();
	}
}

// ---------------------------------------------------------------------------
// Axis lock helpers — linear
// ---------------------------------------------------------------------------

void PhysicsBody4D::set_axis_lock_linear_x(bool p_lock) {
	_axis_lock_linear_x = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 0, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_linear_x() const { return _axis_lock_linear_x; }

void PhysicsBody4D::set_axis_lock_linear_y(bool p_lock) {
	_axis_lock_linear_y = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 1, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_linear_y() const { return _axis_lock_linear_y; }

void PhysicsBody4D::set_axis_lock_linear_z(bool p_lock) {
	_axis_lock_linear_z = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 2, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_linear_z() const { return _axis_lock_linear_z; }

void PhysicsBody4D::set_axis_lock_linear_w(bool p_lock) {
	_axis_lock_linear_w = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 3, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_linear_w() const { return _axis_lock_linear_w; }

// ---------------------------------------------------------------------------
// Axis lock helpers — angular (planes xy=4, xz=5, xw=6, yz=7, yw=8, zw=9)
// (Offset by 4 to separate from linear axes 0-3)
// ---------------------------------------------------------------------------

void PhysicsBody4D::set_axis_lock_angular_xy(bool p_lock) {
	_axis_lock_angular_xy = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 4, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_xy() const { return _axis_lock_angular_xy; }

void PhysicsBody4D::set_axis_lock_angular_xz(bool p_lock) {
	_axis_lock_angular_xz = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 5, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_xz() const { return _axis_lock_angular_xz; }

void PhysicsBody4D::set_axis_lock_angular_xw(bool p_lock) {
	_axis_lock_angular_xw = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 6, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_xw() const { return _axis_lock_angular_xw; }

void PhysicsBody4D::set_axis_lock_angular_yz(bool p_lock) {
	_axis_lock_angular_yz = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 7, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_yz() const { return _axis_lock_angular_yz; }

void PhysicsBody4D::set_axis_lock_angular_yw(bool p_lock) {
	_axis_lock_angular_yw = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 8, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_yw() const { return _axis_lock_angular_yw; }

void PhysicsBody4D::set_axis_lock_angular_zw(bool p_lock) {
	_axis_lock_angular_zw = p_lock;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_axis_lock(_rid, 9, p_lock);
	}
}
bool PhysicsBody4D::get_axis_lock_angular_zw() const { return _axis_lock_angular_zw; }

// ---------------------------------------------------------------------------
// Stubs
// ---------------------------------------------------------------------------

Variant PhysicsBody4D::move_and_collide(const Vector4 &p_motion) {
	return Variant();
}

bool PhysicsBody4D::test_move(const Vector4 &p_motion) {
	return false;
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void PhysicsBody4D::_bind_methods() {
	// Axis lock - linear
	ClassDB::bind_method(D_METHOD("set_axis_lock_linear_x", "lock"), &PhysicsBody4D::set_axis_lock_linear_x);
	ClassDB::bind_method(D_METHOD("get_axis_lock_linear_x"), &PhysicsBody4D::get_axis_lock_linear_x);
	ClassDB::bind_method(D_METHOD("set_axis_lock_linear_y", "lock"), &PhysicsBody4D::set_axis_lock_linear_y);
	ClassDB::bind_method(D_METHOD("get_axis_lock_linear_y"), &PhysicsBody4D::get_axis_lock_linear_y);
	ClassDB::bind_method(D_METHOD("set_axis_lock_linear_z", "lock"), &PhysicsBody4D::set_axis_lock_linear_z);
	ClassDB::bind_method(D_METHOD("get_axis_lock_linear_z"), &PhysicsBody4D::get_axis_lock_linear_z);
	ClassDB::bind_method(D_METHOD("set_axis_lock_linear_w", "lock"), &PhysicsBody4D::set_axis_lock_linear_w);
	ClassDB::bind_method(D_METHOD("get_axis_lock_linear_w"), &PhysicsBody4D::get_axis_lock_linear_w);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_linear_x"), "set_axis_lock_linear_x", "get_axis_lock_linear_x");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_linear_y"), "set_axis_lock_linear_y", "get_axis_lock_linear_y");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_linear_z"), "set_axis_lock_linear_z", "get_axis_lock_linear_z");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_linear_w"), "set_axis_lock_linear_w", "get_axis_lock_linear_w");

	// Axis lock - angular
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_xy", "lock"), &PhysicsBody4D::set_axis_lock_angular_xy);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_xy"), &PhysicsBody4D::get_axis_lock_angular_xy);
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_xz", "lock"), &PhysicsBody4D::set_axis_lock_angular_xz);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_xz"), &PhysicsBody4D::get_axis_lock_angular_xz);
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_xw", "lock"), &PhysicsBody4D::set_axis_lock_angular_xw);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_xw"), &PhysicsBody4D::get_axis_lock_angular_xw);
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_yz", "lock"), &PhysicsBody4D::set_axis_lock_angular_yz);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_yz"), &PhysicsBody4D::get_axis_lock_angular_yz);
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_yw", "lock"), &PhysicsBody4D::set_axis_lock_angular_yw);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_yw"), &PhysicsBody4D::get_axis_lock_angular_yw);
	ClassDB::bind_method(D_METHOD("set_axis_lock_angular_zw", "lock"), &PhysicsBody4D::set_axis_lock_angular_zw);
	ClassDB::bind_method(D_METHOD("get_axis_lock_angular_zw"), &PhysicsBody4D::get_axis_lock_angular_zw);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_xy"), "set_axis_lock_angular_xy", "get_axis_lock_angular_xy");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_xz"), "set_axis_lock_angular_xz", "get_axis_lock_angular_xz");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_xw"), "set_axis_lock_angular_xw", "get_axis_lock_angular_xw");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_yz"), "set_axis_lock_angular_yz", "get_axis_lock_angular_yz");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_yw"), "set_axis_lock_angular_yw", "get_axis_lock_angular_yw");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "axis_lock_angular_zw"), "set_axis_lock_angular_zw", "get_axis_lock_angular_zw");

	// Stubs
	ClassDB::bind_method(D_METHOD("move_and_collide", "motion"), &PhysicsBody4D::move_and_collide);
	ClassDB::bind_method(D_METHOD("test_move", "motion"), &PhysicsBody4D::test_move);
}
