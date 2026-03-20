#include "omni_light_4d.h"
#include "../../servers/visual/visual_server_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>

using namespace godot;

void OmniLight4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_omni_range"), &OmniLight4D::get_omni_range);
	ClassDB::bind_method(D_METHOD("set_omni_range", "range"), &OmniLight4D::set_omni_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_range", PROPERTY_HINT_RANGE, "0,4096,0.01,or_greater,suffix:m"), "set_omni_range", "get_omni_range");

	ClassDB::bind_method(D_METHOD("get_omni_attenuation"), &OmniLight4D::get_omni_attenuation);
	ClassDB::bind_method(D_METHOD("set_omni_attenuation", "attenuation"), &OmniLight4D::set_omni_attenuation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "omni_attenuation", PROPERTY_HINT_EXP_EASING, "attenuation"), "set_omni_attenuation", "get_omni_attenuation");
}

void OmniLight4D::_notification(int p_what) {
	Light4D::_notification(p_what);

	if (p_what == NOTIFICATION_ENTER_TREE) {
		VisualServer4D *vs = VisualServer4D::get_singleton();
		if (vs && _vs_light.is_valid()) {
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_RANGE, _omni_range);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ATTENUATION, _omni_attenuation);
		}
	}
}

void OmniLight4D::set_omni_range(real_t p_range) {
	_omni_range = p_range;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_RANGE, _omni_range);
}

void OmniLight4D::set_omni_attenuation(real_t p_attenuation) {
	_omni_attenuation = p_attenuation;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_ATTENUATION, _omni_attenuation);
}
