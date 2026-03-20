#include "spot_light_4d.h"
#include "../../servers/visual/visual_server_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>

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

void SpotLight4D::_notification(int p_what) {
	Light4D::_notification(p_what);

	if (p_what == NOTIFICATION_ENTER_TREE) {
		VisualServer4D *vs = VisualServer4D::get_singleton();
		if (vs && _vs_light.is_valid()) {
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_RANGE, _spot_range);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ATTENUATION, _spot_attenuation);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPOT_ANGLE, _spot_angle);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPOT_ATTENUATION, _spot_angle_attenuation);
		}
	}
}

void SpotLight4D::set_spot_range(real_t p_range) {
	_spot_range = p_range;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_RANGE, _spot_range);
}

void SpotLight4D::set_spot_attenuation(real_t p_attenuation) {
	_spot_attenuation = p_attenuation;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ATTENUATION, _spot_attenuation);
}

void SpotLight4D::set_spot_angle(real_t p_angle) {
	_spot_angle = p_angle;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPOT_ANGLE, _spot_angle);
}

void SpotLight4D::set_spot_angle_attenuation(real_t p_attenuation) {
	_spot_angle_attenuation = p_attenuation;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SPOT_ATTENUATION, _spot_angle_attenuation);
}
