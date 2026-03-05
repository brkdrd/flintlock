#include "geometry_instance_4d.h"
#include "../../slicer/slicer_4d.h"

void GeometryInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_material_override"), &GeometryInstance4D::get_material_override);
	ClassDB::bind_method(D_METHOD("set_material_override", "material"), &GeometryInstance4D::set_material_override);
	ClassDB::bind_method(D_METHOD("get_transparency"), &GeometryInstance4D::get_transparency);
	ClassDB::bind_method(D_METHOD("set_transparency", "transparency"), &GeometryInstance4D::set_transparency);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material_override", PROPERTY_HINT_RESOURCE_TYPE, "Material4D"), "set_material_override", "get_material_override");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "transparency", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_transparency", "get_transparency");
}

void GeometryInstance4D::set_material_override(const Ref<Material4D> &p_material) {
	_material_override = p_material;
	if (Slicer4D::get_singleton()) {
		Slicer4D::get_singleton()->mark_dirty(this);
	}
}

void GeometryInstance4D::set_transparency(real_t p_transparency) {
	_transparency = p_transparency;
}
