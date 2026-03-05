#include "light_4d.h"
#include "../camera_4d.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/light3d.hpp>

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
	VisualInstance4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_internal_light = _create_light_node();
			if (_internal_light) {
				add_child(_internal_light, false, INTERNAL_MODE_FRONT);
				_update_light_properties();
			}
			set_process(true);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (_internal_light) {
				_internal_light->queue_free();
				_internal_light = nullptr;
			}
			set_process(false);
		} break;

		case NOTIFICATION_PROCESS: {
			_project_light();
		} break;
	}
}

void Light4D::_project_light() {
	if (!_internal_light) return;

	// Find active Camera4D via the current Camera3D
	Viewport *vp = get_viewport();
	if (!vp) return;
	Camera3D *cam3 = vp->get_camera_3d();
	Camera4D *cam4 = nullptr;
	if (cam3) {
		cam4 = Object::cast_to<Camera4D>(cam3->get_parent());
	}

	// Get 4D position of light
	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return;
	Ref<Vector4D> pos4d = gt->get_origin();
	if (pos4d.is_null()) return;

	if (!cam4) {
		// No camera found - just zero the position
		_internal_light->set_position(Vector3());
		return;
	}

	// Get camera's global transform for projection
	Ref<Transform4D> cam_gt = cam4->get_global_transform_4d();
	if (cam_gt.is_null()) return;
	Ref<Basis4D> cam_basis = cam_gt->get_basis();
	Ref<Vector4D> cam_origin = cam_gt->get_origin();
	if (cam_basis.is_null() || cam_origin.is_null()) return;

	// Relative position from camera
	float rx = pos4d->x - cam_origin->x;
	float ry = pos4d->y - cam_origin->y;
	float rz = pos4d->z - cam_origin->z;
	float rw = pos4d->w - cam_origin->w;

	// Project to 3D using camera basis columns 0,1,2
	Ref<Vector4D> col0 = cam_basis->get_column(0);
	Ref<Vector4D> col1 = cam_basis->get_column(1);
	Ref<Vector4D> col2 = cam_basis->get_column(2);
	if (col0.is_null() || col1.is_null() || col2.is_null()) return;

	float x3 = col0->x * rx + col0->y * ry + col0->z * rz + col0->w * rw;
	float y3 = col1->x * rx + col1->y * ry + col1->z * rz + col1->w * rw;
	float z3 = col2->x * rx + col2->y * ry + col2->z * rz + col2->w * rw;

	_internal_light->set_position(Vector3(x3, y3, z3));
}

void Light4D::_update_light_properties() {
	if (!_internal_light) return;

	_internal_light->set_param(Light3D::PARAM_ENERGY, _light_energy);
	_internal_light->set_param(Light3D::PARAM_INDIRECT_ENERGY, _light_indirect_energy);
	_internal_light->set_param(Light3D::PARAM_SPECULAR, _light_specular);
	_internal_light->set_param(Light3D::PARAM_SHADOW_BIAS, _shadow_bias);
	_internal_light->set_param(Light3D::PARAM_SHADOW_NORMAL_BIAS, _shadow_normal_bias);
	_internal_light->set_param(Light3D::PARAM_SHADOW_OPACITY, _shadow_opacity);
	_internal_light->set_param(Light3D::PARAM_SHADOW_BLUR, _shadow_blur);
	_internal_light->set_color(_light_color);
	_internal_light->set_shadow(_shadow_enabled);
	_internal_light->set_editor_only(_editor_only);
	_internal_light->set_layer_mask(_light_cull_mask);
	_internal_light->set_negative(_light_negative);
}

// ─── Setters ──────────────────────────────────────────────────────────────────

void Light4D::set_light_color(const Color &p_color) {
	_light_color = p_color;
	if (_internal_light) _internal_light->set_color(_light_color);
}

void Light4D::set_light_energy(real_t p_energy) {
	_light_energy = p_energy;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_ENERGY, _light_energy);
}

void Light4D::set_light_indirect_energy(real_t p_energy) {
	_light_indirect_energy = p_energy;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_INDIRECT_ENERGY, _light_indirect_energy);
}

void Light4D::set_light_specular(real_t p_specular) {
	_light_specular = p_specular;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_SPECULAR, _light_specular);
}

void Light4D::set_shadow_enabled(bool p_enabled) {
	_shadow_enabled = p_enabled;
	if (_internal_light) _internal_light->set_shadow(_shadow_enabled);
}

void Light4D::set_shadow_bias(real_t p_bias) {
	_shadow_bias = p_bias;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_SHADOW_BIAS, _shadow_bias);
}

void Light4D::set_shadow_normal_bias(real_t p_bias) {
	_shadow_normal_bias = p_bias;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_SHADOW_NORMAL_BIAS, _shadow_normal_bias);
}

void Light4D::set_shadow_opacity(real_t p_opacity) {
	_shadow_opacity = p_opacity;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_SHADOW_OPACITY, _shadow_opacity);
}

void Light4D::set_shadow_blur(real_t p_blur) {
	_shadow_blur = p_blur;
	if (_internal_light) _internal_light->set_param(Light3D::PARAM_SHADOW_BLUR, _shadow_blur);
}

void Light4D::set_light_cull_mask(uint32_t p_mask) {
	_light_cull_mask = p_mask;
	if (_internal_light) _internal_light->set_layer_mask(_light_cull_mask);
}

void Light4D::set_light_negative(bool p_negative) {
	_light_negative = p_negative;
	if (_internal_light) _internal_light->set_negative(_light_negative);
}

void Light4D::set_editor_only(bool p_editor_only) {
	_editor_only = p_editor_only;
	if (_internal_light) _internal_light->set_editor_only(_editor_only);
}
