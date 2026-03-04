#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "basis4d.h"
#include "vector4d.h"

namespace godot {

// 4D affine transform = Basis4D (rotation/scale) + Vector4D (translation).
// Direct analog of Transform3D.
class Transform4D : public RefCounted {
	GDCLASS(Transform4D, RefCounted);

public:
	// Internal storage (C++ accessible directly)
	Ref<Basis4D>  basis;
	Ref<Vector4D> origin;

	Transform4D();

	static Ref<Transform4D> create_identity();
	static Ref<Transform4D> create(const Ref<Basis4D> &p_basis, const Ref<Vector4D> &p_origin);

	Ref<Basis4D>  get_basis()  const { return basis; }
	Ref<Vector4D> get_origin() const { return origin; }
	void set_basis(const Ref<Basis4D> &p_b)   { basis  = p_b; }
	void set_origin(const Ref<Vector4D> &p_o) { origin = p_o; }

	// Composition
	Ref<Transform4D> multiplied(const Ref<Transform4D> &p_b) const;

	// Point / vector transform
	Ref<Vector4D> xform(const Ref<Vector4D> &p_v) const;
	Ref<Vector4D> xform_inv(const Ref<Vector4D> &p_v) const;

	// Inverse (affine)
	Ref<Transform4D> affine_inverse() const;

	// Interpolation
	Ref<Transform4D> interpolate_with(const Ref<Transform4D> &p_to, real_t p_weight) const;

	bool is_equal_approx(const Ref<Transform4D> &p_b) const;

	String to_string() const;

protected:
	static void _bind_methods();
};

} // namespace godot
