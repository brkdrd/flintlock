#pragma once

#include "slice_result.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "math/hyperplane4d.h"
#include "math/transform4d.h"

/// SphereSlicer — Analytic slicing of 4D spheres.
///
/// Uses the mathematical fact that a hyperplane intersection with a 4-sphere
/// produces a 3-sphere (or is empty). The radius of the 3-sphere is computed
/// analytically from the distance to the hyperplane.
class SphereSlicer {
public:
	/// Slice a hypersphere with a hyperplane
	/// Returns a 3D sphere (represented as vertices) or empty if no intersection
	static SliceResult slice(
		const HyperSphereShape4D *p_sphere,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

private:
	/// Generate a tessellated 3D sphere mesh
	static void generate_sphere_mesh(
		SliceResult &r_result,
		real_t p_radius,
		int p_segments = 16
	);
};
