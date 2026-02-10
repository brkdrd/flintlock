#include "hyperplane4d.h"

// -- Constructors ------------------------------------------------------------

Hyperplane4D::Hyperplane4D() :
		normal(0, 0, 0, 1), d(0) {
}

Hyperplane4D::Hyperplane4D(const Vector4 &p_normal, real_t p_d) :
		normal(p_normal), d(p_d) {
}

Hyperplane4D::Hyperplane4D(const Vector4 &p_normal, const Vector4 &p_point) :
		normal(p_normal) {
	// d = normal · point (so that normal · point = d for points on the plane)
	d = normal.dot(p_point);
}

// -- Operations --------------------------------------------------------------

real_t Hyperplane4D::distance_to(const Vector4 &p_point) const {
	// Signed distance from point to hyperplane
	// Equation: normal · x = d for points on the plane
	return normal.dot(p_point) - d;
}

Vector4 Hyperplane4D::project(const Vector4 &p_point) const {
	// Project point onto hyperplane
	return p_point - normal * distance_to(p_point);
}

bool Hyperplane4D::is_point_over(const Vector4 &p_point) const {
	// Returns true if point is on the positive side (normal direction)
	return distance_to(p_point) > 0;
}

Vector4 Hyperplane4D::get_point_on_plane() const {
	// Return a point on the hyperplane
	// Use the closest point to origin: d * normal (for unit normal)
	return d * normal;
}

Hyperplane4D Hyperplane4D::normalized() const {
	real_t len = normal.length();
	if (len < CMP_EPSILON) {
		return Hyperplane4D(); // Return default if degenerate
	}
	return Hyperplane4D(normal / len, d / len);
}

void Hyperplane4D::normalize() {
	real_t len = normal.length();
	if (len < CMP_EPSILON) {
		*this = Hyperplane4D(); // Reset to default if degenerate
		return;
	}
	normal = normal / len;
	d = d / len;
}

bool Hyperplane4D::is_equal_approx(const Hyperplane4D &p_plane) const {
	return normal.is_equal_approx(p_plane.normal) && Math::is_equal_approx(d, p_plane.d);
}

// -- Intersection ------------------------------------------------------------

bool Hyperplane4D::intersects_segment(const Vector4 &p_from, const Vector4 &p_to, Vector4 *r_result) const {
	// Compute signed distances from both endpoints
	real_t dist_from = distance_to(p_from);
	real_t dist_to = distance_to(p_to);

	// Check if endpoints are on opposite sides
	if ((dist_from > 0) == (dist_to > 0)) {
		return false; // Both on same side, no intersection
	}

	// Compute intersection point using linear interpolation
	// t = dist_from / (dist_from - dist_to)
	real_t denom = dist_from - dist_to;
	if (abs(denom) < CMP_EPSILON) {
		return false; // Degenerate case
	}

	real_t t = dist_from / denom;
	if (r_result) {
		*r_result = p_from + (p_to - p_from) * t;
	}
	return true;
}

// -- Tangent basis -----------------------------------------------------------

void Hyperplane4D::get_tangent_basis(Vector4 *r_t1, Vector4 *r_t2, Vector4 *r_t3) const {
	// Construct an orthonormal basis for the 3D hyperplane.
	// Use Gram-Schmidt to generate 3 vectors orthogonal to the normal.

	// Start with arbitrary non-parallel vector
	Vector4 candidate1;
	if (abs(normal.x) < 0.9) {
		candidate1 = Vector4(1, 0, 0, 0);
	} else {
		candidate1 = Vector4(0, 1, 0, 0);
	}

	// First tangent: orthogonalize candidate1 against normal
	Vector4 t1 = candidate1 - normal * normal.dot(candidate1);
	t1 = t1.normalized();

	// Second tangent: use another arbitrary vector
	Vector4 candidate2;
	if (abs(normal.y) < 0.9) {
		candidate2 = Vector4(0, 1, 0, 0);
	} else {
		candidate2 = Vector4(0, 0, 1, 0);
	}

	// Orthogonalize against normal and t1
	Vector4 t2 = candidate2 - normal * normal.dot(candidate2) - t1 * t1.dot(candidate2);
	t2 = t2.normalized();

	// Third tangent: use yet another arbitrary vector
	Vector4 candidate3;
	if (abs(normal.z) < 0.9) {
		candidate3 = Vector4(0, 0, 1, 0);
	} else {
		candidate3 = Vector4(0, 0, 0, 1);
	}

	// Orthogonalize against normal, t1, and t2
	Vector4 t3 = candidate3 - normal * normal.dot(candidate3) - t1 * t1.dot(candidate3) - t2 * t2.dot(candidate3);
	t3 = t3.normalized();

	if (r_t1) *r_t1 = t1;
	if (r_t2) *r_t2 = t2;
	if (r_t3) *r_t3 = t3;
}
