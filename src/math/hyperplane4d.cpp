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
	// Construct an orthonormal basis for the 3D hyperplane using Gram-Schmidt.
	// Uses a fixed priority axis order to prevent basis flipping between frames:
	// always choose the world axis LEAST parallel to the normal as the seed,
	// breaking ties by the fixed order X > Y > Z > W (lowest index wins).

	const Vector4 world_axes[4] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, 0, 1),
	};

	// Sort axis indices by ascending |normal · axis|.  Ties keep original order.
	int order[4] = { 0, 1, 2, 3 };
	real_t abs_d[4] = {
		std::abs(normal.x), std::abs(normal.y),
		std::abs(normal.z), std::abs(normal.w)
	};
	// Simple insertion sort (4 elements — no overhead).
	for (int i = 1; i < 4; i++) {
		int key = order[i];
		int j = i - 1;
		while (j >= 0 && abs_d[order[j]] > abs_d[key]) {
			order[j + 1] = order[j];
			j--;
		}
		order[j + 1] = key;
	}

	// First tangent: seed from the axis most orthogonal to normal.
	Vector4 t1 = world_axes[order[0]];
	t1 = t1 - normal * normal.dot(t1);
	t1 = t1.normalized();

	// Second tangent: seed from the next most orthogonal axis.
	Vector4 t2 = world_axes[order[1]];
	t2 = t2 - normal * normal.dot(t2) - t1 * t1.dot(t2);
	t2 = t2.normalized();

	// Third tangent: seed from the next axis.
	Vector4 t3 = world_axes[order[2]];
	t3 = t3 - normal * normal.dot(t3) - t1 * t1.dot(t3) - t2 * t2.dot(t3);
	t3 = t3.normalized();

	if (r_t1) *r_t1 = t1;
	if (r_t2) *r_t2 = t2;
	if (r_t3) *r_t3 = t3;
}
