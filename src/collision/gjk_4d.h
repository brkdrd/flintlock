#pragma once

#include "math/transform4d.h"
#include "shapes/shape4d.h"
#include <vector>

/// Result of a GJK collision test
struct GJKResult4D {
	bool colliding = false;
	// Future: simplex data for EPA
};

/// GJK4D — Gilbert-Johnson-Keerthi collision detection in 4D.
///
/// Uses support functions from Shape4D to test whether the Minkowski
/// difference contains the origin. Works with any convex shape that
/// provides a support function.
class GJK4D {
public:
	/// Test if two shapes are colliding
	/// Returns true if the shapes overlap in their world-space positions
	static GJKResult4D intersect(
		const Shape4D *p_shape_a,
		const Transform4D &p_transform_a,
		const Shape4D *p_shape_b,
		const Transform4D &p_transform_b
	);

private:
	/// Support function for Minkowski difference A - B
	static Vector4 support(
		const Shape4D *p_a,
		const Transform4D &p_ta,
		const Shape4D *p_b,
		const Transform4D &p_tb,
		const Vector4 &p_direction
	);

	/// Simplex evolution helpers (2, 3, 4, 5 points)
	static bool do_simplex_2(std::vector<Vector4> &simplex, Vector4 &direction);
	static bool do_simplex_3(std::vector<Vector4> &simplex, Vector4 &direction);
	static bool do_simplex_4(std::vector<Vector4> &simplex, Vector4 &direction);
	static bool do_simplex_5(std::vector<Vector4> &simplex, Vector4 &direction);
};
