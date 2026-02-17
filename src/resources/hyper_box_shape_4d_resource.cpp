#include "hyper_box_shape_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

HyperBoxShape4DResource::HyperBoxShape4DResource() {
	shape_data.set_half_extents(half_extents);
}

HyperBoxShape4DResource::~HyperBoxShape4DResource() {}

void HyperBoxShape4DResource::set_half_extents(const Vector4 &p_half_extents) {
	half_extents = p_half_extents;
	shape_data.set_half_extents(half_extents);
	emit_changed();
}

Vector4 HyperBoxShape4DResource::get_half_extents() const {
	return half_extents;
}

Shape4D *HyperBoxShape4DResource::get_shape() const {
	return &shape_data;
}

void HyperBoxShape4DResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_half_extents", "half_extents"), &HyperBoxShape4DResource::set_half_extents);
	ClassDB::bind_method(D_METHOD("get_half_extents"), &HyperBoxShape4DResource::get_half_extents);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "half_extents"), "set_half_extents", "get_half_extents");
}
