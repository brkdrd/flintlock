#pragma once

#include "basis4d.h"

// Rotor4D — even-grade element of Cl(4,0) representing a rotation in 4D.
//
// The even subalgebra has 8 components:
//   1 scalar, 6 bivectors (e12 e13 e14 e23 e24 e34), 1 pseudoscalar (e1234).
//
// A simple rotation in a single plane has the form:
//   R = cos(theta/2) + sin(theta/2) * B_hat
// where B_hat is the unit bivector of the rotation plane.
//
// Vectors are rotated via the sandwich product: v' = R v R†

class Rotor4D {
public:
	// Components: scalar, 6 bivectors, pseudoscalar.
	real_t s;    // Scalar part.
	real_t e12;  // XY bivector.
	real_t e13;  // XZ bivector.
	real_t e14;  // XW bivector.
	real_t e23;  // YZ bivector.
	real_t e24;  // YW bivector.
	real_t e34;  // ZW bivector.
	real_t e1234; // Pseudoscalar.

	// -- Rotation plane indices (matching Basis4D) ---------------------------

	enum RotationPlane {
		PLANE_XY = 0,
		PLANE_XZ = 1,
		PLANE_XW = 2,
		PLANE_YZ = 3,
		PLANE_YW = 4,
		PLANE_ZW = 5,
	};

	// -- Constructors --------------------------------------------------------

	Rotor4D();
	Rotor4D(real_t p_s, real_t p_e12, real_t p_e13, real_t p_e14,
			real_t p_e23, real_t p_e24, real_t p_e34, real_t p_e1234);

	static Rotor4D from_plane_angle(RotationPlane p_plane, real_t p_angle);

	// -- Sandwich product (rotation) -----------------------------------------

	Vector4 rotate(const Vector4 &p_v) const;

	// -- Geometric product (composition) -------------------------------------

	Rotor4D operator*(const Rotor4D &p_other) const;

	// -- Conjugate (reverse) -------------------------------------------------

	Rotor4D conjugate() const;

	// -- Norm ----------------------------------------------------------------

	real_t norm_squared() const;
	real_t norm() const;
	void normalize();
	Rotor4D normalized() const;

	// -- Slerp ---------------------------------------------------------------

	Rotor4D slerp(const Rotor4D &p_to, real_t p_t) const;

	// -- Conversion ----------------------------------------------------------

	Basis4D to_basis() const;
	static Rotor4D from_basis(const Basis4D &p_basis);
};
