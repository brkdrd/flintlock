#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

using namespace godot;

class PinJoint4D : public Joint4D {
	GDCLASS(PinJoint4D, Joint4D);

public:
	PinJoint4D();
	~PinJoint4D();

	void set_damping(float p_damping);
	float get_damping() const;

	void set_impulse_clamp(float p_clamp);
	float get_impulse_clamp() const;

	void set_bias(float p_bias);
	float get_bias() const;

protected:
	static void _bind_methods();
	virtual void _configure_joint() override;

private:
	float _damping = 1.0f;
	float _impulse_clamp = 0.0f;
	float _bias = 0.3f;
};
