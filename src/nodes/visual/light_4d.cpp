#include "light_4d.h"
#include "../../servers/visual/visual_server_4d.h"
#include "../../math/transform4d.h"
#include "../../math/basis4d.h"
#include "../../math/vector4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/world3d.hpp>

using namespace godot;

void Light4D::_bind_methods() {
	// Color
	ClassDB::bind_method(D_METHOD("get_light_color"), &Light4D::get_light_color);
	ClassDB::bind_method(D_METHOD("set_light_color", "color"), &Light4D::set_light_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "light_color"), "set_light_color", "get_light_color");

	// Energy
	ClassDB::bind_method(D_METHOD("get_light_energy"), &Light4D::get_light_energy);
	ClassDB::bind_method(D_METHOD("set_light_energy", "energy"), &Light4D::set_light_energy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_energy", PROPERTY_HINT_RANGE, "0,16,0.001,or_greater"), "set_light_energy", "get_light_energy");

	ClassDB::bind_method(D_METHOD("get_light_indirect_energy"), &Light4D::get_light_indirect_energy);
	ClassDB::bind_method(D_METHOD("set_light_indirect_energy", "energy"), &Light4D::set_light_indirect_energy);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_indirect_energy", PROPERTY_HINT_RANGE, "0,16,0.001,or_greater"), "set_light_indirect_energy", "get_light_indirect_energy");

	// Specular
	ClassDB::bind_method(D_METHOD("get_light_specular"), &Light4D::get_light_specular);
	ClassDB::bind_method(D_METHOD("set_light_specular", "specular"), &Light4D::set_light_specular);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "light_specular", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_light_specular", "get_light_specular");

	// Shadow
	ClassDB::bind_method(D_METHOD("get_shadow_enabled"), &Light4D::get_shadow_enabled);
	ClassDB::bind_method(D_METHOD("set_shadow_enabled", "enabled"), &Light4D::set_shadow_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shadow_enabled"), "set_shadow_enabled", "get_shadow_enabled");

	ClassDB::bind_method(D_METHOD("get_shadow_bias"), &Light4D::get_shadow_bias);
	ClassDB::bind_method(D_METHOD("set_shadow_bias", "bias"), &Light4D::set_shadow_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_bias", PROPERTY_HINT_RANGE, "0,10,0.001"), "set_shadow_bias", "get_shadow_bias");

	ClassDB::bind_method(D_METHOD("get_shadow_normal_bias"), &Light4D::get_shadow_normal_bias);
	ClassDB::bind_method(D_METHOD("set_shadow_normal_bias", "bias"), &Light4D::set_shadow_normal_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_normal_bias", PROPERTY_HINT_RANGE, "0,10,0.001"), "set_shadow_normal_bias", "get_shadow_normal_bias");

	ClassDB::bind_method(D_METHOD("get_shadow_opacity"), &Light4D::get_shadow_opacity);
	ClassDB::bind_method(D_METHOD("set_shadow_opacity", "opacity"), &Light4D::set_shadow_opacity);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_opacity", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_shadow_opacity", "get_shadow_opacity");

	ClassDB::bind_method(D_METHOD("get_shadow_blur"), &Light4D::get_shadow_blur);
	ClassDB::bind_method(D_METHOD("set_shadow_blur", "blur"), &Light4D::set_shadow_blur);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shadow_blur", PROPERTY_HINT_RANGE, "0,10,0.001"), "set_shadow_blur", "get_shadow_blur");

	// Cull mask
	ClassDB::bind_method(D_METHOD("get_light_cull_mask"), &Light4D::get_light_cull_mask);
	ClassDB::bind_method(D_METHOD("set_light_cull_mask", "mask"), &Light4D::set_light_cull_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "light_cull_mask", PROPERTY_HINT_LAYERS_3D_RENDER), "set_light_cull_mask", "get_light_cull_mask");

	// Flags
	ClassDB::bind_method(D_METHOD("get_light_negative"), &Light4D::get_light_negative);
	ClassDB::bind_method(D_METHOD("set_light_negative", "negative"), &Light4D::set_light_negative);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "light_negative"), "set_light_negative", "get_light_negative");

	ClassDB::bind_method(D_METHOD("get_editor_only"), &Light4D::get_editor_only);
	ClassDB::bind_method(D_METHOD("set_editor_only", "editor_only"), &Light4D::set_editor_only);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "editor_only"), "set_editor_only", "get_editor_only");
}

void Light4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs) return;

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_vs_light = vs->light_create(_get_light_type());

			// Set scenario from viewport
			Viewport *vp = get_viewport();
			if (vp) {
				Ref<World3D> world = vp->find_world_3d();
				if (world.is_valid()) {
					vs->light_set_scenario(_vs_light, world->get_scenario());
				}
			}

			_update_light_properties();
			_update_light_transform();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (_vs_light.is_valid()) {
				vs->free_rid(_vs_light);
				_vs_light = RID();
			}
		} break;

	}
}

void Light4D::_on_transform_4d_changed() {
	_update_light_transform();
}

void Light4D::_update_light_transform() {
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs || !_vs_light.is_valid()) return;

	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return;
	Ref<Basis4D> basis = gt->get_basis();
	Ref<Vector4D> origin = gt->get_origin();
	if (basis.is_null() || origin.is_null()) return;

	PackedFloat32Array basis_cols;
	basis_cols.resize(16);
	for (int col = 0; col < 4; col++) {
		basis_cols[col * 4 + 0] = basis->data[col][0];
		basis_cols[col * 4 + 1] = basis->data[col][1];
		basis_cols[col * 4 + 2] = basis->data[col][2];
		basis_cols[col * 4 + 3] = basis->data[col][3];
	}

	Vector4 orig(origin->x, origin->y, origin->z, origin->w);
	vs->light_set_transform_4d(_vs_light, basis_cols, orig);
}

void Light4D::_update_light_properties() {
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs || !_vs_light.is_valid()) return;

	vs->light_set_color(_vs_light, _light_color);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ENERGY, _light_energy);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_INDIRECT_ENERGY, _light_indirect_energy);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPECULAR, _light_specular);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_BIAS, _shadow_bias);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_NORMAL_BIAS, _shadow_normal_bias);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_OPACITY, _shadow_opacity);
	vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_BLUR, _shadow_blur);
	vs->light_set_shadow(_vs_light, _shadow_enabled);
	vs->light_set_negative(_vs_light, _light_negative);
	vs->light_set_cull_mask(_vs_light, _light_cull_mask);
}

// ── Setters ─────────────────────────────────────────────────────────────────

void Light4D::set_light_color(const Color &p_color) {
	_light_color = p_color;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_color(_vs_light, _light_color);
}

void Light4D::set_light_energy(real_t p_energy) {
	_light_energy = p_energy;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ENERGY, _light_energy);
}

void Light4D::set_light_indirect_energy(real_t p_energy) {
	_light_indirect_energy = p_energy;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_INDIRECT_ENERGY, _light_indirect_energy);
}

void Light4D::set_light_specular(real_t p_specular) {
	_light_specular = p_specular;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPECULAR, _light_specular);
}

void Light4D::set_shadow_enabled(bool p_enabled) {
	_shadow_enabled = p_enabled;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_shadow(_vs_light, _shadow_enabled);
}

void Light4D::set_shadow_bias(real_t p_bias) {
	_shadow_bias = p_bias;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_BIAS, _shadow_bias);
}

void Light4D::set_shadow_normal_bias(real_t p_bias) {
	_shadow_normal_bias = p_bias;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_NORMAL_BIAS, _shadow_normal_bias);
}

void Light4D::set_shadow_opacity(real_t p_opacity) {
	_shadow_opacity = p_opacity;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_OPACITY, _shadow_opacity);
}

void Light4D::set_shadow_blur(real_t p_blur) {
	_shadow_blur = p_blur;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_BLUR, _shadow_blur);
}

void Light4D::set_light_cull_mask(uint32_t p_mask) {
	_light_cull_mask = p_mask;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_cull_mask(_vs_light, _light_cull_mask);
}

void Light4D::set_light_negative(bool p_negative) {
	_light_negative = p_negative;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_negative(_vs_light, _light_negative);
}

void Light4D::set_editor_only(bool p_editor_only) {
	_editor_only = p_editor_only;
	// editor_only not directly supported via RS light API
}
