#include "shape_4d_resource.h"

Shape4DResource::Shape4DResource(Type p_type) : type(p_type) {
	switch (type) {
		case TYPE_HYPER_SPHERE:
			shape = new HyperSphereShape4D(1.0);
			break;
		case TYPE_HYPER_BOX:
			shape = new HyperBoxShape4D(Vector4(1, 1, 1, 1));
			break;
		// TODO: Add other shape types when needed
		default:
			shape = nullptr;
			break;
	}
}

Shape4DResource::~Shape4DResource() {
	if (shape) {
		delete shape;
		shape = nullptr;
	}
}

void Shape4DResource::set_sphere_radius(real_t p_radius) {
	if (type == TYPE_HYPER_SPHERE) {
		HyperSphereShape4D *sphere = static_cast<HyperSphereShape4D*>(shape);
		sphere->set_radius(p_radius);
	}
}

void Shape4DResource::set_box_half_extents(const Vector4 &p_extents) {
	if (type == TYPE_HYPER_BOX) {
		HyperBoxShape4D *box = static_cast<HyperBoxShape4D*>(shape);
		box->set_half_extents(p_extents);
	}
}
