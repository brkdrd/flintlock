#pragma once

#include "slice_result.h"
#include "shapes/hyper_box_shape_4d.h"
#include "shapes/convex_hull_4d.h"
#include "math/hyperplane4d.h"
#include "math/transform4d.h"
#include <vector>

/// PolytopeSlicer — Edge-clipping algorithm for convex polytopes.
///
/// Slices convex 4D shapes (boxes, hulls) by:
/// 1. Classifying vertices as front/back relative to hyperplane
/// 2. Finding edge crossings where endpoints are on opposite sides
/// 3. Collecting intersection points to form a 3D convex polytope
/// 4. Triangulating the result
class PolytopeSlicer {
public:
	/// Slice a hyperbox (tesseract) with a hyperplane
	static SliceResult slice(
		const HyperBoxShape4D *p_box,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

	/// Slice a convex hull with a hyperplane
	static SliceResult slice(
		const ConvexHull4D *p_hull,
		const Transform4D &p_transform,
		const Hyperplane4D &p_hyperplane
	);

private:
	/// Slice any set of vertices and edges
	static SliceResult slice_vertices(
		const std::vector<Vector4> &p_vertices,
		const std::vector<std::pair<int, int>> &p_edges,
		const Hyperplane4D &p_hyperplane
	);

	/// Generate edges for a tesseract
	static std::vector<std::pair<int, int>> generate_tesseract_edges();

	/// Generate edges for a convex hull (all pairs - conservative)
	static std::vector<std::pair<int, int>> generate_hull_edges(int p_vertex_count);

	/// Triangulate a convex polygon in 3D
	static void triangulate_convex_polygon(
		SliceResult &r_result,
		const std::vector<Vector3> &p_vertices
	);

	/// Project 4D point onto hyperplane's 3D coordinate system
	static Vector3 project_to_3d(
		const Vector4 &p_point_4d,
		const Hyperplane4D &p_hyperplane
	);
};
