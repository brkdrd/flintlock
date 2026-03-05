#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

using namespace godot;

class ConeTwistJoint4D : public Joint4D {
	GDCLASS(ConeTwistJoint4D, Joint4D);

public:
	ConeTwistJoint4D();
	~ConeTwistJoint4D();

	void set_swing_span(float p_swing_span);
	float get_swing_span() const;

	void set_twist_span(float p_twist_span);
	float get_twist_span() const;

	void set_bias(float p_bias);
	float get_bias() const;

	void set_softness(float p_softness);
	float get_softness() const;

	void set_relaxation(float p_relaxation);
	float get_relaxation() const;

protected:
	static void _bind_methods();
	virtual void _configure_joint() override;

private:
	float _swing_span = 0.785398f;  // ~PI/4
	float _twist_span = 3.14159f;   // ~PI
	float _bias = 0.3f;
	float _softness = 0.8f;
	float _relaxation = 1.0f;
};
