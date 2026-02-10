#pragma once

#include "math/vector4d.h"
#include "math/rotor4d.h"

/// BodyState4D — Physical state of a rigid body in 4D.
///
/// Contains kinematic state (position, velocity, orientation) and
/// dynamic state (mass, force accumulators, damping coefficients).
struct BodyState4D {
	// Mass properties
	real_t mass = 1.0;
	real_t inv_mass = 1.0; // Computed: 1/mass (0 = infinite mass / static)

	// Linear kinematics
	Vector4 position = Vector4(0, 0, 0, 0);
	Vector4 linear_velocity = Vector4(0, 0, 0, 0);

	// Rotational kinematics (simplified: no rotation for now)
	// Future: Rotor4D orientation, bivector angular_velocity

	// Force accumulators (cleared each step)
	Vector4 force = Vector4(0, 0, 0, 0);
	// Future: bivector torque

	// Damping coefficients
	real_t linear_damp = 0.0;
	// Future: angular_damp

	// Body mode (for future use)
	enum Mode {
		MODE_STATIC,    // Infinite mass, no dynamics
		MODE_KINEMATIC, // User-controlled motion
		MODE_RIGID      // Physics-driven
	};
	Mode mode = MODE_RIGID;

	/// Update inverse mass when mass changes
	void update_mass(real_t p_mass) {
		mass = p_mass;
		inv_mass = (mass > CMP_EPSILON) ? (1.0 / mass) : 0.0;
	}
};
