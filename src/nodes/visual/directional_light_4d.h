#pragma once
#include "light_4d.h"
#include <godot_cpp/classes/directional_light3d.hpp>

using namespace godot;

class DirectionalLight4D : public Light4D {
	GDCLASS(DirectionalLight4D, Light4D);

	real_t _directional_shadow_max_distance = 100.0f;
	real_t _directional_shadow_fade_start = 0.8f;
	real_t _directional_shadow_pancake_size = 20.0f;

protected:
	Light3D *_create_light_node() override;
	void _project_light() override;
	static void _bind_methods();

public:
	real_t get_directional_shadow_max_distance() const { return _directional_shadow_max_distance; }
	void set_directional_shadow_max_distance(real_t p_distance);

	real_t get_directional_shadow_fade_start() const { return _directional_shadow_fade_start; }
	void set_directional_shadow_fade_start(real_t p_fade);

	real_t get_directional_shadow_pancake_size() const { return _directional_shadow_pancake_size; }
	void set_directional_shadow_pancake_size(real_t p_size);
};
