#include "hyper_ellipsoid_shape_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

HyperEllipsoidShape4DResource::HyperEllipsoidShape4DResource() {
	shape_data.set_radii(radii);
}

HyperEllipsoidShape4DResource::~HyperEllipsoidShape4DResource() {}

void HyperEllipsoidShape4DResource::set_radii(const Vector4 &p_radii) {
	radii = p_radii;
	shape_data.set_radii(radii);
	emit_changed();
}

Vector4 HyperEllipsoidShape4DResource::get_radii() const {
	return radii;
}

Shape4D *HyperEllipsoidShape4DResource::get_shape() const {
	return &shape_data;
}

void HyperEllipsoidShape4DResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radii", "radii"), &HyperEllipsoidShape4DResource::set_radii);
	ClassDB::bind_method(D_METHOD("get_radii"), &HyperEllipsoidShape4DResource::get_radii);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "radii"), "set_radii", "get_radii");
}
