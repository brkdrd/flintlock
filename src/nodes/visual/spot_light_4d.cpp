#include "spot_light_4d.h"
#include "../camera_4d.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/spot_light3d.hpp>

using namespace godot;

void SpotLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_spot_range"), &SpotLight4D::get_spot_range);
	ClassDB::bind_method(D_METHOD("set_spot_range", "range"), &SpotLight4D::set_spot_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_range", PROPERTY_HINT_RANGE, "0,4096,0.01,or_greater,suffix:m"), "set_spot_range", "get_spot_range");

	ClassDB::bind_method(D_METHOD("get_spot_attenuation"), &SpotLight4D::get_spot_attenuation);
	ClassDB::bind_method(D_METHOD("set_spot_attenuation", "attenuation"), &SpotLight4D::set_spot_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_spot_attenuation", "get_spot_attenuation");

	ClassDB::bind_method(D_METHOD("get_spot_angle"), &SpotLight4D::get_spot_angle);
	ClassDB::bind_method(D_METHOD("set_spot_angle", "angle"), &SpotLight4D::set_spot_angle);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_angle", PROPERTY_HINT_RANGE, "0,90,0.1,degrees"), "set_spot_angle", "get_spot_angle");

	ClassDB::bind_method(D_METHOD("get_spot_angle_attenuation"), &SpotLight4D::get_spot_angle_attenuation);
	ClassDB::bind_method(D_METHOD("set_spot_angle_attenuation", "attenuation"), &SpotLight4D::set_spot_angle_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spot_angle_attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_spot_angle_attenuation", "get_spot_angle_attenuation");
}

Light3D *SpotLight4D::_create_light_node() {
	return memnew(SpotLight3D);
}

void SpotLight4D::_project_light() {
	if (!_internal_light) return;

	// Call parent to set 3D position
	Light4D::_project_light();

	// Find camera for W-distance calculation
	Viewport *vp = get_viewport();
	Camera3D *cam3 = vp ? vp->get_camera_3d() : nullptr;
	Camera4D *cam4 = cam3 ? Object::cast_to<Camera4D>(cam3->get_parent()) : nullptr;

	SpotLight3D *spot = Object::cast_to<SpotLight3D>(_internal_light);
	if (!spot) return;

	// Set spot parameters
	spot->set_param(Light3D::PARAM_SPOT_ANGLE, _spot_angle);
	spot->set_param(Light3D::PARAM_SPOT_ATTENUATION, _spot_angle_attenuation);
	spot->set_param(Light3D::PARAM_ATTENUATION, _spot_attenuation);

	if (cam4) {
		// W-distance attenuation like OmniLight4D
		Vector4 plane_normal = cam4->get_slice_plane_normal();
		float plane_d = cam4->get_slice_plane_distance();

		Ref<Transform4D> gt = get_global_transform_4d();
		if (gt.is_null()) {
			spot->set_param(Light3D::PARAM_RANGE, _spot_range);
			return;
		}
		Ref<Vector4D> pos = gt->get_origin();
		if (pos.is_null()) {
			spot->set_param(Light3D::PARAM_RANGE, _spot_range);
			return;
		}

		Vector4 pos4(pos->x, pos->y, pos->z, pos->w);
		float w_dist = Math::abs(plane_normal.dot(pos4) - plane_d);
		float range_sq = _spot_range * _spot_range - w_dist * w_dist;

		if (range_sq < 0.0f) {
			_internal_light->set_visible(false);
		} else {
			_internal_light->set_visible(true);
			spot->set_param(Light3D::PARAM_RANGE, Math::sqrt(range_sq));
		}

		// Also project the spot direction to 3D
		Ref<Transform4D> cam_gt = cam4->get_global_transform_4d();
		if (!cam_gt.is_null()) {
			Ref<Basis4D> cam_basis = cam_gt->get_basis();
			Ref<Basis4D> light_basis = get_global_transform_4d()->get_basis();
			if (!cam_basis.is_null() && !light_basis.is_null()) {
				// Spot light points along its local -Z in 4D
				Ref<Vector4D> fwd4 = light_basis->get_column(2);
				if (!fwd4.is_null()) {
					float dx = -fwd4->x;
					float dy = -fwd4->y;
					float dz = -fwd4->z;
					float dw = -fwd4->w;

					Ref<Vector4D> col0 = cam_basis->get_column(0);
					Ref<Vector4D> col1 = cam_basis->get_column(1);
					Ref<Vector4D> col2 = cam_basis->get_column(2);
					if (!col0.is_null() && !col1.is_null() && !col2.is_null()) {
						float dir_x = col0->x * dx + col0->y * dy + col0->z * dz + col0->w * dw;
						float dir_y = col1->x * dx + col1->y * dy + col1->z * dz + col1->w * dw;
						float dir_z = col2->x * dx + col2->y * dy + col2->z * dz + col2->w * dw;
						Vector3 dir3(dir_x, dir_y, dir_z);
						float dir_len = dir3.length();
						if (dir_len > 1e-6f) {
							dir3 /= dir_len;
							Vector3 spot_pos = _internal_light->get_position();
							Vector3 target = spot_pos + dir3;
							if ((target - spot_pos).length() > 1e-6f) {
								_internal_light->look_at(target, Vector3(0, 1, 0));
							}
						}
					}
				}
			}
		}
	} else {
		spot->set_param(Light3D::PARAM_RANGE, _spot_range);
	}
}

void SpotLight4D::set_spot_range(real_t p_range) {
	_spot_range = p_range;
	// Applied per-frame in _project_light()
}

void SpotLight4D::set_spot_attenuation(real_t p_attenuation) {
	_spot_attenuation = p_attenuation;
	SpotLight3D *spot = Object::cast_to<SpotLight3D>(_internal_light);
	if (spot) spot->set_param(Light3D::PARAM_ATTENUATION, _spot_attenuation);
}

void SpotLight4D::set_spot_angle(real_t p_angle) {
	_spot_angle = p_angle;
	SpotLight3D *spot = Object::cast_to<SpotLight3D>(_internal_light);
	if (spot) spot->set_param(Light3D::PARAM_SPOT_ANGLE, _spot_angle);
}

void SpotLight4D::set_spot_angle_attenuation(real_t p_attenuation) {
	_spot_angle_attenuation = p_attenuation;
	SpotLight3D *spot = Object::cast_to<SpotLight3D>(_internal_light);
	if (spot) spot->set_param(Light3D::PARAM_SPOT_ATTENUATION, _spot_angle_attenuation);
}
