#include "space_4d.h"
#include "body_4d.h"
#include "dynamics/gravity_4d.h"
#include "dynamics/integrator_4d.h"
#include <algorithm>

void Space4D::add_body(Body4D *p_body) {
	if (std::find(bodies.begin(), bodies.end(), p_body) == bodies.end()) {
		bodies.push_back(p_body);
	}
}

void Space4D::remove_body(Body4D *p_body) {
	auto it = std::find(bodies.begin(), bodies.end(), p_body);
	if (it != bodies.end()) {
		bodies.erase(it);
	}
}

void Space4D::step(real_t p_delta) {
	if (!active) {
		return;
	}

	// Apply gravity forces between all body pairs
	const real_t G = 1.0; // Gravitational constant
	const real_t softening = 0.1; // Small softening to avoid singularities

	for (size_t i = 0; i < bodies.size(); i++) {
		Body4D *body_a = bodies[i];
		BodyState4D &state_a = body_a->get_state();

		// Skip static bodies for force application
		if (state_a.mode == BodyState4D::MODE_STATIC) {
			continue;
		}

		// Gravity from all other bodies
		for (size_t j = 0; j < bodies.size(); j++) {
			if (i == j) continue;

			Body4D *body_b = bodies[j];
			BodyState4D &state_b = body_b->get_state();

			// Compute gravitational force on A due to B
			Vector4 force = Gravity4D::compute_force_softened(
				state_a.position,
				state_a.mass,
				state_b.position,
				state_b.mass,
				G,
				softening
			);

			// Add force to body A's accumulator
			body_a->add_force(force);
		}
	}

	// Integrate all bodies
	for (Body4D *body : bodies) {
		Integrator4D::step(body->get_state(), p_delta);
	}
}
