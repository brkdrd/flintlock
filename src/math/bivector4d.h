#pragma once
#include "vector4d.h"
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Bivector4D: 6-component angular velocity/torque for 4D physics.
// Components correspond to the 6 rotation planes: XY, XZ, XW, YZ, YW, ZW.
// This is the 4D analog of Vector3 angular velocity.
class Bivector4D : public RefCounted {
	GDCLASS(Bivector4D, RefCounted);

public:
	real_t xy = 0.0f, xz = 0.0f, xw = 0.0f;
	real_t yz = 0.0f, yw = 0.0f, zw = 0.0f;

	static Ref<Bivector4D> create(real_t p_xy, real_t p_xz, real_t p_xw, real_t p_yz, real_t p_yw, real_t p_zw);
	static Ref<Bivector4D> zero_bv();

	Ref<Bivector4D> added(Ref<Bivector4D> p_other) const;
	Ref<Bivector4D> subtracted(Ref<Bivector4D> p_other) const;
	Ref<Bivector4D> scaled(real_t p_scalar) const;
	Ref<Bivector4D> negated() const;

	real_t length() const;
	real_t length_squared() const;
	bool is_zero_approx() const;

	// Wedge product of two 4D vectors: r ^ F -> bivector (angular torque from force)
	static Ref<Bivector4D> wedge(Ref<Vector4D> p_r, Ref<Vector4D> p_f);

	// Convert to antisymmetric 4x4 matrix representation (for rotation integration).
	// Returns 16 floats, row-major: W_ij where W_ij = -W_ji.
	// Used as: v_i = sum_j(W_ij * r_j)
	PackedFloat32Array to_matrix() const;

	// Component accessors
	real_t get_xy() const { return xy; }
	real_t get_xz() const { return xz; }
	real_t get_xw() const { return xw; }
	real_t get_yz() const { return yz; }
	real_t get_yw() const { return yw; }
	real_t get_zw() const { return zw; }
	void set_xy(real_t v) { xy = v; }
	void set_xz(real_t v) { xz = v; }
	void set_xw(real_t v) { xw = v; }
	void set_yz(real_t v) { yz = v; }
	void set_yw(real_t v) { yw = v; }
	void set_zw(real_t v) { zw = v; }

	String _to_string() const;

protected:
	static void _bind_methods();
};
