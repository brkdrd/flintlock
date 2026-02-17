#include "mesh_instance_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

MeshInstance4D::MeshInstance4D() {}
MeshInstance4D::~MeshInstance4D() {}

void MeshInstance4D::set_shape(const Ref<Shape4DResourceBase> &p_shape) {
	shape = p_shape;
}

Ref<Shape4DResourceBase> MeshInstance4D::get_shape() const {
	return shape;
}

void MeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &MeshInstance4D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &MeshInstance4D::get_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape4DResourceBase"),
			"set_shape", "get_shape");
}
