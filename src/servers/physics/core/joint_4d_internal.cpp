#include "joint_4d_internal.h"
#include <cmath>
#include <algorithm>

// ============================================================================
// PinJoint4D Implementation
// ============================================================================

void PinJoint4DInternal::prepare(std::vector<RigidBody4DInternal> &bodies, float dt) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	// Compute world-space anchors
	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);

	// Position error
	Vector4 error = wb - wa;
	bias = error * (bias_factor / dt);

	// Compute effective mass matrix (simplified as scalar per axis)
	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			effective_mass[i][j] = (i == j) ? (inv_mass_sum > PHYSICS_EPSILON ? 1.0f / inv_mass_sum : 0.0f) : 0.0f;
		}
	}

	// Warm start
	a.linear_velocity -= accumulated_impulse * a.get_inv_mass();
	b.linear_velocity += accumulated_impulse * b.get_inv_mass();

	Vector4 r_a = wa - a.transform.origin;
	Vector4 r_b = wb - b.transform.origin;
	a.angular_velocity -= a.get_world_inv_inertia().xform(Bivec4::wedge(r_a, accumulated_impulse));
	b.angular_velocity += b.get_world_inv_inertia().xform(Bivec4::wedge(r_b, accumulated_impulse));
}

void PinJoint4DInternal::solve(std::vector<RigidBody4DInternal> &bodies) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);
	Vector4 r_a = wa - a.transform.origin;
	Vector4 r_b = wb - b.transform.origin;

	// Relative velocity at the anchor point
	Vector4 vel_a = a.linear_velocity + a.angular_velocity.cross(r_a);
	Vector4 vel_b = b.linear_velocity + b.angular_velocity.cross(r_b);
	Vector4 rel_vel = vel_a - vel_b;

	// Compute impulse
	Vector4 impulse;
	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();
	if (inv_mass_sum > PHYSICS_EPSILON) {
		impulse = (bias - rel_vel * damping) * (1.0f / inv_mass_sum);
	}

	// Clamp if needed
	if (impulse_clamp > 0.0f) {
		float len = impulse.length();
		if (len > impulse_clamp) {
			impulse *= impulse_clamp / len;
		}
	}

	accumulated_impulse += impulse;

	// Apply impulse
	a.linear_velocity -= impulse * a.get_inv_mass();
	b.linear_velocity += impulse * b.get_inv_mass();
	a.angular_velocity -= a.get_world_inv_inertia().xform(Bivec4::wedge(r_a, impulse));
	b.angular_velocity += b.get_world_inv_inertia().xform(Bivec4::wedge(r_b, impulse));
}

// ============================================================================
// HingeJoint4D Implementation
// ============================================================================

void HingeJoint4DInternal::prepare(std::vector<RigidBody4DInternal> &bodies, float dt) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);

	// Position constraint bias
	Vector4 error = wb - wa;
	position_bias = error * (0.2f / dt);

	// Warm start position
	a.linear_velocity -= accumulated_position_impulse * a.get_inv_mass();
	b.linear_velocity += accumulated_position_impulse * b.get_inv_mass();
}

void HingeJoint4DInternal::solve(std::vector<RigidBody4DInternal> &bodies) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);
	Vector4 r_a = wa - a.transform.origin;
	Vector4 r_b = wb - b.transform.origin;

	// Solve position constraint (same as pin joint)
	Vector4 vel_a = a.linear_velocity + a.angular_velocity.cross(r_a);
	Vector4 vel_b = b.linear_velocity + b.angular_velocity.cross(r_b);
	Vector4 rel_vel = vel_a - vel_b;

	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();
	if (inv_mass_sum > PHYSICS_EPSILON) {
		Vector4 impulse = (position_bias - rel_vel) * (1.0f / inv_mass_sum);
		accumulated_position_impulse += impulse;
		a.linear_velocity -= impulse * a.get_inv_mass();
		b.linear_velocity += impulse * b.get_inv_mass();
		a.angular_velocity -= a.get_world_inv_inertia().xform(Bivec4::wedge(r_a, impulse));
		b.angular_velocity += b.get_world_inv_inertia().xform(Bivec4::wedge(r_b, impulse));
	}

	// Angular constraint: lock all rotation planes except the free one
	Bivec4 rel_omega = a.angular_velocity - b.angular_velocity;
	for (int plane = 0; plane < 6; plane++) {
		if (plane == rotation_plane) {
			// Apply motor if enabled
			if (motor_enabled) {
				float omega_plane = rel_omega.v[plane];
				float motor_impulse = (motor_target_velocity - omega_plane);
				motor_impulse = fmaxf(-motor_max_impulse, fminf(motor_impulse, motor_max_impulse));

				Bivec4 angular_impulse;
				angular_impulse.v[plane] = motor_impulse;
				a.angular_velocity += a.get_world_inv_inertia().xform(angular_impulse);
				b.angular_velocity -= b.get_world_inv_inertia().xform(angular_impulse);
			}
			continue;
		}

		// Lock this rotation plane
		float omega_plane = rel_omega.v[plane];
		if (fabsf(omega_plane) > PHYSICS_EPSILON) {
			// Compute effective mass for this rotation plane
			float inv_I_a = a.get_world_inv_inertia().m[plane][plane];
			float inv_I_b = b.get_world_inv_inertia().m[plane][plane];
			float eff_mass = inv_I_a + inv_I_b;
			if (eff_mass > PHYSICS_EPSILON) {
				float impulse = -omega_plane / eff_mass;
				Bivec4 angular_impulse;
				angular_impulse.v[plane] = impulse;
				a.angular_velocity += a.get_world_inv_inertia().xform(angular_impulse);
				b.angular_velocity -= b.get_world_inv_inertia().xform(angular_impulse);
			}
		}
	}
}

// ============================================================================
// SliderJoint4D Implementation
// ============================================================================

void SliderJoint4DInternal::prepare(std::vector<RigidBody4DInternal> &bodies, float dt) {
	accumulated_slide_impulse = 0.0f;
}

void SliderJoint4DInternal::solve(std::vector<RigidBody4DInternal> &bodies) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);

	// Get slide axis in world space (from body A's basis)
	Vector4 axis = a.transform.basis.get_column(slide_axis);

	// Constrain relative velocity to be along the axis only
	Vector4 rel_vel = a.linear_velocity - b.linear_velocity;
	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();
	if (inv_mass_sum < PHYSICS_EPSILON) return;

	// For each non-slide direction, apply constraint
	for (int i = 0; i < 4; i++) {
		if (i == slide_axis) continue;
		Vector4 constrain_dir = a.transform.basis.get_column(i);
		float vel_component = rel_vel.dot(constrain_dir);

		// Position error in this direction
		Vector4 diff = wb - wa;
		float pos_error = diff.dot(constrain_dir);
		float bias = pos_error * 0.2f / 0.016667f; // Assuming ~60fps

		float impulse = -(vel_component - bias) / inv_mass_sum;

		Vector4 P = constrain_dir * impulse;
		a.linear_velocity -= P * a.get_inv_mass();
		b.linear_velocity += P * b.get_inv_mass();
	}

	// Lock all rotation
	Bivec4 rel_omega = a.angular_velocity - b.angular_velocity;
	for (int plane = 0; plane < 6; plane++) {
		float omega_plane = rel_omega.v[plane];
		if (fabsf(omega_plane) < PHYSICS_EPSILON) continue;
		float inv_I_a = a.get_world_inv_inertia().m[plane][plane];
		float inv_I_b = b.get_world_inv_inertia().m[plane][plane];
		float eff_mass = inv_I_a + inv_I_b;
		if (eff_mass > PHYSICS_EPSILON) {
			float impulse = -omega_plane / eff_mass;
			Bivec4 angular_impulse;
			angular_impulse.v[plane] = impulse;
			a.angular_velocity += a.get_world_inv_inertia().xform(angular_impulse);
			b.angular_velocity -= b.get_world_inv_inertia().xform(angular_impulse);
		}
	}
}

// ============================================================================
// ConeTwistJoint4D Implementation
// ============================================================================

void ConeTwistJoint4DInternal::prepare(std::vector<RigidBody4DInternal> &bodies, float dt) {
	accumulated_position_impulse = Vector4(0, 0, 0, 0);
}

void ConeTwistJoint4DInternal::solve(std::vector<RigidBody4DInternal> &bodies) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	// Position constraint (same as pin)
	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);
	Vector4 r_a = wa - a.transform.origin;
	Vector4 r_b = wb - b.transform.origin;

	Vector4 vel_a = a.linear_velocity + a.angular_velocity.cross(r_a);
	Vector4 vel_b = b.linear_velocity + b.angular_velocity.cross(r_b);
	Vector4 rel_vel = vel_a - vel_b;

	Vector4 error = wb - wa;
	Vector4 bias = error * (0.2f / 0.016667f);

	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();
	if (inv_mass_sum > PHYSICS_EPSILON) {
		Vector4 impulse = (bias - rel_vel * damping_val) * (1.0f / inv_mass_sum);
		accumulated_position_impulse += impulse;
		a.linear_velocity -= impulse * a.get_inv_mass();
		b.linear_velocity += impulse * b.get_inv_mass();
		a.angular_velocity -= a.get_world_inv_inertia().xform(Bivec4::wedge(r_a, impulse));
		b.angular_velocity += b.get_world_inv_inertia().xform(Bivec4::wedge(r_b, impulse));
	}

	// Angular constraint: limit angular velocity magnitude in each plane
	Bivec4 rel_omega = a.angular_velocity - b.angular_velocity;
	float total_angle_sq = rel_omega.length_squared();

	if (total_angle_sq > swing_limit * swing_limit) {
		float scale = swing_limit / sqrtf(total_angle_sq);
		Bivec4 correction = rel_omega * (1.0f - scale) * softness;
		for (int plane = 0; plane < 6; plane++) {
			float inv_I_a = a.get_world_inv_inertia().m[plane][plane];
			float inv_I_b = b.get_world_inv_inertia().m[plane][plane];
			float eff_mass = inv_I_a + inv_I_b;
			if (eff_mass > PHYSICS_EPSILON) {
				float impulse = -correction.v[plane] / eff_mass;
				Bivec4 angular_impulse;
				angular_impulse.v[plane] = impulse;
				a.angular_velocity += a.get_world_inv_inertia().xform(angular_impulse);
				b.angular_velocity -= b.get_world_inv_inertia().xform(angular_impulse);
			}
		}
	}
}

// ============================================================================
// Generic10DOFJoint4D Implementation
// ============================================================================

void Generic10DOFJoint4DInternal::prepare(std::vector<RigidBody4DInternal> &bodies, float dt) {
	// Reset accumulators
	for (int i = 0; i < 4; i++) accumulated_linear[i] = 0;
	for (int i = 0; i < 6; i++) accumulated_angular[i] = 0;
}

void Generic10DOFJoint4DInternal::solve(std::vector<RigidBody4DInternal> &bodies) {
	RigidBody4DInternal &a = bodies[body_a_index];
	RigidBody4DInternal &b = bodies[body_b_index];

	Vector4 wa = a.transform.xform(local_anchor_a);
	Vector4 wb = b.transform.xform(local_anchor_b);
	Vector4 diff = wb - wa;

	float inv_mass_sum = a.get_inv_mass() + b.get_inv_mass();

	// Linear constraints
	for (int axis = 0; axis < 4; axis++) {
		if (!linear_limit_enabled[axis]) {
			// Free along this axis - just enforce anchor
			Vector4 dir = a.transform.basis.get_column(axis);
			float pos_err = diff.dot(dir);
			float vel_err = (a.linear_velocity - b.linear_velocity).dot(dir);

			if (inv_mass_sum > PHYSICS_EPSILON) {
				float impulse = -(vel_err - pos_err * 0.2f / 0.016667f) / inv_mass_sum;
				Vector4 P = dir * impulse;
				a.linear_velocity -= P * a.get_inv_mass();
				b.linear_velocity += P * b.get_inv_mass();
				accumulated_linear[axis] += impulse;
			}
		} else {
			// Limited along this axis
			Vector4 dir = a.transform.basis.get_column(axis);
			float pos = diff.dot(dir);
			if (pos < linear_lower[axis]) {
				float correction = linear_lower[axis] - pos;
				float vel_err = (a.linear_velocity - b.linear_velocity).dot(dir);
				if (inv_mass_sum > PHYSICS_EPSILON) {
					float impulse = -(vel_err - correction * 0.2f / 0.016667f) / inv_mass_sum;
					impulse = fmaxf(impulse, 0.0f);
					Vector4 P = dir * impulse;
					a.linear_velocity -= P * a.get_inv_mass();
					b.linear_velocity += P * b.get_inv_mass();
				}
			} else if (pos > linear_upper[axis]) {
				float correction = linear_upper[axis] - pos;
				float vel_err = (a.linear_velocity - b.linear_velocity).dot(dir);
				if (inv_mass_sum > PHYSICS_EPSILON) {
					float impulse = -(vel_err - correction * 0.2f / 0.016667f) / inv_mass_sum;
					impulse = fminf(impulse, 0.0f);
					Vector4 P = dir * impulse;
					a.linear_velocity -= P * a.get_inv_mass();
					b.linear_velocity += P * b.get_inv_mass();
				}
			}
		}
	}

	// Angular constraints
	Bivec4 rel_omega = a.angular_velocity - b.angular_velocity;
	for (int plane = 0; plane < 6; plane++) {
		if (!angular_limit_enabled[plane]) {
			// Lock this plane
			float omega = rel_omega.v[plane];
			if (fabsf(omega) > PHYSICS_EPSILON) {
				float inv_I_a = a.get_world_inv_inertia().m[plane][plane];
				float inv_I_b = b.get_world_inv_inertia().m[plane][plane];
				float eff = inv_I_a + inv_I_b;
				if (eff > PHYSICS_EPSILON) {
					float impulse = -omega / eff;
					Bivec4 ai;
					ai.v[plane] = impulse;
					a.angular_velocity += a.get_world_inv_inertia().xform(ai);
					b.angular_velocity -= b.get_world_inv_inertia().xform(ai);
					accumulated_angular[plane] += impulse;
				}
			}
		}
		// If limit enabled, the angular DOF is free within limits
		// Angular limit enforcement would require tracking the angle,
		// which needs integration. For now, limit angular velocity range.
	}
}

// ============================================================================
// Solve all joints
// ============================================================================
void solve_joints_4d(
	std::vector<std::shared_ptr<Joint4DInternal>> &joints,
	std::vector<RigidBody4DInternal> &bodies,
	float dt,
	int iterations
) {
	// Sort by priority
	// (skip sorting for now, just iterate)

	// Prepare all joints
	for (auto &joint : joints) {
		if (joint->body_a_index >= 0 && joint->body_b_index >= 0) {
			joint->prepare(bodies, dt);
		}
	}

	// Solve iterations
	for (int iter = 0; iter < iterations; iter++) {
		for (auto &joint : joints) {
			if (joint->body_a_index >= 0 && joint->body_b_index >= 0) {
				joint->solve(bodies);
			}
		}
	}
}
