#include "basis4d.h"

// -- Constructors ------------------------------------------------------------

Basis4D::Basis4D() {
	rows[0] = Vector4(1, 0, 0, 0);
	rows[1] = Vector4(0, 1, 0, 0);
	rows[2] = Vector4(0, 0, 1, 0);
	rows[3] = Vector4(0, 0, 0, 1);
}

Basis4D::Basis4D(const Vector4 &p_row0, const Vector4 &p_row1, const Vector4 &p_row2, const Vector4 &p_row3) {
	rows[0] = p_row0;
	rows[1] = p_row1;
	rows[2] = p_row2;
	rows[3] = p_row3;
}

Basis4D Basis4D::from_rotation(RotationPlane p_plane, real_t p_angle) {
	Basis4D b;
	real_t c = std::cos(p_angle);
	real_t s = std::sin(p_angle);

	// Each rotation plane swaps two axes: axis_a -> axis_b, axis_b -> -axis_a.
	int a, bb;
	switch (p_plane) {
		case PLANE_XY: a = 0; bb = 1; break;
		case PLANE_XZ: a = 0; bb = 2; break;
		case PLANE_XW: a = 0; bb = 3; break;
		case PLANE_YZ: a = 1; bb = 2; break;
		case PLANE_YW: a = 1; bb = 3; break;
		case PLANE_ZW: a = 2; bb = 3; break;
	}

	b.rows[a].components[a] = c;
	b.rows[a].components[bb] = -s;
	b.rows[bb].components[a] = s;
	b.rows[bb].components[bb] = c;

	return b;
}

Basis4D Basis4D::from_scale(const Vector4 &p_scale) {
	Basis4D b;
	b.rows[0] = Vector4(p_scale.x, 0, 0, 0);
	b.rows[1] = Vector4(0, p_scale.y, 0, 0);
	b.rows[2] = Vector4(0, 0, p_scale.z, 0);
	b.rows[3] = Vector4(0, 0, 0, p_scale.w);
	return b;
}

// -- Row/column access -------------------------------------------------------

Vector4 Basis4D::get_row(int p_idx) const {
	return rows[p_idx];
}

void Basis4D::set_row(int p_idx, const Vector4 &p_row) {
	rows[p_idx] = p_row;
}

Vector4 Basis4D::get_column(int p_idx) const {
	return Vector4(rows[0].components[p_idx], rows[1].components[p_idx],
			rows[2].components[p_idx], rows[3].components[p_idx]);
}

void Basis4D::set_column(int p_idx, const Vector4 &p_col) {
	rows[0].components[p_idx] = p_col.x;
	rows[1].components[p_idx] = p_col.y;
	rows[2].components[p_idx] = p_col.z;
	rows[3].components[p_idx] = p_col.w;
}

// -- Transform ---------------------------------------------------------------

Vector4 Basis4D::xform(const Vector4 &p_vec) const {
	return Vector4(
			rows[0].dot(p_vec),
			rows[1].dot(p_vec),
			rows[2].dot(p_vec),
			rows[3].dot(p_vec));
}

Vector4 Basis4D::xform_inv(const Vector4 &p_vec) const {
	// Multiply by transpose (columns instead of rows).
	return Vector4(
			rows[0].x * p_vec.x + rows[1].x * p_vec.y + rows[2].x * p_vec.z + rows[3].x * p_vec.w,
			rows[0].y * p_vec.x + rows[1].y * p_vec.y + rows[2].y * p_vec.z + rows[3].y * p_vec.w,
			rows[0].z * p_vec.x + rows[1].z * p_vec.y + rows[2].z * p_vec.z + rows[3].z * p_vec.w,
			rows[0].w * p_vec.x + rows[1].w * p_vec.y + rows[2].w * p_vec.z + rows[3].w * p_vec.w);
}

// -- Matrix operations -------------------------------------------------------

Basis4D Basis4D::operator*(const Basis4D &p_other) const {
	Basis4D result;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			real_t sum = 0;
			for (int k = 0; k < 4; k++) {
				sum += rows[i].components[k] * p_other.rows[k].components[j];
			}
			result.rows[i].components[j] = sum;
		}
	}
	return result;
}

Basis4D Basis4D::transposed() const {
	Basis4D t;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			t.rows[i].components[j] = rows[j].components[i];
		}
	}
	return t;
}

// Helper: 3x3 determinant from 9 elements.
static real_t det3(real_t a, real_t b, real_t c,
		real_t d, real_t e, real_t f,
		real_t g, real_t h, real_t i) {
	return a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
}

real_t Basis4D::determinant() const {
	// Laplace expansion along the first row.
	const auto &r = rows;
	real_t cofactor0 = det3(
			r[1].y, r[1].z, r[1].w,
			r[2].y, r[2].z, r[2].w,
			r[3].y, r[3].z, r[3].w);
	real_t cofactor1 = det3(
			r[1].x, r[1].z, r[1].w,
			r[2].x, r[2].z, r[2].w,
			r[3].x, r[3].z, r[3].w);
	real_t cofactor2 = det3(
			r[1].x, r[1].y, r[1].w,
			r[2].x, r[2].y, r[2].w,
			r[3].x, r[3].y, r[3].w);
	real_t cofactor3 = det3(
			r[1].x, r[1].y, r[1].z,
			r[2].x, r[2].y, r[2].z,
			r[3].x, r[3].y, r[3].z);

	return r[0].x * cofactor0 - r[0].y * cofactor1 + r[0].z * cofactor2 - r[0].w * cofactor3;
}

Basis4D Basis4D::inverse() const {
	// Compute the adjugate (transpose of cofactor matrix) divided by determinant.
	real_t det = determinant();
	if (std::abs(det) < CMP_EPSILON) {
		return Basis4D(); // Return identity for singular matrices.
	}

	real_t inv_det = 1.0 / det;
	Basis4D result;
	const auto &m = rows;

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			// Build the 3x3 minor by excluding row j and column i (transposed).
			real_t minor[9];
			int idx = 0;
			for (int r = 0; r < 4; r++) {
				if (r == j) continue;
				for (int c = 0; c < 4; c++) {
					if (c == i) continue;
					minor[idx++] = m[r].components[c];
				}
			}
			real_t cofactor = det3(minor[0], minor[1], minor[2],
					minor[3], minor[4], minor[5],
					minor[6], minor[7], minor[8]);
			// Checkerboard sign: (-1)^(i+j)
			if ((i + j) % 2 != 0) {
				cofactor = -cofactor;
			}
			result.rows[i].components[j] = cofactor * inv_det;
		}
	}
	return result;
}

// -- Queries -----------------------------------------------------------------

bool Basis4D::is_orthogonal() const {
	Basis4D product = *this * transposed();
	Basis4D identity;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (std::abs(product.rows[i].components[j] - identity.rows[i].components[j]) > CMP_EPSILON) {
				return false;
			}
		}
	}
	return true;
}

bool Basis4D::is_rotation() const {
	return is_orthogonal() && std::abs(determinant() - 1.0) < CMP_EPSILON;
}

// -- Orthonormalization ------------------------------------------------------

void Basis4D::orthonormalize() {
	// 4D Gram-Schmidt on columns.
	for (int i = 0; i < 4; i++) {
		Vector4 col = get_column(i);
		for (int j = 0; j < i; j++) {
			Vector4 prev = get_column(j);
			col = col - prev * col.dot(prev);
		}
		real_t len = col.length();
		if (len > CMP_EPSILON) {
			col = col / len;
		}
		set_column(i, col);
	}
}

Basis4D Basis4D::orthonormalized() const {
	Basis4D copy = *this;
	copy.orthonormalize();
	return copy;
}
