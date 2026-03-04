#include "basis4d.h"

#include <godot_cpp/core/class_db.hpp>
#include <Eigen/Dense>
#include <cmath>
#include <cstring>

namespace godot {

using EigenMat = Eigen::Matrix<float, 4, 4, Eigen::ColMajor>;

// ─── Eigen helpers ────────────────────────────────────────────────────────────

static EigenMat to_eigen(const real_t d[4][4]) {
	EigenMat m;
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			m(r, c) = (float)d[c][r];
	return m;
}

static void from_eigen(real_t d[4][4], const EigenMat &m) {
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			d[c][r] = (real_t)m(r, c);
}

// ─── Factories ────────────────────────────────────────────────────────────────

Ref<Basis4D> Basis4D::create_identity() {
	Ref<Basis4D> b;
	b.instantiate();
	return b;
}

Ref<Basis4D> Basis4D::from_columns(
		const Ref<Vector4D> &c0, const Ref<Vector4D> &c1,
		const Ref<Vector4D> &c2, const Ref<Vector4D> &c3) {
	Ref<Basis4D> b;
	b.instantiate();
	b->data[0][0] = c0->x; b->data[0][1] = c0->y; b->data[0][2] = c0->z; b->data[0][3] = c0->w;
	b->data[1][0] = c1->x; b->data[1][1] = c1->y; b->data[1][2] = c1->z; b->data[1][3] = c1->w;
	b->data[2][0] = c2->x; b->data[2][1] = c2->y; b->data[2][2] = c2->z; b->data[2][3] = c2->w;
	b->data[3][0] = c3->x; b->data[3][1] = c3->y; b->data[3][2] = c3->z; b->data[3][3] = c3->w;
	return b;
}

// Build a rotation matrix for the given plane at the given angle.
// A 4D rotation in plane (i,j) rotates the i-j plane, leaving the other two axes fixed.
Ref<Basis4D> Basis4D::from_rotation(int p_plane, real_t p_angle) {
	Ref<Basis4D> b = create_identity();
	float c = std::cos(p_angle);
	float s = std::sin(p_angle);

	// (i, j) indices for each plane
	static const int plane_i[6] = {0, 0, 0, 1, 1, 2};
	static const int plane_j[6] = {1, 2, 3, 2, 3, 3};

	if (p_plane < 0 || p_plane > 5) return b;
	int pi = plane_i[p_plane];
	int pj = plane_j[p_plane];

	// Rotation matrix for plane (pi, pj): identity except 2x2 block
	b->data[pi][pi] = c;
	b->data[pj][pi] = s;
	b->data[pi][pj] = -s;
	b->data[pj][pj] = c;
	return b;
}

// ─── Column / row access ──────────────────────────────────────────────────────

Ref<Vector4D> Basis4D::get_column(int p_col) const {
	if (p_col < 0 || p_col > 3) return Vector4D::_make(0, 0, 0, 0);
	return Vector4D::_make(data[p_col][0], data[p_col][1], data[p_col][2], data[p_col][3]);
}
void Basis4D::set_column(int p_col, const Ref<Vector4D> &p_vec) {
	if (p_col < 0 || p_col > 3) return;
	data[p_col][0] = p_vec->x; data[p_col][1] = p_vec->y;
	data[p_col][2] = p_vec->z; data[p_col][3] = p_vec->w;
}
Ref<Vector4D> Basis4D::get_row(int p_row) const {
	if (p_row < 0 || p_row > 3) return Vector4D::_make(0, 0, 0, 0);
	return Vector4D::_make(data[0][p_row], data[1][p_row], data[2][p_row], data[3][p_row]);
}

// ─── Rotation ─────────────────────────────────────────────────────────────────

Ref<Basis4D> Basis4D::rotated(int p_plane, real_t p_angle) const {
	Ref<Basis4D> rot = from_rotation(p_plane, p_angle);
	return rot->multiplied(Ref<Basis4D>(const_cast<Basis4D *>(this)));
}
void Basis4D::rotate(int p_plane, real_t p_angle) {
	Ref<Basis4D> r = rotated(p_plane, p_angle);
	std::memcpy(data, r->data, sizeof(data));
}

// ─── Scale ────────────────────────────────────────────────────────────────────

Ref<Basis4D> Basis4D::scaled(const Ref<Vector4D> &p_scale) const {
	Ref<Basis4D> b;
	b.instantiate();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[c][r] = data[c][r];
	// Scale each column
	real_t sv[4] = {p_scale->x, p_scale->y, p_scale->z, p_scale->w};
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[c][r] *= sv[r];
	return b;
}

Ref<Vector4D> Basis4D::get_scale() const {
	return Vector4D::_make(
		get_column(0)->length(),
		get_column(1)->length(),
		get_column(2)->length(),
		get_column(3)->length()
	);
}

// ─── Orthonormalize (Gram-Schmidt) ────────────────────────────────────────────

Ref<Basis4D> Basis4D::orthonormalized() const {
	Ref<Basis4D> b;
	b.instantiate();
	std::memcpy(b->data, data, sizeof(data));
	b->orthonormalize();
	return b;
}

void Basis4D::orthonormalize() {
	// Gram-Schmidt on columns
	Ref<Vector4D> cols[4];
	for (int i = 0; i < 4; i++) cols[i] = get_column(i);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			real_t proj = cols[i]->dot(cols[j]);
			cols[i] = cols[i]->subtracted(cols[j]->multiplied_scalar(proj));
		}
		cols[i] = cols[i]->normalized();
		set_column(i, cols[i]);
	}
}

// ─── Linear algebra ───────────────────────────────────────────────────────────

real_t Basis4D::determinant() const {
	EigenMat m = to_eigen(data);
	return (real_t)m.determinant();
}

Ref<Basis4D> Basis4D::inverse() const {
	EigenMat m = to_eigen(data);
	EigenMat inv = m.inverse();
	Ref<Basis4D> b;
	b.instantiate();
	from_eigen(b->data, inv);
	return b;
}

Ref<Basis4D> Basis4D::transposed() const {
	Ref<Basis4D> b;
	b.instantiate();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[r][c] = data[c][r];
	return b;
}

// ─── Matrix multiplication ────────────────────────────────────────────────────

Ref<Basis4D> Basis4D::multiplied(const Ref<Basis4D> &p_b) const {
	Ref<Basis4D> result;
	result.instantiate();
	// result = this * p_b
	for (int c = 0; c < 4; c++) {
		for (int r = 0; r < 4; r++) {
			real_t sum = 0;
			for (int k = 0; k < 4; k++)
				sum += data[k][r] * p_b->data[c][k];
			result->data[c][r] = sum;
		}
	}
	return result;
}

Ref<Vector4D> Basis4D::xform(const Ref<Vector4D> &p_v) const {
	return Vector4D::_make(
		data[0][0]*p_v->x + data[1][0]*p_v->y + data[2][0]*p_v->z + data[3][0]*p_v->w,
		data[0][1]*p_v->x + data[1][1]*p_v->y + data[2][1]*p_v->z + data[3][1]*p_v->w,
		data[0][2]*p_v->x + data[1][2]*p_v->y + data[2][2]*p_v->z + data[3][2]*p_v->w,
		data[0][3]*p_v->x + data[1][3]*p_v->y + data[2][3]*p_v->z + data[3][3]*p_v->w
	);
}

Ref<Vector4D> Basis4D::xform_inv(const Ref<Vector4D> &p_v) const {
	return inverse()->xform(p_v);
}

bool Basis4D::is_equal_approx(const Ref<Basis4D> &p_b) const {
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			if (std::abs(data[c][r] - p_b->data[c][r]) > 1e-5f) return false;
	return true;
}

String Basis4D::to_string() const {
	return String("[Basis4D]");
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void Basis4D::_bind_methods() {
	ClassDB::bind_static_method("Basis4D", D_METHOD("create_identity"), &Basis4D::create_identity);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_columns", "c0", "c1", "c2", "c3"), &Basis4D::from_columns);
	ClassDB::bind_static_method("Basis4D", D_METHOD("from_rotation", "plane", "angle"), &Basis4D::from_rotation);

	ClassDB::bind_method(D_METHOD("get_column", "col"), &Basis4D::get_column);
	ClassDB::bind_method(D_METHOD("set_column", "col", "vec"), &Basis4D::set_column);
	ClassDB::bind_method(D_METHOD("get_row", "row"), &Basis4D::get_row);

	ClassDB::bind_method(D_METHOD("rotated", "plane", "angle"), &Basis4D::rotated);
	ClassDB::bind_method(D_METHOD("rotate", "plane", "angle"), &Basis4D::rotate);

	ClassDB::bind_method(D_METHOD("scaled", "scale"), &Basis4D::scaled);
	ClassDB::bind_method(D_METHOD("get_scale"), &Basis4D::get_scale);

	ClassDB::bind_method(D_METHOD("orthonormalized"), &Basis4D::orthonormalized);
	ClassDB::bind_method(D_METHOD("orthonormalize"), &Basis4D::orthonormalize);

	ClassDB::bind_method(D_METHOD("determinant"), &Basis4D::determinant);
	ClassDB::bind_method(D_METHOD("inverse"), &Basis4D::inverse);
	ClassDB::bind_method(D_METHOD("transposed"), &Basis4D::transposed);

	ClassDB::bind_method(D_METHOD("multiplied", "b"), &Basis4D::multiplied);
	ClassDB::bind_method(D_METHOD("xform", "v"), &Basis4D::xform);
	ClassDB::bind_method(D_METHOD("xform_inv", "v"), &Basis4D::xform_inv);

	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &Basis4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("to_string"), &Basis4D::to_string);

	// PlaneAxis enum constants
	BIND_ENUM_CONSTANT(PLANE_XY);
	BIND_ENUM_CONSTANT(PLANE_XZ);
	BIND_ENUM_CONSTANT(PLANE_XW);
	BIND_ENUM_CONSTANT(PLANE_YZ);
	BIND_ENUM_CONSTANT(PLANE_YW);
	BIND_ENUM_CONSTANT(PLANE_ZW);
}

} // namespace godot
