#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

using namespace godot;

class HingeJoint4D : public Joint4D {
	GDCLASS(HingeJoint4D, Joint4D);

public:
	HingeJoint4D();
	~HingeJoint4D();

	void set_angular_limit_enabled(bool p_enabled);
	bool is_angular_limit_enabled() const;

	void set_angular_limit_upper(float p_upper);
	float get_angular_limit_upper() const;

	void set_angular_limit_lower(float p_lower);
	float get_angular_limit_lower() const;

	void set_motor_enabled(bool p_enabled);
	bool is_motor_enabled() const;

	void set_motor_target_velocity(float p_velocity);
	float get_motor_target_velocity() const;

	void set_motor_max_impulse(float p_impulse);
	float get_motor_max_impulse() const;

protected:
	static void _bind_methods();
	virtual void _configure_joint() override;

private:
	bool _angular_limit_enabled = false;
	float _angular_limit_upper = 1.5708f;  // ~PI/2
	float _angular_limit_lower = -1.5708f; // ~-PI/2
	bool _motor_enabled = false;
	float _motor_target_velocity = 0.0f;
	float _motor_max_impulse = 1.0f;
};
