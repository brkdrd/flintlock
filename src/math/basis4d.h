#pragma once

#include "math_defs_4d.h"

// Basis4D — 4x4 matrix representing rotation and scale in 4D.
//
// Stored as 4 row Vector4s. The default constructor produces an identity matrix.
// Provides rotation constructors for all 6 coordinate planes, matrix arithmetic,
// transpose, inverse, and determinant.

class Basis4D {
public:
	Vector4 rows[4];

	// -- Rotation plane indices ----------------------------------------------

	enum RotationPlane {
		PLANE_XY = 0,
		PLANE_XZ = 1,
		PLANE_XW = 2,
		PLANE_YZ = 3,
		PLANE_YW = 4,
		PLANE_ZW = 5,
	};

	// -- Constructors --------------------------------------------------------

	Basis4D();
	Basis4D(const Vector4 &p_row0, const Vector4 &p_row1, const Vector4 &p_row2, const Vector4 &p_row3);

	static Basis4D from_rotation(RotationPlane p_plane, real_t p_angle);
	static Basis4D from_scale(const Vector4 &p_scale);

	// -- Row/column access ---------------------------------------------------

	Vector4 get_row(int p_idx) const;
	void set_row(int p_idx, const Vector4 &p_row);
	Vector4 get_column(int p_idx) const;
	void set_column(int p_idx, const Vector4 &p_col);

	// -- Transform -----------------------------------------------------------

	Vector4 xform(const Vector4 &p_vec) const;
	Vector4 xform_inv(const Vector4 &p_vec) const;

	// -- Matrix operations ---------------------------------------------------

	Basis4D operator*(const Basis4D &p_other) const;
	Basis4D transposed() const;
	Basis4D inverse() const;
	real_t determinant() const;

	// -- Queries -------------------------------------------------------------

	bool is_orthogonal() const;
	bool is_rotation() const;

	// -- Orthonormalization ---------------------------------------------------

	void orthonormalize();
	Basis4D orthonormalized() const;
};
