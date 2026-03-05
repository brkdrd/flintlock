#pragma once
#include "light_4d.h"
#include <godot_cpp/classes/spot_light3d.hpp>

using namespace godot;

class SpotLight4D : public Light4D {
	GDCLASS(SpotLight4D, Light4D);

	real_t _spot_range = 5.0f;
	real_t _spot_attenuation = 1.0f;
	real_t _spot_angle = 45.0f;
	real_t _spot_angle_attenuation = 1.0f;

protected:
	Light3D *_create_light_node() override;
	void _project_light() override;
	static void _bind_methods();

public:
	real_t get_spot_range() const { return _spot_range; }
	void set_spot_range(real_t p_range);

	real_t get_spot_attenuation() const { return _spot_attenuation; }
	void set_spot_attenuation(real_t p_attenuation);

	real_t get_spot_angle() const { return _spot_angle; }
	void set_spot_angle(real_t p_angle);

	real_t get_spot_angle_attenuation() const { return _spot_angle_attenuation; }
	void set_spot_angle_attenuation(real_t p_attenuation);
};
