#include "geometry_instance_4d.h"

#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

GeometryInstance4D::GeometryInstance4D() {}
GeometryInstance4D::~GeometryInstance4D() {}

void GeometryInstance4D::set_material(const Ref<Material> &p_material) {
	material = p_material;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && instance_rid.is_valid()) {
		RID mat_rid = material.is_valid() ? material->get_rid() : RID();
		rs->instance_geometry_set_material_override(instance_rid, mat_rid);
	}
}

Ref<Material> GeometryInstance4D::get_material() const {
	return material;
}

void GeometryInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_material", "material"), &GeometryInstance4D::set_material);
	ClassDB::bind_method(D_METHOD("get_material"), &GeometryInstance4D::get_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"),
			"set_material", "get_material");
}
