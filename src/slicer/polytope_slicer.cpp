#include "polytope_slicer.h"
#include <algorithm>

SliceResult PolytopeSlicer::slice(
	const HyperBoxShape4D *p_box,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	if (!p_box) {
		return SliceResult();
	}

	// Generate all 16 vertices of the tesseract
	Vector4 half_extents = p_box->get_half_extents();
	std::vector<Vector4> vertices;

	for (int i = 0; i < 16; i++) {
		Vector4 vertex(
			(i & 1) ? half_extents.x : -half_extents.x,
			(i & 2) ? half_extents.y : -half_extents.y,
			(i & 4) ? half_extents.z : -half_extents.z,
			(i & 8) ? half_extents.w : -half_extents.w
		);

		// Transform to world space
		vertices.push_back(p_transform.xform(vertex));
	}

	// Generate tesseract edges
	auto edges = generate_tesseract_edges();

	return slice_vertices(vertices, edges, p_hyperplane);
}

SliceResult PolytopeSlicer::slice(
	const ConvexHull4D *p_hull,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	if (!p_hull || p_hull->get_vertex_count() == 0) {
		return SliceResult();
	}

	// Transform all vertices to world space
	std::vector<Vector4> vertices;
	for (int i = 0; i < p_hull->get_vertex_count(); i++) {
		Vector4 vertex = p_hull->get_vertex(i);
		vertices.push_back(p_transform.xform(vertex));
	}

	// Generate edges (conservative: test all pairs)
	auto edges = generate_hull_edges(vertices.size());

	return slice_vertices(vertices, edges, p_hyperplane);
}

SliceResult PolytopeSlicer::slice_vertices(
	const std::vector<Vector4> &p_vertices,
	const std::vector<std::pair<int, int>> &p_edges,
	const Hyperplane4D &p_hyperplane
) {
	SliceResult result;

	if (p_vertices.empty()) {
		return result;
	}

	// Classify vertices relative to hyperplane
	std::vector<real_t> distances;
	for (const auto &vertex : p_vertices) {
		distances.push_back(p_hyperplane.distance_to(vertex));
	}

	// Find edge crossings
	std::vector<Vector4> intersection_points_4d;

	for (const auto &edge : p_edges) {
		int i0 = edge.first;
		int i1 = edge.second;

		real_t d0 = distances[i0];
		real_t d1 = distances[i1];

		// Check if endpoints are on opposite sides
		if ((d0 > 0) != (d1 > 0) || (std::abs(d0) < CMP_EPSILON || std::abs(d1) < CMP_EPSILON)) {
			// Edge crosses the hyperplane
			Vector4 intersection;
			if (p_hyperplane.intersects_segment(p_vertices[i0], p_vertices[i1], &intersection)) {
				intersection_points_4d.push_back(intersection);
			}
		}
	}

	// If no intersections, return empty
	if (intersection_points_4d.empty()) {
		return result;
	}

	// Project 4D intersection points to 3D hyperplane coordinates
	std::vector<Vector3> vertices_3d;
	for (const auto &point_4d : intersection_points_4d) {
		vertices_3d.push_back(project_to_3d(point_4d, p_hyperplane));
	}

	// Remove duplicate vertices
	std::vector<Vector3> unique_vertices;
	for (const auto &v : vertices_3d) {
		bool is_duplicate = false;
		for (const auto &existing : unique_vertices) {
			if (existing.is_equal_approx(v)) {
				is_duplicate = true;
				break;
			}
		}
		if (!is_duplicate) {
			unique_vertices.push_back(v);
		}
	}

	// If we have at least 3 vertices, create geometry
	if (unique_vertices.size() >= 3) {
		// Triangulate the convex polygon
		triangulate_convex_polygon(result, unique_vertices);
	}

	return result;
}

std::vector<std::pair<int, int>> PolytopeSlicer::generate_tesseract_edges() {
	// A tesseract has 32 edges
	// Each vertex is connected to 4 neighbors (differing in exactly one bit)
	std::vector<std::pair<int, int>> edges;

	for (int i = 0; i < 16; i++) {
		// Connect to vertices differing by one bit
		for (int bit = 0; bit < 4; bit++) {
			int neighbor = i ^ (1 << bit); // Flip one bit
			if (neighbor > i) { // Avoid duplicates
				edges.push_back({i, neighbor});
			}
		}
	}

	return edges;
}

std::vector<std::pair<int, int>> PolytopeSlicer::generate_hull_edges(int p_vertex_count) {
	// For a convex hull, we conservatively test all edge pairs
	// A proper implementation would compute the actual hull edges
	std::vector<std::pair<int, int>> edges;

	for (int i = 0; i < p_vertex_count; i++) {
		for (int j = i + 1; j < p_vertex_count; j++) {
			edges.push_back({i, j});
		}
	}

	return edges;
}

void PolytopeSlicer::triangulate_convex_polygon(
	SliceResult &r_result,
	const std::vector<Vector3> &p_vertices
) {
	if (p_vertices.size() < 3) {
		return;
	}

	// For a convex polygon, we can use fan triangulation
	// But first, we need to order the vertices

	// Compute centroid
	Vector3 centroid(0, 0, 0);
	for (const auto &v : p_vertices) {
		centroid = centroid + v;
	}
	centroid = centroid / (real_t)p_vertices.size();

	// Add centroid as first vertex
	int base_index = r_result.vertex_count();
	r_result.add_vertex(centroid);
	r_result.add_normal(Vector3(0, 0, 1)); // Default normal

	// Add all polygon vertices
	for (const auto &v : p_vertices) {
		r_result.add_vertex(v);
		r_result.add_normal(Vector3(0, 0, 1)); // Default normal
	}

	// Create fan triangulation from centroid
	int n = p_vertices.size();
	for (int i = 0; i < n; i++) {
		int next = (i + 1) % n;
		r_result.add_triangle(
			base_index,           // Centroid
			base_index + 1 + i,   // Current vertex
			base_index + 1 + next // Next vertex
		);
	}
}

Vector3 PolytopeSlicer::project_to_3d(
	const Vector4 &p_point_4d,
	const Hyperplane4D &p_hyperplane
) {
	// Project the 4D point onto the hyperplane
	Vector4 projected = p_hyperplane.project(p_point_4d);

	// Get the hyperplane's tangent basis (3 orthonormal vectors)
	Vector4 t1, t2, t3;
	p_hyperplane.get_tangent_basis(&t1, &t2, &t3);

	// Get a reference point on the hyperplane
	Vector4 origin = p_hyperplane.get_point_on_plane();

	// Express the projected point in the tangent basis
	Vector4 offset = projected - origin;

	// Project onto tangent basis to get 3D coordinates
	real_t x = offset.dot(t1);
	real_t y = offset.dot(t2);
	real_t z = offset.dot(t3);

	return Vector3(x, y, z);
}
