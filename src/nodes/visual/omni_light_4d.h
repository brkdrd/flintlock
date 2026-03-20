#pragma once
#include "light_4d.h"

using namespace godot;

class OmniLight4D : public Light4D {
	GDCLASS(OmniLight4D, Light4D);

	real_t _omni_range = 5.0f;
	real_t _omni_attenuation = 1.0f;

protected:
	int _get_light_type() const override { return 1; } // omni
	static void _bind_methods();
	void _notification(int p_what);

public:
	real_t get_omni_range() const { return _omni_range; }
	void set_omni_range(real_t p_range);

	real_t get_omni_attenuation() const { return _omni_attenuation; }
	void set_omni_attenuation(real_t p_attenuation);
};
