#pragma once
#include "light_4d.h"
#include <godot_cpp/classes/omni_light3d.hpp>

using namespace godot;

class OmniLight4D : public Light4D {
	GDCLASS(OmniLight4D, Light4D);

	real_t _omni_range = 5.0f;
	real_t _omni_attenuation = 1.0f;

protected:
	Light3D *_create_light_node() override;
	void _project_light() override;
	static void _bind_methods();

public:
	real_t get_omni_range() const { return _omni_range; }
	void set_omni_range(real_t p_range);

	real_t get_omni_attenuation() const { return _omni_attenuation; }
	void set_omni_attenuation(real_t p_attenuation);
};
