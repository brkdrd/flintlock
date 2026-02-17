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

#if __has_include(<godot_cpp/variant/array.hpp>)
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/classes/mesh.hpp>
using namespace godot;
#endif

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

#if __has_include(<godot_cpp/variant/array.hpp>)
	/// Convert a SliceResult to a Godot surface arrays Array ready for
	/// RS::mesh_add_surface_from_arrays. Returns an empty Array if the slice
	/// is empty. The returned Array has Mesh::ARRAY_MAX elements.
	static Array build_surface_arrays(const SliceResult &p_result);
#endif

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
