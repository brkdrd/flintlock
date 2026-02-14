#include "collision_shape_4d.h"
#include <godot_cpp/core/class_db.hpp>

CollisionShape4D::CollisionShape4D() {
}

CollisionShape4D::~CollisionShape4D() {
}

void CollisionShape4D::_ready() {
	// TODO: Attach shape to parent body when shape resources are implemented
}

void CollisionShape4D::set_shape(const Ref<Resource> &p_shape) {
	shape = p_shape;
	// TODO: Update physics server with new shape
}

Ref<Resource> CollisionShape4D::get_shape() const {
	return shape;
}

void CollisionShape4D::set_disabled(bool p_disabled) {
	disabled = p_disabled;
	// TODO: Enable/disable collision in physics server
}

bool CollisionShape4D::is_disabled() const {
	return disabled;
}

void CollisionShape4D::_bind_methods() {
	// Shape
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape4D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape4D::get_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Resource"), "set_shape", "get_shape");

	// Disabled
	ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &CollisionShape4D::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &CollisionShape4D::is_disabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");
}
