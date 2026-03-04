#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>

namespace godot {

// 4D vector - NOT Godot's built-in Vector4 (which is for homogeneous coords).
// This is a true 4D spatial vector with full math API.
class Vector4D : public RefCounted {
	GDCLASS(Vector4D, RefCounted);

public:
	real_t x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

	// Factory / constructors
	static Ref<Vector4D> create(real_t p_x, real_t p_y, real_t p_z, real_t p_w);
	static Ref<Vector4D> from_vector4(const Vector4 &p_v);

	// GDScript constructors
	void set_xyzw(real_t p_x, real_t p_y, real_t p_z, real_t p_w);

	// Component access
	real_t get_x() const { return x; }
	real_t get_y() const { return y; }
	real_t get_z() const { return z; }
	real_t get_w() const { return w; }
	void set_x(real_t v) { x = v; }
	void set_y(real_t v) { y = v; }
	void set_z(real_t v) { z = v; }
	void set_w(real_t v) { w = v; }
	real_t get_axis(int p_axis) const;
	void set_axis(int p_axis, real_t p_value);

	// Math
	Ref<Vector4D> added(const Ref<Vector4D> &p_b) const;
	Ref<Vector4D> subtracted(const Ref<Vector4D> &p_b) const;
	Ref<Vector4D> multiplied_scalar(real_t p_s) const;
	Ref<Vector4D> divided_scalar(real_t p_s) const;
	Ref<Vector4D> negated() const;

	real_t dot(const Ref<Vector4D> &p_b) const;
	real_t length() const;
	real_t length_squared() const;
	Ref<Vector4D> normalized() const;
	bool is_normalized() const;
	real_t distance_to(const Ref<Vector4D> &p_to) const;
	real_t distance_squared_to(const Ref<Vector4D> &p_to) const;

	Ref<Vector4D> lerp(const Ref<Vector4D> &p_to, real_t p_weight) const;
	Ref<Vector4D> slerp(const Ref<Vector4D> &p_to, real_t p_weight) const;

	Ref<Vector4D> abs() const;
	Ref<Vector4D> sign() const;
	Ref<Vector4D> floor() const;
	Ref<Vector4D> ceil() const;
	Ref<Vector4D> round() const;
	Ref<Vector4D> clamp(const Ref<Vector4D> &p_min, const Ref<Vector4D> &p_max) const;

	int min_axis_index() const;
	int max_axis_index() const;

	bool is_equal_approx(const Ref<Vector4D> &p_b) const;
	bool is_zero_approx() const;

	// Interop
	Vector4 to_vector4() const;

	// String representation
	String to_string() const;

	// Internal (C++ only) helpers
	static Ref<Vector4D> _make(real_t x, real_t y, real_t z, real_t w);

protected:
	static void _bind_methods();
};

} // namespace godot
