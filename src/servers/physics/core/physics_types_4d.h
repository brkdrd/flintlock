#pragma once

#include <cmath>
#include <cstring>
#include <algorithm>
#include <godot_cpp/variant/vector4.hpp>

using namespace godot;

// ============================================================================
// Lightweight internal physics types - no Ref<>, no GDScript exposure.
// These are POD-like structs for maximum performance in the physics pipeline.
// ============================================================================

static constexpr float PHYSICS_EPSILON = 1e-6f;
static constexpr int NUM_ROTATION_PLANES = 6;

// ============================================================================
// Bivec4 - 6-component bivector for angular velocity/torque in 4D
// Components: XY, XZ, XW, YZ, YW, ZW
// ============================================================================
struct Bivec4 {
	float v[6] = { 0, 0, 0, 0, 0, 0 };

	Bivec4() = default;
	Bivec4(float xy, float xz, float xw, float yz, float yw, float zw) {
		v[0] = xy; v[1] = xz; v[2] = xw;
		v[3] = yz; v[4] = yw; v[5] = zw;
	}

	float &xy() { return v[0]; }
	float &xz() { return v[1]; }
	float &xw() { return v[2]; }
	float &yz() { return v[3]; }
	float &yw() { return v[4]; }
	float &zw() { return v[5]; }
	float xy() const { return v[0]; }
	float xz() const { return v[1]; }
	float xw() const { return v[2]; }
	float yz() const { return v[3]; }
	float yw() const { return v[4]; }
	float zw() const { return v[5]; }

	Bivec4 operator+(const Bivec4 &b) const {
		return { v[0]+b.v[0], v[1]+b.v[1], v[2]+b.v[2], v[3]+b.v[3], v[4]+b.v[4], v[5]+b.v[5] };
	}
	Bivec4 operator-(const Bivec4 &b) const {
		return { v[0]-b.v[0], v[1]-b.v[1], v[2]-b.v[2], v[3]-b.v[3], v[4]-b.v[4], v[5]-b.v[5] };
	}
	Bivec4 operator*(float s) const {
		return { v[0]*s, v[1]*s, v[2]*s, v[3]*s, v[4]*s, v[5]*s };
	}
	Bivec4 &operator+=(const Bivec4 &b) {
		for (int i = 0; i < 6; i++) v[i] += b.v[i];
		return *this;
	}
	Bivec4 &operator-=(const Bivec4 &b) {
		for (int i = 0; i < 6; i++) v[i] -= b.v[i];
		return *this;
	}
	Bivec4 &operator*=(float s) {
		for (int i = 0; i < 6; i++) v[i] *= s;
		return *this;
	}

	float dot(const Bivec4 &b) const {
		float sum = 0;
		for (int i = 0; i < 6; i++) sum += v[i] * b.v[i];
		return sum;
	}
	float length_squared() const { return dot(*this); }
	float length() const { return sqrtf(length_squared()); }
	bool is_zero() const {
		for (int i = 0; i < 6; i++) if (fabsf(v[i]) > PHYSICS_EPSILON) return false;
		return true;
	}

	// Wedge product of two 4D vectors: a ^ b -> bivector
	static Bivec4 wedge(const Vector4 &a, const Vector4 &b) {
		return {
			a.x * b.y - a.y * b.x, // XY
			a.x * b.z - a.z * b.x, // XZ
			a.x * b.w - a.w * b.x, // XW
			a.y * b.z - a.z * b.y, // YZ
			a.y * b.w - a.w * b.y, // YW
			a.z * b.w - a.w * b.z  // ZW
		};
	}

	// Apply bivector as angular velocity to a position vector: omega x r
	// Returns the velocity at point r due to rotation omega.
	// v_i = sum_j W_ij * r_j where W is the antisymmetric matrix of the bivector.
	Vector4 cross(const Vector4 &r) const {
		// W matrix (antisymmetric):
		//     0   -xy  -xz  -xw
		//    xy    0   -yz  -yw
		//    xz   yz    0   -zw
		//    xw   yw   zw    0
		return Vector4(
			-v[0]*r.y - v[1]*r.z - v[2]*r.w,
			 v[0]*r.x - v[3]*r.z - v[4]*r.w,
			 v[1]*r.x + v[3]*r.y - v[5]*r.w,
			 v[2]*r.x + v[4]*r.y + v[5]*r.z
		);
	}
};

// ============================================================================
// Basis4 - 4x4 rotation/scale matrix (column-major)
// m[col][row] - consistent with Godot convention
// ============================================================================
struct Basis4 {
	float m[4][4];

	Basis4() { set_identity(); }

	void set_identity() {
		memset(m, 0, sizeof(m));
		m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
	}

	void set_zero() { memset(m, 0, sizeof(m)); }

	static Basis4 identity() { return Basis4(); }

	Vector4 get_column(int col) const {
		return Vector4(m[col][0], m[col][1], m[col][2], m[col][3]);
	}
	void set_column(int col, const Vector4 &v) {
		m[col][0] = v.x; m[col][1] = v.y; m[col][2] = v.z; m[col][3] = v.w;
	}

	Vector4 get_row(int row) const {
		return Vector4(m[0][row], m[1][row], m[2][row], m[3][row]);
	}

	// Matrix-vector multiply: result = M * v
	Vector4 xform(const Vector4 &v) const {
		return Vector4(
			m[0][0]*v.x + m[1][0]*v.y + m[2][0]*v.z + m[3][0]*v.w,
			m[0][1]*v.x + m[1][1]*v.y + m[2][1]*v.z + m[3][1]*v.w,
			m[0][2]*v.x + m[1][2]*v.y + m[2][2]*v.z + m[3][2]*v.w,
			m[0][3]*v.x + m[1][3]*v.y + m[2][3]*v.z + m[3][3]*v.w
		);
	}

	// Transpose-vector multiply: result = M^T * v
	Vector4 xform_inv(const Vector4 &v) const {
		return Vector4(
			m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z + m[0][3]*v.w,
			m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z + m[1][3]*v.w,
			m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z + m[2][3]*v.w,
			m[3][0]*v.x + m[3][1]*v.y + m[3][2]*v.z + m[3][3]*v.w
		);
	}

	// Matrix-matrix multiply
	Basis4 operator*(const Basis4 &b) const {
		Basis4 r;
		r.set_zero();
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				for (int k = 0; k < 4; k++) {
					r.m[col][row] += m[k][row] * b.m[col][k];
				}
			}
		}
		return r;
	}

	Basis4 transposed() const {
		Basis4 r;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				r.m[i][j] = m[j][i];
		return r;
	}

	// Determinant via Laplace expansion along first column
	float determinant() const {
		float det = 0;
		for (int i = 0; i < 4; i++) {
			float minor = _minor3x3(0, i);
			det += (i % 2 == 0 ? 1.0f : -1.0f) * m[0][i] * minor;
		}
		return det;
	}

	// Inverse via adjugate/determinant
	Basis4 inverse() const {
		float det = determinant();
		if (fabsf(det) < PHYSICS_EPSILON) return identity();
		float inv_det = 1.0f / det;

		Basis4 adj;
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				float minor = _minor3x3(col, row);
				adj.m[row][col] = ((col + row) % 2 == 0 ? 1.0f : -1.0f) * minor * inv_det;
			}
		}
		return adj;
	}

	// Build rotation matrix for a given plane (0-5: XY,XZ,XW,YZ,YW,ZW)
	static Basis4 rotation(int plane, float angle) {
		Basis4 r;
		float c = cosf(angle), s = sinf(angle);
		// Indices of the two axes involved in this rotation plane
		static const int axis_a[6] = {0, 0, 0, 1, 1, 2};
		static const int axis_b[6] = {1, 2, 3, 2, 3, 3};
		int a = axis_a[plane], b = axis_b[plane];
		r.m[a][a] = c;  r.m[b][a] = -s;
		r.m[a][b] = s;  r.m[b][b] = c;
		return r;
	}

	// Apply bivector angular velocity to update the basis (for rotation integration)
	// dR/dt = W * R where W is the antisymmetric matrix of the bivector
	Basis4 rotated_by_bivector(const Bivec4 &bv, float dt) const {
		// Build the antisymmetric matrix W * dt
		Basis4 W;
		W.set_zero();
		W.m[1][0] = -bv.xy() * dt; W.m[0][1] =  bv.xy() * dt;
		W.m[2][0] = -bv.xz() * dt; W.m[0][2] =  bv.xz() * dt;
		W.m[3][0] = -bv.xw() * dt; W.m[0][3] =  bv.xw() * dt;
		W.m[2][1] = -bv.yz() * dt; W.m[1][2] =  bv.yz() * dt;
		W.m[3][1] = -bv.yw() * dt; W.m[1][3] =  bv.yw() * dt;
		W.m[3][2] = -bv.zw() * dt; W.m[2][3] =  bv.zw() * dt;

		// R' = R + W * R (first-order approximation)
		Basis4 result = *this;
		Basis4 delta = W * (*this);
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				result.m[i][j] += delta.m[i][j];

		// Re-orthonormalize using Gram-Schmidt
		_orthonormalize(result);
		return result;
	}

	// Scale by diagonal
	Basis4 scaled(const Vector4 &s) const {
		Basis4 r = *this;
		for (int row = 0; row < 4; row++) {
			r.m[0][row] *= s.x;
			r.m[1][row] *= s.y;
			r.m[2][row] *= s.z;
			r.m[3][row] *= s.w;
		}
		return r;
	}

	Vector4 get_scale() const {
		return Vector4(
			get_column(0).length(),
			get_column(1).length(),
			get_column(2).length(),
			get_column(3).length()
		);
	}

private:
	// 3x3 minor for cofactor expansion (skip column skip_col, row skip_row)
	float _minor3x3(int skip_col, int skip_row) const {
		float sub[3][3];
		int ci = 0;
		for (int col = 0; col < 4; col++) {
			if (col == skip_col) continue;
			int ri = 0;
			for (int row = 0; row < 4; row++) {
				if (row == skip_row) continue;
				sub[ci][ri] = m[col][row];
				ri++;
			}
			ci++;
		}
		return sub[0][0] * (sub[1][1]*sub[2][2] - sub[1][2]*sub[2][1])
			 - sub[1][0] * (sub[0][1]*sub[2][2] - sub[0][2]*sub[2][1])
			 + sub[2][0] * (sub[0][1]*sub[1][2] - sub[0][2]*sub[1][1]);
	}

	static void _orthonormalize(Basis4 &b) {
		for (int i = 0; i < 4; i++) {
			Vector4 col = b.get_column(i);
			for (int j = 0; j < i; j++) {
				Vector4 prev = b.get_column(j);
				col -= prev * col.dot(prev);
			}
			float len = col.length();
			if (len > PHYSICS_EPSILON) {
				col /= len;
			}
			b.set_column(i, col);
		}
	}
};

// ============================================================================
// Xform4 - 4D affine transform (Basis4 + Vector4 origin)
// ============================================================================
struct Xform4 {
	Basis4 basis;
	Vector4 origin;

	Xform4() : origin(0, 0, 0, 0) {}
	Xform4(const Basis4 &b, const Vector4 &o) : basis(b), origin(o) {}

	static Xform4 identity() { return Xform4(); }

	// Transform a point: M * p + origin
	Vector4 xform(const Vector4 &p) const {
		return basis.xform(p) + origin;
	}

	// Inverse transform a point (assumes orthonormal basis)
	Vector4 xform_inv(const Vector4 &p) const {
		Vector4 d = p - origin;
		return basis.xform_inv(d);
	}

	// Transform a direction (no translation)
	Vector4 xform_direction(const Vector4 &d) const {
		return basis.xform(d);
	}

	// Inverse transform a direction
	Vector4 xform_inv_direction(const Vector4 &d) const {
		return basis.xform_inv(d);
	}

	// Compose transforms: this * other
	Xform4 operator*(const Xform4 &other) const {
		return Xform4(basis * other.basis, xform(other.origin));
	}

	// Affine inverse (assumes orthonormal basis)
	Xform4 affine_inverse() const {
		Basis4 inv_b = basis.transposed();
		return Xform4(inv_b, inv_b.xform(-origin));
	}

	// General inverse
	Xform4 inverse() const {
		Basis4 inv_b = basis.inverse();
		return Xform4(inv_b, inv_b.xform(-origin));
	}

	// Convert to/from PackedFloat32Array (20 floats: 16 basis + 4 origin)
	void to_packed(float out[20]) const {
		for (int col = 0; col < 4; col++) {
			out[col*4+0] = basis.m[col][0];
			out[col*4+1] = basis.m[col][1];
			out[col*4+2] = basis.m[col][2];
			out[col*4+3] = basis.m[col][3];
		}
		out[16] = origin.x; out[17] = origin.y;
		out[18] = origin.z; out[19] = origin.w;
	}

	static Xform4 from_packed(const float in[20]) {
		Xform4 xf;
		for (int col = 0; col < 4; col++) {
			xf.basis.m[col][0] = in[col*4+0];
			xf.basis.m[col][1] = in[col*4+1];
			xf.basis.m[col][2] = in[col*4+2];
			xf.basis.m[col][3] = in[col*4+3];
		}
		xf.origin = Vector4(in[16], in[17], in[18], in[19]);
		return xf;
	}
};

// ============================================================================
// AABB4 - 4D axis-aligned bounding box
// ============================================================================
struct AABB4 {
	Vector4 min_bound;
	Vector4 max_bound;

	AABB4() : min_bound(0,0,0,0), max_bound(0,0,0,0) {}
	AABB4(const Vector4 &mn, const Vector4 &mx) : min_bound(mn), max_bound(mx) {}

	// Create from center + half-extents
	static AABB4 from_center_extents(const Vector4 &center, const Vector4 &half) {
		return AABB4(center - half, center + half);
	}

	Vector4 get_center() const { return (min_bound + max_bound) * 0.5f; }
	Vector4 get_size() const { return max_bound - min_bound; }
	Vector4 get_half_extents() const { return (max_bound - min_bound) * 0.5f; }

	bool intersects(const AABB4 &other) const {
		return min_bound.x <= other.max_bound.x && max_bound.x >= other.min_bound.x &&
			   min_bound.y <= other.max_bound.y && max_bound.y >= other.min_bound.y &&
			   min_bound.z <= other.max_bound.z && max_bound.z >= other.min_bound.z &&
			   min_bound.w <= other.max_bound.w && max_bound.w >= other.min_bound.w;
	}

	bool contains_point(const Vector4 &p) const {
		return p.x >= min_bound.x && p.x <= max_bound.x &&
			   p.y >= min_bound.y && p.y <= max_bound.y &&
			   p.z >= min_bound.z && p.z <= max_bound.z &&
			   p.w >= min_bound.w && p.w <= max_bound.w;
	}

	bool encloses(const AABB4 &other) const {
		return min_bound.x <= other.min_bound.x && max_bound.x >= other.max_bound.x &&
			   min_bound.y <= other.min_bound.y && max_bound.y >= other.max_bound.y &&
			   min_bound.z <= other.min_bound.z && max_bound.z >= other.max_bound.z &&
			   min_bound.w <= other.min_bound.w && max_bound.w >= other.max_bound.w;
	}

	AABB4 merged(const AABB4 &other) const {
		return AABB4(
			Vector4(fminf(min_bound.x, other.min_bound.x), fminf(min_bound.y, other.min_bound.y),
					fminf(min_bound.z, other.min_bound.z), fminf(min_bound.w, other.min_bound.w)),
			Vector4(fmaxf(max_bound.x, other.max_bound.x), fmaxf(max_bound.y, other.max_bound.y),
					fmaxf(max_bound.z, other.max_bound.z), fmaxf(max_bound.w, other.max_bound.w))
		);
	}

	AABB4 grown(float amount) const {
		Vector4 g(amount, amount, amount, amount);
		return AABB4(min_bound - g, max_bound + g);
	}

	AABB4 expanded(const Vector4 &point) const {
		return AABB4(
			Vector4(fminf(min_bound.x, point.x), fminf(min_bound.y, point.y),
					fminf(min_bound.z, point.z), fminf(min_bound.w, point.w)),
			Vector4(fmaxf(max_bound.x, point.x), fmaxf(max_bound.y, point.y),
					fmaxf(max_bound.z, point.z), fmaxf(max_bound.w, point.w))
		);
	}

	// Surface area heuristic for BVH (in 4D, this is the "hyper-surface area" = sum of all 2D face areas)
	// For a 4D box, there are C(4,2)=6 pairs of axes, each contributing a rectangular face
	// with area = product of the other 2 dimensions. Actually for BVH we just need a cost metric.
	// We use the sum of products of all pairs of edge lengths (proportional to surface area).
	float surface_area() const {
		Vector4 s = get_size();
		return 2.0f * (s.x*s.y + s.x*s.z + s.x*s.w + s.y*s.z + s.y*s.w + s.z*s.w);
	}

	// Ray intersection: returns true if ray hits, t_min/t_max set to intersection range
	bool intersect_ray(const Vector4 &ray_origin, const Vector4 &ray_dir, float &t_min, float &t_max) const {
		t_min = 0.0f;
		t_max = 1e30f;
		const float *orig = &ray_origin.x;
		const float *dir = &ray_dir.x;
		const float *bmin = &min_bound.x;
		const float *bmax = &max_bound.x;
		for (int i = 0; i < 4; i++) {
			if (fabsf(dir[i]) < PHYSICS_EPSILON) {
				if (orig[i] < bmin[i] || orig[i] > bmax[i]) return false;
			} else {
				float inv_d = 1.0f / dir[i];
				float t0 = (bmin[i] - orig[i]) * inv_d;
				float t1 = (bmax[i] - orig[i]) * inv_d;
				if (t0 > t1) std::swap(t0, t1);
				t_min = fmaxf(t_min, t0);
				t_max = fminf(t_max, t1);
				if (t_min > t_max) return false;
			}
		}
		return true;
	}
};

// ============================================================================
// Inertia4 - 6x6 inertia tensor for 4D rigid body
// Maps bivector angular velocities to angular momenta
// For axis-aligned shapes, this is diagonal.
// ============================================================================
struct Inertia4 {
	float m[6][6];

	Inertia4() { set_zero(); }

	void set_zero() { memset(m, 0, sizeof(m)); }

	// Diagonal inertia (most common for basic shapes)
	static Inertia4 diagonal(float i0, float i1, float i2, float i3, float i4, float i5) {
		Inertia4 r;
		r.m[0][0] = i0; r.m[1][1] = i1; r.m[2][2] = i2;
		r.m[3][3] = i3; r.m[4][4] = i4; r.m[5][5] = i5;
		return r;
	}

	// Uniform inertia (sphere)
	static Inertia4 uniform(float i_val) {
		return diagonal(i_val, i_val, i_val, i_val, i_val, i_val);
	}

	// Apply: angular_momentum = I * angular_velocity
	Bivec4 xform(const Bivec4 &omega) const {
		Bivec4 result;
		for (int i = 0; i < 6; i++) {
			result.v[i] = 0;
			for (int j = 0; j < 6; j++) {
				result.v[i] += m[i][j] * omega.v[j];
			}
		}
		return result;
	}

	// Inverse (for diagonal case, just reciprocal)
	Inertia4 inverse() const {
		// General 6x6 inverse - for simplicity, use diagonal approximation
		// Full physics shapes produce diagonal inertia tensors in local space
		Inertia4 r;
		for (int i = 0; i < 6; i++) {
			if (fabsf(m[i][i]) > PHYSICS_EPSILON) {
				r.m[i][i] = 1.0f / m[i][i];
			}
		}
		return r;
	}

	// Rotate inertia tensor by basis: I' = R * I * R^T
	// For a diagonal tensor and orthonormal R, this produces the world-space tensor.
	// The rotation acts on bivector space: a bivector b_ij transforms as b'_kl = R_ki R_lj b_ij
	// We need the 6x6 rotation matrix that acts on the bivector components.
	Inertia4 rotated(const Basis4 &R) const {
		// Build the 6x6 bivector rotation matrix
		// Bivector component ordering: XY=0, XZ=1, XW=2, YZ=3, YW=4, ZW=5
		static const int axis_a[6] = {0, 0, 0, 1, 1, 2};
		static const int axis_b[6] = {1, 2, 3, 2, 3, 3};

		float bv_rot[6][6];
		for (int i = 0; i < 6; i++) {
			int a = axis_a[i], b = axis_b[i];
			for (int j = 0; j < 6; j++) {
				int c = axis_a[j], d = axis_b[j];
				// R_ac * R_bd - R_ad * R_bc
				bv_rot[i][j] = R.m[c][a] * R.m[d][b] - R.m[d][a] * R.m[c][b];
			}
		}

		// I' = bv_rot * I * bv_rot^T
		float temp[6][6] = {};
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 6; j++)
				for (int k = 0; k < 6; k++)
					temp[i][j] += bv_rot[i][k] * m[k][j];

		Inertia4 result;
		for (int i = 0; i < 6; i++)
			for (int j = 0; j < 6; j++)
				for (int k = 0; k < 6; k++)
					result.m[i][j] += temp[i][k] * bv_rot[j][k]; // bv_rot^T
		return result;
	}
};

// ============================================================================
// Utility: Triple cross product in 4D
// Given 3 vectors u, v, w in 4D, returns the vector perpendicular to all three.
// Computed as the "4D cross product" via cofactor expansion:
// | e_x  e_y  e_z  e_w |
// | u_x  u_y  u_z  u_w |
// | v_x  v_y  v_z  v_w |
// | w_x  w_y  w_z  w_w |
// ============================================================================
inline Vector4 triple_cross_4d(const Vector4 &u, const Vector4 &v, const Vector4 &w) {
	float nx = +(u.y * (v.z*w.w - v.w*w.z) - u.z * (v.y*w.w - v.w*w.y) + u.w * (v.y*w.z - v.z*w.y));
	float ny = -(u.x * (v.z*w.w - v.w*w.z) - u.z * (v.x*w.w - v.w*w.x) + u.w * (v.x*w.z - v.z*w.x));
	float nz = +(u.x * (v.y*w.w - v.w*w.y) - u.y * (v.x*w.w - v.w*w.x) + u.w * (v.x*w.y - v.y*w.x));
	float nw = -(u.x * (v.y*w.z - v.z*w.y) - u.y * (v.x*w.z - v.z*w.x) + u.z * (v.x*w.y - v.y*w.x));
	return Vector4(nx, ny, nz, nw);
}

// Helper: component-wise min/max for Vector4
inline Vector4 vec4_min(const Vector4 &a, const Vector4 &b) {
	return Vector4(fminf(a.x, b.x), fminf(a.y, b.y), fminf(a.z, b.z), fminf(a.w, b.w));
}
inline Vector4 vec4_max(const Vector4 &a, const Vector4 &b) {
	return Vector4(fmaxf(a.x, b.x), fmaxf(a.y, b.y), fmaxf(a.z, b.z), fmaxf(a.w, b.w));
}
inline Vector4 vec4_abs(const Vector4 &v) {
	return Vector4(fabsf(v.x), fabsf(v.y), fabsf(v.z), fabsf(v.w));
}
inline float vec4_max_component(const Vector4 &v) {
	return fmaxf(fmaxf(v.x, v.y), fmaxf(v.z, v.w));
}
