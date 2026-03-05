#include "environment_4d.h"

void Environment4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_ambient_light_color"), &Environment4D::get_ambient_light_color);
	ClassDB::bind_method(D_METHOD("set_ambient_light_color", "color"), &Environment4D::set_ambient_light_color);
	ClassDB::bind_method(D_METHOD("get_ambient_light_energy"), &Environment4D::get_ambient_light_energy);
	ClassDB::bind_method(D_METHOD("set_ambient_light_energy", "energy"), &Environment4D::set_ambient_light_energy);
	ClassDB::bind_method(D_METHOD("get_fog_enabled"), &Environment4D::get_fog_enabled);
	ClassDB::bind_method(D_METHOD("set_fog_enabled", "enabled"), &Environment4D::set_fog_enabled);
	ClassDB::bind_method(D_METHOD("get_fog_density"), &Environment4D::get_fog_density);
	ClassDB::bind_method(D_METHOD("set_fog_density", "density"), &Environment4D::set_fog_density);
	ClassDB::bind_method(D_METHOD("get_fog_color"), &Environment4D::get_fog_color);
	ClassDB::bind_method(D_METHOD("set_fog_color", "color"), &Environment4D::set_fog_color);
	ClassDB::bind_method(D_METHOD("get_background_mode"), &Environment4D::get_background_mode);
	ClassDB::bind_method(D_METHOD("set_background_mode", "mode"), &Environment4D::set_background_mode);
	ClassDB::bind_method(D_METHOD("get_environment"), &Environment4D::get_environment);

	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "ambient_light_color"), "set_ambient_light_color", "get_ambient_light_color");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "ambient_light_energy", PROPERTY_HINT_RANGE, "0,16,0.01"), "set_ambient_light_energy", "get_ambient_light_energy");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "fog_enabled"), "set_fog_enabled", "get_fog_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fog_density", PROPERTY_HINT_RANGE, "0,1,0.0001"), "set_fog_density", "get_fog_density");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "fog_color"), "set_fog_color", "get_fog_color");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "background_mode", PROPERTY_HINT_ENUM, "Sky,Color,Canvas"), "set_background_mode", "get_background_mode");
}

void Environment4D::set_ambient_light_color(const Color &p_color) { _ambient_light_color = p_color; _env_dirty = true; emit_changed(); }
void Environment4D::set_ambient_light_energy(real_t p_energy) { _ambient_light_energy = p_energy; _env_dirty = true; emit_changed(); }
void Environment4D::set_fog_enabled(bool p_enabled) { _fog_enabled = p_enabled; _env_dirty = true; emit_changed(); }
void Environment4D::set_fog_density(real_t p_density) { _fog_density = p_density; _env_dirty = true; emit_changed(); }
void Environment4D::set_fog_color(const Color &p_color) { _fog_color = p_color; _env_dirty = true; emit_changed(); }
void Environment4D::set_background_mode(int p_mode) { _background_mode = p_mode; _env_dirty = true; emit_changed(); }

Ref<Environment> Environment4D::get_environment() const {
	if (_env_dirty || _cached_env.is_null()) {
		_cached_env.instantiate();
		_cached_env->set_ambient_light_color(_ambient_light_color);
		_cached_env->set_ambient_light_energy(_ambient_light_energy);
		_cached_env->set_fog_enabled(_fog_enabled);
		if (_fog_enabled) {
			_cached_env->set_fog_density(_fog_density);
		}
		_env_dirty = false;
	}
	return _cached_env;
}
