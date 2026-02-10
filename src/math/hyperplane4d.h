#pragma once

#include "math_defs_4d.h"

/// Hyperplane4D represents a 3D hyperplane in 4D space.
/// Defined by a normal vector (unit) and a scalar distance from origin.
/// Equation: normal · point = d (for points on the plane)
class Hyperplane4D {
public:
	Vector4 normal; // Unit normal vector
	real_t d;       // Distance from origin (signed)

	// Constructors
	Hyperplane4D();
	Hyperplane4D(const Vector4 &p_normal, real_t p_d);
	Hyperplane4D(const Vector4 &p_normal, const Vector4 &p_point);

	// Operations
	real_t distance_to(const Vector4 &p_point) const;
	Vector4 project(const Vector4 &p_point) const;
	bool is_point_over(const Vector4 &p_point) const;
	Vector4 get_point_on_plane() const;
	Hyperplane4D normalized() const;
	void normalize();

	// Intersection
	bool intersects_segment(const Vector4 &p_from, const Vector4 &p_to, Vector4 *r_result) const;

	// Tangent basis
	void get_tangent_basis(Vector4 *r_t1, Vector4 *r_t2, Vector4 *r_t3) const;

	// Comparison
	bool is_equal_approx(const Hyperplane4D &p_plane) const;
};
