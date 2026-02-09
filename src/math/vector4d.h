#pragma once

#include "math_defs_4d.h"

// Vector4D -- 4D vector type for the Flintlock physics engine.
//
// Provides all standard vector operations plus 4D-specific operations needed
// for 4D physics simulation: triple cross product, plane rotation, hyperplane
// reflection, projection/rejection, and more.
//
// This is a value type with the same memory layout as Godot's Vector4.
// Implicit conversions to/from Vector4 are provided for interoperability.

class Vector4D {
public:
	union {
		struct {
			real_t x, y, z, w;
		};
		real_t components[4];
	};

	// -- Constructors --------------------------------------------------------

	Vector4D();
	Vector4D(real_t p_x, real_t p_y, real_t p_z, real_t p_w);
	Vector4D(const Vector4 &p_v);

	// -- Conversion ----------------------------------------------------------

	operator Vector4() const;

	// -- Element access ------------------------------------------------------

	real_t &operator[](int p_idx);
	const real_t &operator[](int p_idx) const;

	// -- Arithmetic operators ------------------------------------------------

	Vector4D operator+(const Vector4D &p_other) const;
	Vector4D operator-(const Vector4D &p_other) const;
	Vector4D operator*(real_t p_scalar) const;
	Vector4D operator/(real_t p_scalar) const;
	Vector4D operator-() const;
	Vector4D &operator+=(const Vector4D &p_other);
	Vector4D &operator-=(const Vector4D &p_other);
	Vector4D &operator*=(real_t p_scalar);
	Vector4D &operator/=(real_t p_scalar);
	bool operator==(const Vector4D &p_other) const;
	bool operator!=(const Vector4D &p_other) const;

	// -- Component-wise operations -------------------------------------------

	Vector4D component_mul(const Vector4D &p_other) const;
	Vector4D component_div(const Vector4D &p_other) const;
	Vector4D abs() const;
	Vector4D sign() const;
	Vector4D floor() const;
	Vector4D ceil() const;
	Vector4D round() const;
	Vector4D clamp(const Vector4D &p_min, const Vector4D &p_max) const;
	Vector4D snapped(const Vector4D &p_step) const;

	// -- Length and distance --------------------------------------------------

	real_t dot(const Vector4D &p_other) const;
	real_t length_squared() const;
	real_t length() const;
	real_t distance_to(const Vector4D &p_other) const;
	real_t distance_squared_to(const Vector4D &p_other) const;

	// -- Normalization -------------------------------------------------------

	Vector4D normalized() const;
	void normalize();
	bool is_normalized() const;
	Vector4D safe_normalized(const Vector4D &p_fallback = Vector4D()) const;

	// -- Comparison ----------------------------------------------------------

	bool is_zero_approx() const;
	bool is_equal_approx(const Vector4D &p_other) const;
	bool is_finite() const;

	// -- Interpolation -------------------------------------------------------

	Vector4D lerp(const Vector4D &p_to, real_t p_weight) const;
	Vector4D move_toward(const Vector4D &p_target, real_t p_delta) const;

	// -- Axis queries --------------------------------------------------------

	int min_axis_index() const;
	int max_axis_index() const;
	Vector4D direction_to(const Vector4D &p_other) const;

	// -- 4D-specific: triple cross product -----------------------------------

	// The 4D generalization of the cross product. Given three vectors in 4D,
	// returns a vector perpendicular to all three. This is the Hodge dual of
	// the trivector u ^ v ^ w.
	static Vector4D triple_cross(const Vector4D &p_u, const Vector4D &p_v, const Vector4D &p_w);

	// -- 4D-specific: projection and rejection -------------------------------

	// Project this vector onto another vector.
	Vector4D project(const Vector4D &p_onto) const;

	// Component of this vector perpendicular to another vector.
	Vector4D reject(const Vector4D &p_from) const;

	// -- 4D-specific: reflection and sliding ---------------------------------

	// Reflect this vector across a hyperplane with the given unit normal.
	// The normal must be normalized.
	Vector4D reflect(const Vector4D &p_normal) const;

	// Remove the component of this vector along the normal (slide along
	// the hyperplane). The normal must be normalized.
	Vector4D slide(const Vector4D &p_normal) const;

	// -- 4D-specific: angles -------------------------------------------------

	// Angle in radians between this vector and another.
	real_t angle_to(const Vector4D &p_other) const;

	// Check if this vector is perpendicular to another (dot product ~ 0).
	bool is_perpendicular_to(const Vector4D &p_other) const;

	// -- 4D-specific: plane rotation -----------------------------------------

	// Rotate this vector in the plane defined by two coordinate axes.
	// p_axis1 and p_axis2 are axis indices (0=x, 1=y, 2=z, 3=w).
	// Rotates from p_axis1 toward p_axis2 by p_angle radians.
	Vector4D rotated_in_plane(int p_axis1, int p_axis2, real_t p_angle) const;

	// -- 4D-specific: perpendicular construction -----------------------------

	// Find an arbitrary unit vector perpendicular to this one.
	// This vector must be non-zero.
	Vector4D find_any_perpendicular() const;

	// -- Named constants -----------------------------------------------------

	static const Vector4D ZERO;
	static const Vector4D ONE;
	static const Vector4D AXIS_X;
	static const Vector4D AXIS_Y;
	static const Vector4D AXIS_Z;
	static const Vector4D AXIS_W;
};

Vector4D operator*(real_t p_scalar, const Vector4D &p_vec);
