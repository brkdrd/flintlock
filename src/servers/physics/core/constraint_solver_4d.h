#pragma once

#include "contact_4d.h"
#include "rigid_body_4d_internal.h"
#include <vector>

// ============================================================================
// Sequential Impulse Constraint Solver for 4D physics.
// Resolves contact constraints and joint constraints.
// ============================================================================

static constexpr int SOLVER_ITERATIONS = 10;
static constexpr float BAUMGARTE_FACTOR = 0.2f;
static constexpr float SLOP = 0.01f; // Penetration slop
static constexpr float RESTITUTION_THRESHOLD = 1.0f;

// Solve all contact constraints for one step
void solve_contacts_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies,
	float dt
);

// Pre-step: compute effective masses and biases
void prepare_contacts_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies,
	float dt
);

// Single iteration of the solver
void solve_iteration_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies
);
