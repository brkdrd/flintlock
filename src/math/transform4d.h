#pragma once

#include "basis4d.h"

// Transform4D — Basis4D + Vector4 origin representing an affine transform in 4D.
//
// The default constructor produces an identity transform.

class Transform4D {
public:
	Basis4D basis;
	Vector4 origin;

	// -- Constructors --------------------------------------------------------

	Transform4D();
	Transform4D(const Basis4D &p_basis, const Vector4 &p_origin);

	// -- Transform points and directions -------------------------------------

	Vector4 xform(const Vector4 &p_point) const;
	Vector4 xform_inv(const Vector4 &p_point) const;
	Vector4 basis_xform(const Vector4 &p_dir) const;

	// -- Composition ---------------------------------------------------------

	Transform4D operator*(const Transform4D &p_other) const;

	// -- Inverse -------------------------------------------------------------

	Transform4D inverse() const;
	Transform4D affine_inverse() const;

	// -- Builders ------------------------------------------------------------

	static Transform4D translated(const Vector4 &p_offset);
	Transform4D rotated(const Basis4D &p_rotation) const;
	Transform4D scaled(const Vector4 &p_scale) const;
};
