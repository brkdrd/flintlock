#pragma once

#include "math/vector4d.h"

/// Gravity4D — 4D gravitational force computation.
///
/// In 4D space, gravity follows the inverse-cube law:
///   F = G * m₁ * m₂ / r³
///
/// This is the physically correct generalization from 3D's inverse-square
/// law, derived from Gauss's law over a 3-sphere surface.
class Gravity4D {
public:
	/// Compute gravitational force on body A due to body B
	/// Returns force vector pointing from A towards B
	/// Formula: F = G * m_a * m_b * (direction / r³)
	static Vector4 compute_force(
		const Vector4 &p_pos_a,
		real_t p_mass_a,
		const Vector4 &p_pos_b,
		real_t p_mass_b,
		real_t p_G = 1.0
	);

	/// Compute softened gravitational force to avoid singularities
	/// Formula: F = G * m_a * m_b * direction / (r² + ε²)^(3/2)
	/// Prevents infinite force when bodies are very close
	static Vector4 compute_force_softened(
		const Vector4 &p_pos_a,
		real_t p_mass_a,
		const Vector4 &p_pos_b,
		real_t p_mass_b,
		real_t p_G,
		real_t p_softening
	);
};
