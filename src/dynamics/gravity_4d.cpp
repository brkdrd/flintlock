#include "gravity_4d.h"
#include <cmath>

Vector4 Gravity4D::compute_force(
	const Vector4 &p_pos_a,
	real_t p_mass_a,
	const Vector4 &p_pos_b,
	real_t p_mass_b,
	real_t p_G
) {
	// Direction from A to B
	Vector4 direction = p_pos_b - p_pos_a;
	real_t r_squared = direction.length_squared();

	// Avoid singularity
	if (r_squared < CMP_EPSILON) {
		return Vector4(0, 0, 0, 0);
	}

	real_t r = std::sqrt(r_squared);
	real_t r_cubed = r_squared * r;

	// Normalize direction
	Vector4 unit_direction = direction / r;

	// F = G * m1 * m2 / r³ (in direction from A to B)
	real_t force_magnitude = p_G * p_mass_a * p_mass_b / r_cubed;

	return unit_direction * force_magnitude;
}

Vector4 Gravity4D::compute_force_softened(
	const Vector4 &p_pos_a,
	real_t p_mass_a,
	const Vector4 &p_pos_b,
	real_t p_mass_b,
	real_t p_G,
	real_t p_softening
) {
	// Direction from A to B
	Vector4 direction = p_pos_b - p_pos_a;
	real_t r_squared = direction.length_squared();
	real_t eps_squared = p_softening * p_softening;

	// Softened denominator: (r² + ε²)^(3/2)
	real_t denom = std::pow(r_squared + eps_squared, 1.5);

	// Avoid division by zero
	if (denom < CMP_EPSILON) {
		return Vector4(0, 0, 0, 0);
	}

	// F = G * m1 * m2 / (r² + ε²)^(3/2)
	real_t force_magnitude = p_G * p_mass_a * p_mass_b / denom;

	// Normalize direction (with softening, direction is still from A to B)
	real_t r = std::sqrt(r_squared);
	if (r < CMP_EPSILON) {
		// If at same position, no preferred direction
		return Vector4(0, 0, 0, 0);
	}

	Vector4 unit_direction = direction / r;

	return unit_direction * force_magnitude;
}
