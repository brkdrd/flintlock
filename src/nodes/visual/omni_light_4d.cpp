#include "omni_light_4d.h"
#include "../camera_4d.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/omni_light3d.hpp>

using namespace godot;

void OmniLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_omni_range"), &OmniLight4D::get_omni_range);
	ClassDB::bind_method(D_METHOD("set_omni_range", "range"), &OmniLight4D::set_omni_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_range", PROPERTY_HINT_RANGE, "0,4096,0.01,or_greater,suffix:m"), "set_omni_range", "get_omni_range");

	ClassDB::bind_method(D_METHOD("get_omni_attenuation"), &OmniLight4D::get_omni_attenuation);
	ClassDB::bind_method(D_METHOD("set_omni_attenuation", "attenuation"), &OmniLight4D::set_omni_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_omni_attenuation", "get_omni_attenuation");
}

Light3D *OmniLight4D::_create_light_node() {
	return memnew(OmniLight3D);
}

void OmniLight4D::_project_light() {
	if (!_internal_light) return;

	// Call parent to set 3D position
	Light4D::_project_light();

	// Find camera to compute W-distance for range attenuation
	Viewport *vp = get_viewport();
	Camera3D *cam3 = vp ? vp->get_camera_3d() : nullptr;
	Camera4D *cam4 = cam3 ? Object::cast_to<Camera4D>(cam3->get_parent()) : nullptr;

	OmniLight3D *omni = Object::cast_to<OmniLight3D>(_internal_light);
	if (!omni) return;

	if (cam4) {
		// Get distance along the slice plane normal (the W-component distance)
		Vector4 plane_normal = cam4->get_slice_plane_normal();
		float plane_d = cam4->get_slice_plane_distance();

		Ref<Transform4D> gt = get_global_transform_4d();
		if (gt.is_null()) {
			omni->set_param(Light3D::PARAM_RANGE, _omni_range);
			return;
		}
		Ref<Vector4D> pos = gt->get_origin();
		if (pos.is_null()) {
			omni->set_param(Light3D::PARAM_RANGE, _omni_range);
			return;
		}

		Vector4 pos4(pos->x, pos->y, pos->z, pos->w);
		float w_dist = Math::abs(plane_normal.dot(pos4) - plane_d);
		float range_sq = _omni_range * _omni_range - w_dist * w_dist;

		if (range_sq < 0.0f) {
			// Light is beyond its range in W - hide it
			_internal_light->set_visible(false);
		} else {
			_internal_light->set_visible(true);
			omni->set_param(Light3D::PARAM_RANGE, Math::sqrt(range_sq));
		}
	} else {
		// No camera, use full range
		omni->set_param(Light3D::PARAM_RANGE, _omni_range);
	}

	omni->set_param(Light3D::PARAM_ATTENUATION, _omni_attenuation);
}

void OmniLight4D::set_omni_range(real_t p_range) {
	_omni_range = p_range;
	// Actual range on OmniLight3D is set in _project_light() per-frame
}

void OmniLight4D::set_omni_attenuation(real_t p_attenuation) {
	_omni_attenuation = p_attenuation;
	OmniLight3D *omni = Object::cast_to<OmniLight3D>(_internal_light);
	if (omni) omni->set_param(Light3D::PARAM_ATTENUATION, _omni_attenuation);
}
