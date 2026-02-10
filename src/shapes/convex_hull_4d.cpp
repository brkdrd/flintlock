#include "convex_hull_4d.h"
#include <cmath>
#include <algorithm>

// -- Constructor -------------------------------------------------------------

ConvexHull4D::ConvexHull4D() {
}

ConvexHull4D::ConvexHull4D(const std::vector<Vector4> &p_vertices) :
		vertices(p_vertices) {
}

// -- Accessors ---------------------------------------------------------------

void ConvexHull4D::set_vertices(const std::vector<Vector4> &p_vertices) {
	vertices = p_vertices;
}

const std::vector<Vector4> &ConvexHull4D::get_vertices() const {
	return vertices;
}

int ConvexHull4D::get_vertex_count() const {
	return vertices.size();
}

Vector4 ConvexHull4D::get_vertex(int p_index) const {
	if (p_index >= 0 && p_index < (int)vertices.size()) {
		return vertices[p_index];
	}
	return Vector4(0, 0, 0, 0);
}

// -- Shape4D interface -------------------------------------------------------

Vector4 ConvexHull4D::get_support(const Vector4 &p_direction) const {
	if (vertices.empty()) {
		return Vector4(0, 0, 0, 0);
	}

	// Brute-force search: find vertex with maximum dot product
	int best_index = 0;
	real_t best_dot = vertices[0].dot(p_direction);

	for (size_t i = 1; i < vertices.size(); i++) {
		real_t dot = vertices[i].dot(p_direction);
		if (dot > best_dot) {
			best_dot = dot;
			best_index = i;
		}
	}

	return vertices[best_index];
}

AABB4D ConvexHull4D::get_aabb(const Transform4D &p_transform) const {
	if (vertices.empty()) {
		return AABB4D(Vector4(0, 0, 0, 0), Vector4(0, 0, 0, 0));
	}

	// Transform all vertices and compute min/max
	Vector4 first_transformed = p_transform.xform(vertices[0]);
	Vector4 min_corner = first_transformed;
	Vector4 max_corner = first_transformed;

	for (size_t i = 1; i < vertices.size(); i++) {
		Vector4 transformed = p_transform.xform(vertices[i]);

		for (int axis = 0; axis < 4; axis++) {
			min_corner.coord[axis] = std::min(min_corner.coord[axis], transformed.coord[axis]);
			max_corner.coord[axis] = std::max(max_corner.coord[axis], transformed.coord[axis]);
		}
	}

	return AABB4D(min_corner, max_corner - min_corner);
}

bool ConvexHull4D::contains_point(const Vector4 &p_point) const {
	if (vertices.empty()) {
		return false;
	}

	// For a convex hull, a point is inside if it's behind all faces
	// We use a support function approach: check if the point is
	// within the supporting hyperplanes

	// Simple approach: compute centroid and check if point is
	// "inside" relative to each vertex normal

	// Compute centroid
	Vector4 centroid(0, 0, 0, 0);
	for (const auto &v : vertices) {
		centroid += v;
	}
	centroid = centroid / (real_t)vertices.size();

	// Check if point is on the same side as centroid for all vertices
	// This is a simplified containment test that works for star-shaped hulls
	// A more robust test would use proper hyperplane equations

	// For each vertex, check if point is closer to centroid than vertex is
	// This works if the hull is star-shaped from centroid (which convex hulls are)
	for (const auto &vertex : vertices) {
		Vector4 to_vertex = vertex - centroid;
		Vector4 to_point = p_point - centroid;

		// If the point is farther from centroid than the vertex in that direction
		// it might be outside
		real_t vertex_dist_sq = to_vertex.length_squared();
		real_t point_proj = to_point.dot(to_vertex);

		if (point_proj > vertex_dist_sq + CMP_EPSILON) {
			return false;
		}
	}

	// Additional check: use support function
	// A point is inside if Support(hull, centroid - point) . (centroid - point) >= |centroid - point|²
	// This checks if the point is not beyond any supporting hyperplane

	Vector4 dirs[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1),
		(p_point - centroid).normalized()
	};

	for (const auto &dir : dirs) {
		if (dir.length_squared() < CMP_EPSILON) continue;

		Vector4 support = get_support(dir);
		real_t support_dist = support.dot(dir);
		real_t point_dist = p_point.dot(dir);

		// If point is farther in this direction than support point, it's outside
		if (point_dist > support_dist + CMP_EPSILON) {
			return false;
		}
	}

	return true;
}
