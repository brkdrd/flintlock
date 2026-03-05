#include "material_4d.h"

void Material4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_albedo_color"), &Material4D::get_albedo_color);
	ClassDB::bind_method(D_METHOD("set_albedo_color", "color"), &Material4D::set_albedo_color);
	ClassDB::bind_method(D_METHOD("get_albedo_texture"), &Material4D::get_albedo_texture);
	ClassDB::bind_method(D_METHOD("set_albedo_texture", "texture"), &Material4D::set_albedo_texture);
	ClassDB::bind_method(D_METHOD("get_metallic"), &Material4D::get_metallic);
	ClassDB::bind_method(D_METHOD("set_metallic", "metallic"), &Material4D::set_metallic);
	ClassDB::bind_method(D_METHOD("get_roughness"), &Material4D::get_roughness);
	ClassDB::bind_method(D_METHOD("set_roughness", "roughness"), &Material4D::set_roughness);
	ClassDB::bind_method(D_METHOD("get_emission"), &Material4D::get_emission);
	ClassDB::bind_method(D_METHOD("set_emission", "emission"), &Material4D::set_emission);
	ClassDB::bind_method(D_METHOD("get_emission_energy"), &Material4D::get_emission_energy);
	ClassDB::bind_method(D_METHOD("set_emission_energy", "energy"), &Material4D::set_emission_energy);
	ClassDB::bind_method(D_METHOD("get_standard_material"), &Material4D::get_standard_material);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "albedo_color"), "set_albedo_color", "get_albedo_color");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "albedo_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_albedo_texture", "get_albedo_texture");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "metallic", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_metallic", "get_metallic");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "roughness", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_roughness", "get_roughness");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "emission"), "set_emission", "get_emission");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "emission_energy", PROPERTY_HINT_RANGE, "0,16,0.01"), "set_emission_energy", "get_emission_energy");
}

void Material4D::set_albedo_color(const Color &p_color) {
	_albedo_color = p_color;
	_material_dirty = true;
	emit_changed();
}

void Material4D::set_albedo_texture(const Ref<Texture2D> &p_texture) {
	_albedo_texture = p_texture;
	_material_dirty = true;
	emit_changed();
}

void Material4D::set_metallic(real_t p_metallic) {
	_metallic = p_metallic;
	_material_dirty = true;
	emit_changed();
}

void Material4D::set_roughness(real_t p_roughness) {
	_roughness = p_roughness;
	_material_dirty = true;
	emit_changed();
}

void Material4D::set_emission(const Color &p_emission) {
	_emission = p_emission;
	_material_dirty = true;
	emit_changed();
}

void Material4D::set_emission_energy(real_t p_energy) {
	_emission_energy = p_energy;
	_material_dirty = true;
	emit_changed();
}

Ref<StandardMaterial3D> Material4D::get_standard_material() const {
	if (_material_dirty || _cached_material.is_null()) {
		_cached_material.instantiate();
		_cached_material->set_albedo(_albedo_color);
		if (_albedo_texture.is_valid()) {
			_cached_material->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, _albedo_texture);
		}
		_cached_material->set_metallic(_metallic);
		_cached_material->set_roughness(_roughness);
		if (_emission != Color(0, 0, 0, 0) || _emission_energy > 0.0f) {
			_cached_material->set_feature(StandardMaterial3D::FEATURE_EMISSION, true);
			_cached_material->set_emission(_emission);
			_cached_material->set_emission_energy_multiplier(_emission_energy);
		}
		_material_dirty = false;
	}
	return _cached_material;
}
