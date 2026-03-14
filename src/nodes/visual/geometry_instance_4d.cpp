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
	apply_material_params();
}

void GeometryInstance4D::set_transparency(real_t p_transparency) {
	_transparency = p_transparency;
}

Ref<Material> GeometryInstance4D::get_active_material_3d() const {
	if (_material_override.is_valid()) {
		return _material_override->get_standard_material();
	}
	return Ref<Material>();
}

void GeometryInstance4D::apply_material_params() {
	if (!_rs_material.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();

	Color albedo(1.0f, 1.0f, 1.0f, 1.0f);
	float roughness = 1.0f;
	float metallic = 0.0f;

	if (_material_override.is_valid()) {
		albedo = _material_override->get_albedo_color();
		roughness = _material_override->get_roughness();
		metallic = _material_override->get_metallic();
	}

	rs->material_set_param(_rs_material, "albedo_color", albedo);
	rs->material_set_param(_rs_material, "roughness_value", roughness);
	rs->material_set_param(_rs_material, "metallic_value", metallic);
}
