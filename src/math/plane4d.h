#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "vector4d.h"

namespace godot {

// A 3D hyperplane embedded in 4D space.
// Defined by: normal (unit Vector4D) and d (signed distance from origin).
// A point P is on the hyperplane iff normal.dot(P) == d.
// Analog of Godot's Plane, but in 4D.
class Plane4D : public RefCounted {
	GDCLASS(Plane4D, RefCounted);

public:
	Ref<Vector4D> normal;
	real_t d = 0.0f;

	Plane4D();

	static Ref<Plane4D> create(const Ref<Vector4D> &p_normal, real_t p_d);
	static Ref<Plane4D> from_point_normal(const Ref<Vector4D> &p_point, const Ref<Vector4D> &p_normal);

	Ref<Vector4D> get_normal() const { return normal; }
	real_t        get_d()      const { return d; }
	void set_normal(const Ref<Vector4D> &n) { normal = n; }
	void set_d(real_t v) { d = v; }

	// Property adapter using built-in Vector4 (avoids ClassDB default-value warning)
	Vector4 get_normal_v4() const { return Vector4(normal->x, normal->y, normal->z, normal->w); }
	void set_normal_v4(const Vector4 &n) { normal->x = n.x; normal->y = n.y; normal->z = n.z; normal->w = n.w; }

	// Signed distance from a point to the hyperplane.
	// Positive = same side as normal direction.
	real_t distance_to(const Ref<Vector4D> &p_point) const;

	bool is_point_over(const Ref<Vector4D> &p_point) const;

	// Project a point onto the hyperplane.
	Ref<Vector4D> project(const Ref<Vector4D> &p_point) const;

	// Intersect a segment [a, b] with the hyperplane.
	// Returns null if segment does not cross hyperplane.
	Ref<Vector4D> intersects_segment(const Ref<Vector4D> &p_a, const Ref<Vector4D> &p_b) const;

	// Get 3 orthogonal tangent vectors spanning the hyperplane.
	// Uses Gram-Schmidt with a stable priority-based seed axis.
	void get_tangent_basis(Ref<Vector4D> &t1, Ref<Vector4D> &t2, Ref<Vector4D> &t3) const;
	Array get_tangent_basis_array() const; // returns [t1, t2, t3]

	bool is_equal_approx(const Ref<Plane4D> &p_b) const;
	String to_string() const;

protected:
	static void _bind_methods();
};

} // namespace godot
