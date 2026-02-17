#include "area_4d.h"
#include <godot_cpp/core/class_db.hpp>

Area4D::Area4D() {}
Area4D::~Area4D() {}

void Area4D::set_gravity(real_t p_gravity) {
	gravity = p_gravity;
}

real_t Area4D::get_gravity() const {
	return gravity;
}

void Area4D::set_gravity_direction(const Vector4 &p_direction) {
	gravity_direction = p_direction;
}

Vector4 Area4D::get_gravity_direction() const {
	return gravity_direction;
}

void Area4D::set_monitoring(bool p_enable) {
	monitoring = p_enable;
}

bool Area4D::is_monitoring() const {
	return monitoring;
}

void Area4D::set_monitorable(bool p_enable) {
	monitorable = p_enable;
}

bool Area4D::is_monitorable() const {
	return monitorable;
}

void Area4D::_ready() {
	// TODO: Create area in PhysicsServer4D when area support is added.
}

void Area4D::_exit_tree() {
	// TODO: Free area RID when area support is added.
}

void Area4D::_bind_methods() {
	// Collision layer/mask methods are inherited from CollisionObject4D.

	ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &Area4D::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &Area4D::get_gravity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

	ClassDB::bind_method(D_METHOD("set_gravity_direction", "direction"), &Area4D::set_gravity_direction);
	ClassDB::bind_method(D_METHOD("get_gravity_direction"), &Area4D::get_gravity_direction);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "gravity_direction"), "set_gravity_direction", "get_gravity_direction");

	ClassDB::bind_method(D_METHOD("set_monitoring", "enable"), &Area4D::set_monitoring);
	ClassDB::bind_method(D_METHOD("is_monitoring"), &Area4D::is_monitoring);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitoring"), "set_monitoring", "is_monitoring");

	ClassDB::bind_method(D_METHOD("set_monitorable", "enable"), &Area4D::set_monitorable);
	ClassDB::bind_method(D_METHOD("is_monitorable"), &Area4D::is_monitorable);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitorable"), "set_monitorable", "is_monitorable");

	// Signals for enter/exit detection.
	ADD_SIGNAL(MethodInfo("body_entered", PropertyInfo(Variant::OBJECT, "body")));
	ADD_SIGNAL(MethodInfo("body_exited", PropertyInfo(Variant::OBJECT, "body")));
	ADD_SIGNAL(MethodInfo("area_entered", PropertyInfo(Variant::OBJECT, "area")));
	ADD_SIGNAL(MethodInfo("area_exited", PropertyInfo(Variant::OBJECT, "area")));
}
