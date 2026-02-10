#pragma once

#include "body_state_4d.h"

/// Integrator4D — Semi-implicit (symplectic) Euler integration for 4D dynamics.
///
/// Uses the velocity Verlet variant:
///   1. v += (F / m) * dt   (update velocity first)
///   2. x += v * dt         (then update position with new velocity)
///
/// This ordering provides better energy conservation than explicit Euler.
class Integrator4D {
public:
	/// Integrate one timestep for the given body state
	/// Modifies body in place:
	///   - Updates velocity from forces
	///   - Updates position from velocity
	///   - Applies damping
	///   - Clears force accumulators
	static void step(BodyState4D &r_body, real_t p_dt);
};
