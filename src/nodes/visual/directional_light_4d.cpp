#include "directional_light_4d.h"
#include "../camera_4d.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/directional_light3d.hpp>

using namespace godot;

void DirectionalLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_directional_shadow_max_distance"), &DirectionalLight4D::get_directional_shadow_max_distance);
	ClassDB::bind_method(D_METHOD("set_directional_shadow_max_distance", "distance"), &DirectionalLight4D::set_directional_shadow_max_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "directional_shadow_max_distance", PROPERTY_HINT_RANGE, "0,8192,0.1,or_greater"), "set_directional_shadow_max_distance", "get_directional_shadow_max_distance");

	ClassDB::bind_method(D_METHOD("get_directional_shadow_fade_start"), &DirectionalLight4D::get_directional_shadow_fade_start);
	ClassDB::bind_method(D_METHOD("set_directional_shadow_fade_start", "fade"), &DirectionalLight4D::set_directional_shadow_fade_start);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "directional_shadow_fade_start", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_directional_shadow_fade_start", "get_directional_shadow_fade_start");

	ClassDB::bind_method(D_METHOD("get_directional_shadow_pancake_size"), &DirectionalLight4D::get_directional_shadow_pancake_size);
	ClassDB::bind_method(D_METHOD("set_directional_shadow_pancake_size", "size"), &DirectionalLight4D::set_directional_shadow_pancake_size);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "directional_shadow_pancake_size", PROPERTY_HINT_RANGE, "0,1024,0.1,or_greater"), "set_directional_shadow_pancake_size", "get_directional_shadow_pancake_size");
}

Light3D *DirectionalLight4D::_create_light_node() {
	return memnew(DirectionalLight3D);
}

void DirectionalLight4D::_project_light() {
	if (!_internal_light) return;

	// Find active Camera4D via the current Camera3D
	Viewport *vp = get_viewport();
	if (!vp) return;
	Camera3D *cam3 = vp->get_camera_3d();
	Camera4D *cam4 = nullptr;
	if (cam3) {
		cam4 = Object::cast_to<Camera4D>(cam3->get_parent());
	}

	if (!cam4) {
		// No camera - point light straight down as default
		_internal_light->set_rotation(Vector3(-Math_PI / 2.0f, 0.0f, 0.0f));
		return;
	}

	// Get camera basis for projecting directions
	Ref<Transform4D> cam_gt = cam4->get_global_transform_4d();
	if (cam_gt.is_null()) return;
	Ref<Basis4D> cam_basis = cam_gt->get_basis();
	if (cam_basis.is_null()) return;

	// Get the light's 4D global basis
	Ref<Transform4D> light_gt = get_global_transform_4d();
	if (light_gt.is_null()) return;
	Ref<Basis4D> light_basis = light_gt->get_basis();
	if (light_basis.is_null()) return;

	// Project the light's forward direction (column 2, negated = "toward") to 3D
	Ref<Vector4D> fwd4 = light_basis->get_column(2);
	if (fwd4.is_null()) return;

	// Negate for "toward" direction
	float dx = -fwd4->x;
	float dy = -fwd4->y;
	float dz = -fwd4->z;
	float dw = -fwd4->w;

	// Project direction into camera's 3D space using camera basis columns 0,1,2
	Ref<Vector4D> col0 = cam_basis->get_column(0);
	Ref<Vector4D> col1 = cam_basis->get_column(1);
	Ref<Vector4D> col2 = cam_basis->get_column(2);
	if (col0.is_null() || col1.is_null() || col2.is_null()) return;

	float dir_x = col0->x * dx + col0->y * dy + col0->z * dz + col0->w * dw;
	float dir_y = col1->x * dx + col1->y * dy + col1->z * dz + col1->w * dw;
	float dir_z = col2->x * dx + col2->y * dy + col2->z * dz + col2->w * dw;

	Vector3 dir3(dir_x, dir_y, dir_z);
	float len = dir3.length();
	if (len < 1e-6f) return;
	dir3 /= len;

	// Directional lights: only rotation matters.
	// Place the light far away opposite to the direction it shines, looking at origin.
	Vector3 light_pos = -dir3 * 1000.0f;
	_internal_light->set_global_position(light_pos);
	_internal_light->look_at(Vector3(0, 0, 0), Vector3(0, 1, 0));

	// Apply directional-specific shadow params
	DirectionalLight3D *dir_light = Object::cast_to<DirectionalLight3D>(_internal_light);
	if (dir_light) {
		dir_light->set_param(Light3D::PARAM_SHADOW_MAX_DISTANCE, _directional_shadow_max_distance);
		dir_light->set_param(Light3D::PARAM_SHADOW_FADE_START, _directional_shadow_fade_start);
		dir_light->set_param(Light3D::PARAM_SHADOW_PANCAKE_SIZE, _directional_shadow_pancake_size);
	}
}

void DirectionalLight4D::set_directional_shadow_max_distance(real_t p_distance) {
	_directional_shadow_max_distance = p_distance;
	DirectionalLight3D *dir_light = Object::cast_to<DirectionalLight3D>(_internal_light);
	if (dir_light) dir_light->set_param(Light3D::PARAM_SHADOW_MAX_DISTANCE, _directional_shadow_max_distance);
}

void DirectionalLight4D::set_directional_shadow_fade_start(real_t p_fade) {
	_directional_shadow_fade_start = p_fade;
	DirectionalLight3D *dir_light = Object::cast_to<DirectionalLight3D>(_internal_light);
	if (dir_light) dir_light->set_param(Light3D::PARAM_SHADOW_FADE_START, _directional_shadow_fade_start);
}

void DirectionalLight4D::set_directional_shadow_pancake_size(real_t p_size) {
	_directional_shadow_pancake_size = p_size;
	DirectionalLight3D *dir_light = Object::cast_to<DirectionalLight3D>(_internal_light);
	if (dir_light) dir_light->set_param(Light3D::PARAM_SHADOW_PANCAKE_SIZE, _directional_shadow_pancake_size);
}
