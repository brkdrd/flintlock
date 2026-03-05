#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

using namespace godot;

class SliderJoint4D : public Joint4D {
	GDCLASS(SliderJoint4D, Joint4D);

public:
	SliderJoint4D();
	~SliderJoint4D();

	void set_linear_limit_upper(float p_upper);
	float get_linear_limit_upper() const;

	void set_linear_limit_lower(float p_lower);
	float get_linear_limit_lower() const;

	void set_linear_softness(float p_softness);
	float get_linear_softness() const;

	void set_linear_restitution(float p_restitution);
	float get_linear_restitution() const;

	void set_linear_damping(float p_damping);
	float get_linear_damping() const;

	void set_motor_linear_enabled(bool p_enabled);
	bool is_motor_linear_enabled() const;

protected:
	static void _bind_methods();
	virtual void _configure_joint() override;

private:
	float _linear_limit_upper = 1.0f;
	float _linear_limit_lower = -1.0f;
	float _linear_softness = 1.0f;
	float _linear_restitution = 0.7f;
	float _linear_damping = 1.0f;
	bool _motor_linear_enabled = false;
};
