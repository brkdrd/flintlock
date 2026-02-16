#include "hyper_capsule_shape_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

HyperCapsuleShape4DResource::HyperCapsuleShape4DResource() {
	radius = 1.0;
	height = 2.0;
}

HyperCapsuleShape4DResource::~HyperCapsuleShape4DResource() {
}

void HyperCapsuleShape4DResource::set_radius(real_t p_radius) {
	radius = p_radius;
	emit_changed();  // Notify that the resource has changed
}

real_t HyperCapsuleShape4DResource::get_radius() const {
	return radius;
}

void HyperCapsuleShape4DResource::set_height(real_t p_height) {
	height = p_height;
	emit_changed();
}

real_t HyperCapsuleShape4DResource::get_height() const {
	return height;
}

void HyperCapsuleShape4DResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &HyperCapsuleShape4DResource::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &HyperCapsuleShape4DResource::get_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.01,100.0,0.01"), "set_radius", "get_radius");

	ClassDB::bind_method(D_METHOD("set_height", "height"), &HyperCapsuleShape4DResource::set_height);
	ClassDB::bind_method(D_METHOD("get_height"), &HyperCapsuleShape4DResource::get_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.01,100.0,0.01"), "set_height", "get_height");
}
