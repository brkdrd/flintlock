#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "vector4d.h"

// Forward declare Eigen matrix to keep it out of the header
namespace Eigen { template<typename S, int R, int C, int O, int MR, int MC> class Matrix; }

namespace godot {

// Rotation planes in 4D: 6 independent bi-vector planes.
// This enum is exposed to GDScript.
enum PlaneAxis {
	PLANE_XY = 0,
	PLANE_XZ = 1,
	PLANE_XW = 2,
	PLANE_YZ = 3,
	PLANE_YW = 4,
	PLANE_ZW = 5,
};

// 4x4 matrix representing rotation and/or scale in 4D.
// Analog of Godot's Basis (3x3 matrix).
class Basis4D : public RefCounted {
	GDCLASS(Basis4D, RefCounted);

public:
	// Stored column-major: columns[0..3] are the 4 basis column vectors.
	// col[i][j] = element at row j, column i.
	real_t data[4][4] = {
		{1, 0, 0, 0},
		{0, 1, 0, 0},
		{0, 0, 1, 0},
		{0, 0, 0, 1}
	};

	// Constructors
	static Ref<Basis4D> create_identity();
	static Ref<Basis4D> from_columns(
		const Ref<Vector4D> &c0,
		const Ref<Vector4D> &c1,
		const Ref<Vector4D> &c2,
		const Ref<Vector4D> &c3
	);
	static Ref<Basis4D> from_rotation(int p_plane, real_t p_angle);

	// Column access
	Ref<Vector4D> get_column(int p_col) const;
	void set_column(int p_col, const Ref<Vector4D> &p_vec);

	// Row access
	Ref<Vector4D> get_row(int p_row) const;

	// Rotation
	Ref<Basis4D> rotated(int p_plane, real_t p_angle) const;
	void rotate(int p_plane, real_t p_angle);

	// Scale
	Ref<Basis4D> scaled(const Ref<Vector4D> &p_scale) const;
	Ref<Vector4D> get_scale() const;

	// Orthonormalization
	Ref<Basis4D> orthonormalized() const;
	void orthonormalize();

	// Linear algebra
	real_t determinant() const;
	Ref<Basis4D> inverse() const;
	Ref<Basis4D> transposed() const;

	// Matrix multiplication
	Ref<Basis4D> multiplied(const Ref<Basis4D> &p_b) const;
	Ref<Vector4D> xform(const Ref<Vector4D> &p_v) const;
	Ref<Vector4D> xform_inv(const Ref<Vector4D> &p_v) const;

	bool is_equal_approx(const Ref<Basis4D> &p_b) const;

	String to_string() const;

protected:
	static void _bind_methods();

private:
	// Internal Eigen helpers (implemented in .cpp, not exposed in header)
	void _to_eigen(float out[16]) const;
	void _from_eigen(const float in[16]);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::PlaneAxis);
