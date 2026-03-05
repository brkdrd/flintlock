#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

using namespace godot;

// Generic 10-DOF joint for 4D physics.
// Linear DOFs: X, Y, Z, W (4 axes)
// Angular DOFs: XY, XZ, XW, YZ, YW, ZW (6 bivector planes)
// Each DOF has: limit_enabled (bool), limit_upper (float), limit_lower (float), motor_enabled (bool)

class Generic10DOFJoint4D : public Joint4D {
	GDCLASS(Generic10DOFJoint4D, Joint4D);

public:
	Generic10DOFJoint4D();
	~Generic10DOFJoint4D();

	// ─── Linear X ─────────────────────────────────────────────────────────────
	void set_linear_x_limit_enabled(bool p_enabled);
	bool is_linear_x_limit_enabled() const;
	void set_linear_x_limit_upper(float p_upper);
	float get_linear_x_limit_upper() const;
	void set_linear_x_limit_lower(float p_lower);
	float get_linear_x_limit_lower() const;
	void set_linear_x_motor_enabled(bool p_enabled);
	bool is_linear_x_motor_enabled() const;

	// ─── Linear Y ─────────────────────────────────────────────────────────────
	void set_linear_y_limit_enabled(bool p_enabled);
	bool is_linear_y_limit_enabled() const;
	void set_linear_y_limit_upper(float p_upper);
	float get_linear_y_limit_upper() const;
	void set_linear_y_limit_lower(float p_lower);
	float get_linear_y_limit_lower() const;
	void set_linear_y_motor_enabled(bool p_enabled);
	bool is_linear_y_motor_enabled() const;

	// ─── Linear Z ─────────────────────────────────────────────────────────────
	void set_linear_z_limit_enabled(bool p_enabled);
	bool is_linear_z_limit_enabled() const;
	void set_linear_z_limit_upper(float p_upper);
	float get_linear_z_limit_upper() const;
	void set_linear_z_limit_lower(float p_lower);
	float get_linear_z_limit_lower() const;
	void set_linear_z_motor_enabled(bool p_enabled);
	bool is_linear_z_motor_enabled() const;

	// ─── Linear W ─────────────────────────────────────────────────────────────
	void set_linear_w_limit_enabled(bool p_enabled);
	bool is_linear_w_limit_enabled() const;
	void set_linear_w_limit_upper(float p_upper);
	float get_linear_w_limit_upper() const;
	void set_linear_w_limit_lower(float p_lower);
	float get_linear_w_limit_lower() const;
	void set_linear_w_motor_enabled(bool p_enabled);
	bool is_linear_w_motor_enabled() const;

	// ─── Angular XY ───────────────────────────────────────────────────────────
	void set_angular_xy_limit_enabled(bool p_enabled);
	bool is_angular_xy_limit_enabled() const;
	void set_angular_xy_limit_upper(float p_upper);
	float get_angular_xy_limit_upper() const;
	void set_angular_xy_limit_lower(float p_lower);
	float get_angular_xy_limit_lower() const;
	void set_angular_xy_motor_enabled(bool p_enabled);
	bool is_angular_xy_motor_enabled() const;

	// ─── Angular XZ ───────────────────────────────────────────────────────────
	void set_angular_xz_limit_enabled(bool p_enabled);
	bool is_angular_xz_limit_enabled() const;
	void set_angular_xz_limit_upper(float p_upper);
	float get_angular_xz_limit_upper() const;
	void set_angular_xz_limit_lower(float p_lower);
	float get_angular_xz_limit_lower() const;
	void set_angular_xz_motor_enabled(bool p_enabled);
	bool is_angular_xz_motor_enabled() const;

	// ─── Angular XW ───────────────────────────────────────────────────────────
	void set_angular_xw_limit_enabled(bool p_enabled);
	bool is_angular_xw_limit_enabled() const;
	void set_angular_xw_limit_upper(float p_upper);
	float get_angular_xw_limit_upper() const;
	void set_angular_xw_limit_lower(float p_lower);
	float get_angular_xw_limit_lower() const;
	void set_angular_xw_motor_enabled(bool p_enabled);
	bool is_angular_xw_motor_enabled() const;

	// ─── Angular YZ ───────────────────────────────────────────────────────────
	void set_angular_yz_limit_enabled(bool p_enabled);
	bool is_angular_yz_limit_enabled() const;
	void set_angular_yz_limit_upper(float p_upper);
	float get_angular_yz_limit_upper() const;
	void set_angular_yz_limit_lower(float p_lower);
	float get_angular_yz_limit_lower() const;
	void set_angular_yz_motor_enabled(bool p_enabled);
	bool is_angular_yz_motor_enabled() const;

	// ─── Angular YW ───────────────────────────────────────────────────────────
	void set_angular_yw_limit_enabled(bool p_enabled);
	bool is_angular_yw_limit_enabled() const;
	void set_angular_yw_limit_upper(float p_upper);
	float get_angular_yw_limit_upper() const;
	void set_angular_yw_limit_lower(float p_lower);
	float get_angular_yw_limit_lower() const;
	void set_angular_yw_motor_enabled(bool p_enabled);
	bool is_angular_yw_motor_enabled() const;

	// ─── Angular ZW ───────────────────────────────────────────────────────────
	void set_angular_zw_limit_enabled(bool p_enabled);
	bool is_angular_zw_limit_enabled() const;
	void set_angular_zw_limit_upper(float p_upper);
	float get_angular_zw_limit_upper() const;
	void set_angular_zw_limit_lower(float p_lower);
	float get_angular_zw_limit_lower() const;
	void set_angular_zw_motor_enabled(bool p_enabled);
	bool is_angular_zw_motor_enabled() const;

protected:
	static void _bind_methods();
	virtual void _configure_joint() override;

private:
	// Linear DOF data
	struct LinearDOF {
		bool limit_enabled = false;
		float limit_upper = 0.0f;
		float limit_lower = 0.0f;
		bool motor_enabled = false;
	};

	// Angular DOF data
	struct AngularDOF {
		bool limit_enabled = false;
		float limit_upper = 0.0f;
		float limit_lower = 0.0f;
		bool motor_enabled = false;
	};

	LinearDOF _linear_x;
	LinearDOF _linear_y;
	LinearDOF _linear_z;
	LinearDOF _linear_w;

	AngularDOF _angular_xy;
	AngularDOF _angular_xz;
	AngularDOF _angular_xw;
	AngularDOF _angular_yz;
	AngularDOF _angular_yw;
	AngularDOF _angular_zw;
};
