#pragma once

#include "rigid_body_4d_internal.h"
#include "constraint_solver_4d.h"
#include <vector>

// ============================================================================
// Joint constraint types for the 4D physics engine.
// All joints are solved as velocity-level constraints using sequential impulse.
// ============================================================================

enum JointType4D {
	JOINT_4D_PIN = 0,
	JOINT_4D_HINGE = 1,
	JOINT_4D_SLIDER = 2,
	JOINT_4D_CONE_TWIST = 3,
	JOINT_4D_GENERIC_10DOF = 4,
};

struct Joint4DInternal {
	JointType4D type;
	int body_a_index = -1;
	int body_b_index = -1;
	int solver_priority = 1;
	bool exclude_collision = true;

	// Anchor points in local space of each body
	Vector4 local_anchor_a;
	Vector4 local_anchor_b;

	virtual ~Joint4DInternal() = default;
	virtual void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) = 0;
	virtual void solve(std::vector<RigidBody4DInternal> &bodies) = 0;
};

// ============================================================================
// PinJoint: Constrains two bodies to share a common point in 4D space.
// 4 DOF constraint (position only, no rotation constraint).
// ============================================================================
struct PinJoint4DInternal : public Joint4DInternal {
	float damping = 1.0f;
	float impulse_clamp = 0.0f;
	float bias_factor = 0.3f;

	// Solver data
	Vector4 bias;
	float effective_mass[4][4]; // 4x4 effective mass matrix
	Vector4 accumulated_impulse;

	PinJoint4DInternal() { type = JOINT_4D_PIN; }

	void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) override;
	void solve(std::vector<RigidBody4DInternal> &bodies) override;
};

// ============================================================================
// HingeJoint: Constrains rotation to a single plane in 4D.
// In 4D, a "hinge" allows rotation in one specific plane while constraining
// the other 5 rotation planes. Plus the 4 positional DOFs = 9 constraints.
// ============================================================================
struct HingeJoint4DInternal : public Joint4DInternal {
	int rotation_plane = 0; // Which rotation plane is free (0-5: XY,XZ,XW,YZ,YW,ZW)
	float lower_limit = -3.14159f;
	float upper_limit = 3.14159f;
	bool limit_enabled = false;
	float motor_target_velocity = 0.0f;
	float motor_max_impulse = 0.0f;
	bool motor_enabled = false;

	// Solver data
	Vector4 position_bias;
	Vector4 accumulated_position_impulse;

	HingeJoint4DInternal() { type = JOINT_4D_HINGE; }

	void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) override;
	void solve(std::vector<RigidBody4DInternal> &bodies) override;
};

// ============================================================================
// SliderJoint: Allows translation along one axis in 4D.
// Constrains 3 translational DOFs and all 6 rotational DOFs.
// ============================================================================
struct SliderJoint4DInternal : public Joint4DInternal {
	int slide_axis = 1; // 0=X, 1=Y, 2=Z, 3=W
	float lower_limit = -1.0f;
	float upper_limit = 1.0f;
	bool limit_enabled = false;

	// Solver data
	float accumulated_slide_impulse = 0.0f;

	SliderJoint4DInternal() { type = JOINT_4D_SLIDER; }

	void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) override;
	void solve(std::vector<RigidBody4DInternal> &bodies) override;
};

// ============================================================================
// ConeTwistJoint: Allows rotation within a cone in 4D.
// Constrains position (4 DOFs) and limits rotation angles.
// ============================================================================
struct ConeTwistJoint4DInternal : public Joint4DInternal {
	float swing_limit = 0.5f;
	float twist_limit = 0.5f;
	float softness = 0.8f;
	float damping_val = 1.0f;

	// Solver data
	Vector4 accumulated_position_impulse;

	ConeTwistJoint4DInternal() { type = JOINT_4D_CONE_TWIST; }

	void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) override;
	void solve(std::vector<RigidBody4DInternal> &bodies) override;
};

// ============================================================================
// Generic10DOFJoint: Configurable joint with limits on all 10 DOFs
// (4 translational + 6 rotational in 4D).
// ============================================================================
struct Generic10DOFJoint4DInternal : public Joint4DInternal {
	// Linear limits (per axis)
	float linear_lower[4] = {0, 0, 0, 0};
	float linear_upper[4] = {0, 0, 0, 0};
	bool linear_limit_enabled[4] = {false, false, false, false};

	// Angular limits (per rotation plane)
	float angular_lower[6] = {0, 0, 0, 0, 0, 0};
	float angular_upper[6] = {0, 0, 0, 0, 0, 0};
	bool angular_limit_enabled[6] = {false, false, false, false, false, false};

	// Solver data
	float accumulated_linear[4] = {0, 0, 0, 0};
	float accumulated_angular[6] = {0, 0, 0, 0, 0, 0};

	Generic10DOFJoint4DInternal() { type = JOINT_4D_GENERIC_10DOF; }

	void prepare(std::vector<RigidBody4DInternal> &bodies, float dt) override;
	void solve(std::vector<RigidBody4DInternal> &bodies) override;
};

// Solve all joints
void solve_joints_4d(
	std::vector<std::shared_ptr<Joint4DInternal>> &joints,
	std::vector<RigidBody4DInternal> &bodies,
	float dt,
	int iterations = SOLVER_ITERATIONS
);
