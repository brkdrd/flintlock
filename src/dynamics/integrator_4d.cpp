#include "integrator_4d.h"

void Integrator4D::step(BodyState4D &r_body, real_t p_dt) {
	// Update inverse mass in case mass was changed directly
	if (r_body.mass > CMP_EPSILON) {
		r_body.inv_mass = 1.0 / r_body.mass;
	} else {
		r_body.inv_mass = 0.0; // Static body
	}

	// Skip dynamics for static bodies
	if (r_body.inv_mass < CMP_EPSILON) {
		// Clear forces and return
		r_body.force = Vector4(0, 0, 0, 0);
		return;
	}

	// Semi-implicit Euler integration
	// Step 1: Update velocity from forces
	Vector4 acceleration = r_body.force * r_body.inv_mass;
	r_body.linear_velocity = r_body.linear_velocity + acceleration * p_dt;

	// Step 2: Apply damping
	// velocity *= (1 - damp * dt)
	// Clamp to avoid negative damping or over-damping
	real_t damping_factor = 1.0 - r_body.linear_damp * p_dt;
	if (damping_factor < 0.0) {
		damping_factor = 0.0;
	}
	r_body.linear_velocity = r_body.linear_velocity * damping_factor;

	// Step 3: Update position from new velocity
	r_body.position = r_body.position + r_body.linear_velocity * p_dt;

	// Step 4: Clear force accumulators
	r_body.force = Vector4(0, 0, 0, 0);
}
