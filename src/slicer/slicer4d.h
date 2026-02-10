#pragma once

#include "slice_result.h"
#include "shapes/shape4d.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "shapes/hyper_box_shape_4d.h"
#include "shapes/hyper_capsule_shape_4d.h"
#include "shapes/hyper_ellipsoid_shape_4d.h"
#include "shapes/convex_hull_4d.h"
#include "math/hyperplane4d.h"
#include "math/transform4d.h"

/// Slicer4D — Main dispatcher for slicing 4D shapes with hyperplanes.
///
/// Routes to appropriate specialized slicer based on shape type.
/// Produces 3D cross-section geometry suitable for rendering.
class Slicer4D {
public:
	/// Slice any shape with a hyperplane
	/// Automatically dispatches to the appropriate slicer implementation
	static SliceResult slice_shape(
		const Shape4D *p_shape,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a hypersphere (analytic)
	static SliceResult slice_sphere(
		const HyperSphereShape4D *p_sphere,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a hyperbox (edge-clipping)
	static SliceResult slice_box(
		const HyperBoxShape4D *p_box,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a hypercapsule (combination approach)
	static SliceResult slice_capsule(
		const HyperCapsuleShape4D *p_capsule,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a hyperellipsoid (approximation)
	static SliceResult slice_ellipsoid(
		const HyperEllipsoidShape4D *p_ellipsoid,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a convex hull (edge-clipping)
	static SliceResult slice_hull(
		const ConvexHull4D *p_hull,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);
};
