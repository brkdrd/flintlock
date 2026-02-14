#include "hyper_sphere_shape_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

HyperSphereShape4DResource::HyperSphereShape4DResource() {
	radius = 1.0;
}

HyperSphereShape4DResource::~HyperSphereShape4DResource() {
}

void HyperSphereShape4DResource::set_radius(real_t p_radius) {
	radius = p_radius;
	emit_changed();  // Notify that the resource has changed
}

real_t HyperSphereShape4DResource::get_radius() const {
	return radius;
}

void HyperSphereShape4DResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &HyperSphereShape4DResource::set_radius);
	ClassDB::bind_method(D_METHOD("get_radius"), &HyperSphereShape4DResource::get_radius);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.01,100.0,0.01"), "set_radius", "get_radius");
}
