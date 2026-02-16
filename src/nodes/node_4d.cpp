#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>

Node4D::Node4D() {
	transform_4d = Transform4D();
}

Node4D::~Node4D() {
}

void Node4D::_update_3d_position() {
	// Project 4D position to 3D by using X, Y, Z components
	// (W component is not visible in 3D space)
	Vector3 pos_3d(transform_4d.origin.x, transform_4d.origin.y, transform_4d.origin.z);
	set_position(pos_3d);
}

// -- Transform4D access ------------------------------------------------------

void Node4D::set_transform_4d(const Transform4D &p_transform) {
	transform_4d = p_transform;
	_update_3d_position();
}

Transform4D Node4D::get_transform_4d() const {
	return transform_4d;
}

// -- Position (4D origin) ----------------------------------------------------

void Node4D::set_position_4d(const Vector4 &p_position) {
	transform_4d.origin = p_position;
	_update_3d_position();
}

Vector4 Node4D::get_position_4d() const {
	return transform_4d.origin;
}

// -- Individual coordinate access --------------------------------------------

void Node4D::set_position_4d_x(real_t p_x) {
	transform_4d.origin.x = p_x;
	_update_3d_position();
}

void Node4D::set_position_4d_y(real_t p_y) {
	transform_4d.origin.y = p_y;
	_update_3d_position();
}

void Node4D::set_position_4d_z(real_t p_z) {
	transform_4d.origin.z = p_z;
	_update_3d_position();
}

void Node4D::set_position_4d_w(real_t p_w) {
	transform_4d.origin.w = p_w;
	// W doesn't affect 3D position, so no need to update
}

real_t Node4D::get_position_4d_x() const {
	return transform_4d.origin.x;
}

real_t Node4D::get_position_4d_y() const {
	return transform_4d.origin.y;
}

real_t Node4D::get_position_4d_z() const {
	return transform_4d.origin.z;
}

real_t Node4D::get_position_4d_w() const {
	return transform_4d.origin.w;
}

// -- Translation -------------------------------------------------------------

void Node4D::translate_4d(const Vector4 &p_offset) {
	transform_4d.origin = transform_4d.origin + p_offset;
	_update_3d_position();
}

// -- Godot lifecycle ---------------------------------------------------------

void Node4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_update_3d_position();
			break;
	}
}

// -- Bind methods ------------------------------------------------------------

void Node4D::_bind_methods() {
	// Position
	ClassDB::bind_method(D_METHOD("set_position_4d", "position"), &Node4D::set_position_4d);
	ClassDB::bind_method(D_METHOD("get_position_4d"), &Node4D::get_position_4d);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position_4d"), "set_position_4d", "get_position_4d");

	// Individual coordinates
	ClassDB::bind_method(D_METHOD("set_position_4d_x", "x"), &Node4D::set_position_4d_x);
	ClassDB::bind_method(D_METHOD("set_position_4d_y", "y"), &Node4D::set_position_4d_y);
	ClassDB::bind_method(D_METHOD("set_position_4d_z", "z"), &Node4D::set_position_4d_z);
	ClassDB::bind_method(D_METHOD("set_position_4d_w", "w"), &Node4D::set_position_4d_w);

	ClassDB::bind_method(D_METHOD("get_position_4d_x"), &Node4D::get_position_4d_x);
	ClassDB::bind_method(D_METHOD("get_position_4d_y"), &Node4D::get_position_4d_y);
	ClassDB::bind_method(D_METHOD("get_position_4d_z"), &Node4D::get_position_4d_z);
	ClassDB::bind_method(D_METHOD("get_position_4d_w"), &Node4D::get_position_4d_w);

	// Translation
	ClassDB::bind_method(D_METHOD("translate_4d", "offset"), &Node4D::translate_4d);

	// Note: Transform4D is not exposed directly because it's a custom C++ type
	// Users should use position_4d and individual setters for now
}
