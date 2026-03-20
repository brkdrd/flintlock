#include "directional_light_4d.h"
#include "../../servers/visual/visual_server_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>

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

void DirectionalLight4D::_notification(int p_what) {
	Light4D::_notification(p_what);

	if (p_what == NOTIFICATION_ENTER_TREE) {
		// Apply directional-specific shadow params
		VisualServer4D *vs = VisualServer4D::get_singleton();
		if (vs && _vs_light.is_valid()) {
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_MAX_DISTANCE, _directional_shadow_max_distance);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_FADE_START, _directional_shadow_fade_start);
			vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_PANCAKE_SIZE, _directional_shadow_pancake_size);
		}
	}
}

void DirectionalLight4D::set_directional_shadow_max_distance(real_t p_distance) {
	_directional_shadow_max_distance = p_distance;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_MAX_DISTANCE, _directional_shadow_max_distance);
}

void DirectionalLight4D::set_directional_shadow_fade_start(real_t p_fade) {
	_directional_shadow_fade_start = p_fade;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_FADE_START, _directional_shadow_fade_start);
}

void DirectionalLight4D::set_directional_shadow_pancake_size(real_t p_size) {
	_directional_shadow_pancake_size = p_size;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_light.is_valid()) vs->light_set_param(_vs_light, RenderingServer::LIGHT_PARAM_SHADOW_PANCAKE_SIZE, _directional_shadow_pancake_size);
}
