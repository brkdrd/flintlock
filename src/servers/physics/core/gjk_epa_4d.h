#pragma once

#include "shape_4d_internal.h"

// ============================================================================
// GJK (Gilbert-Johnson-Keerthi) and EPA (Expanding Polytope Algorithm) in 4D
//
// GJK: Determines if two convex shapes overlap by searching the Minkowski
//       difference for the origin. Uses simplices up to pentachoron (5 vertices).
//
// EPA: Given that GJK found overlap, determines the minimum penetration depth
//       and contact normal. Uses tetrahedral facets on the 4D polytope boundary.
// ============================================================================

// Maximum iterations to prevent infinite loops
static constexpr int GJK_MAX_ITERATIONS = 64;
static constexpr int EPA_MAX_ITERATIONS = 128;
static constexpr int EPA_MAX_FACES = 512;
static constexpr float GJK_TOLERANCE = 1e-6f;
static constexpr float EPA_TOLERANCE = 1e-5f;

// ============================================================================
// GJK Result
// ============================================================================
struct GJKResult {
	bool colliding = false;
	Vector4 closest_on_a;  // Closest point on shape A (when not colliding)
	Vector4 closest_on_b;  // Closest point on shape B (when not colliding)
	float distance = 0.0f; // Separation distance (negative when overlapping)

	// Simplex data passed to EPA when colliding
	int simplex_count = 0;
	Vector4 simplex[5];        // Minkowski difference points
	Vector4 simplex_a[5];      // Support points on A
	Vector4 simplex_b[5];      // Support points on B
};

// ============================================================================
// EPA Result
// ============================================================================
struct EPAResult {
	bool valid = false;
	Vector4 normal;      // Penetration normal (from B to A)
	float depth = 0.0f;  // Penetration depth
	Vector4 point_on_a;  // Contact point on shape A
	Vector4 point_on_b;  // Contact point on shape B
};

// ============================================================================
// Combined collision result for the narrowphase
// ============================================================================
struct CollisionResult {
	bool colliding = false;
	Vector4 normal;       // Contact normal (from B to A in world space)
	float depth = 0.0f;   // Penetration depth
	Vector4 point_on_a;   // Contact point on A (world space)
	Vector4 point_on_b;   // Contact point on B (world space)
};

// ============================================================================
// Ray cast result
// ============================================================================
struct RayCastResult4D {
	bool hit = false;
	float t = 0.0f;       // Parameter along ray (0 to 1)
	Vector4 point;         // Hit point in world space
	Vector4 normal;        // Hit normal in world space
};

// Run GJK on two shapes
GJKResult gjk_4d(const MinkowskiPair &pair);

// Run EPA on two overlapping shapes (requires GJK simplex as input)
EPAResult epa_4d(const MinkowskiPair &pair, const GJKResult &gjk);

// Combined: run GJK, then EPA if overlapping
CollisionResult collide_shapes_4d(
	const Shape4DInternal *shape_a, const Xform4 &xform_a,
	const Shape4DInternal *shape_b, const Xform4 &xform_b
);

// Ray cast against a single shape
RayCastResult4D ray_cast_shape_4d(
	const Vector4 &ray_origin, const Vector4 &ray_dir, float ray_length,
	const Shape4DInternal *shape, const Xform4 &xform
);

// Point containment test
bool point_in_shape_4d(
	const Vector4 &point,
	const Shape4DInternal *shape, const Xform4 &xform
);
