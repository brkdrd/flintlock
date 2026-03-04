#include "transform4d.h"
#include <godot_cpp/core/class_db.hpp>

namespace godot {

Transform4D::Transform4D() {
	basis.instantiate();
	origin.instantiate();
}

Ref<Transform4D> Transform4D::create_identity() {
	Ref<Transform4D> t;
	t.instantiate();
	return t;
}

Ref<Transform4D> Transform4D::create(const Ref<Basis4D> &p_basis, const Ref<Vector4D> &p_origin) {
	Ref<Transform4D> t;
	t.instantiate();
	t->basis  = p_basis;
	t->origin = p_origin;
	return t;
}

Ref<Transform4D> Transform4D::multiplied(const Ref<Transform4D> &p_b) const {
	// result.basis = this.basis * p_b.basis
	// result.origin = this.basis.xform(p_b.origin) + this.origin
	Ref<Transform4D> result;
	result.instantiate();
	result->basis  = basis->multiplied(p_b->basis);
	result->origin = basis->xform(p_b->origin)->added(origin);
	return result;
}

Ref<Vector4D> Transform4D::xform(const Ref<Vector4D> &p_v) const {
	return basis->xform(p_v)->added(origin);
}

Ref<Vector4D> Transform4D::xform_inv(const Ref<Vector4D> &p_v) const {
	Ref<Vector4D> diff = p_v->subtracted(origin);
	return basis->xform_inv(diff);
}

Ref<Transform4D> Transform4D::affine_inverse() const {
	Ref<Basis4D> inv_basis = basis->inverse();
	Ref<Vector4D> inv_origin = inv_basis->xform(origin)->negated();
	return create(inv_basis, inv_origin);
}

Ref<Transform4D> Transform4D::interpolate_with(const Ref<Transform4D> &p_to, real_t t) const {
	Ref<Transform4D> result;
	result.instantiate();
	result->origin = origin->lerp(p_to->origin, t);
	// Simple basis lerp (not SLERP - good enough for small angles)
	Ref<Basis4D> lb;
	lb.instantiate();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			lb->data[c][r] = basis->data[c][r] + (p_to->basis->data[c][r] - basis->data[c][r]) * t;
	lb->orthonormalize();
	result->basis = lb;
	return result;
}

bool Transform4D::is_equal_approx(const Ref<Transform4D> &p_b) const {
	return basis->is_equal_approx(p_b->basis) && origin->is_equal_approx(p_b->origin);
}

String Transform4D::to_string() const {
	return String("[Transform4D]");
}

void Transform4D::_bind_methods() {
	ClassDB::bind_static_method("Transform4D", D_METHOD("create_identity"), &Transform4D::create_identity);
	ClassDB::bind_static_method("Transform4D", D_METHOD("create", "basis", "origin"), &Transform4D::create);

	ClassDB::bind_method(D_METHOD("get_basis"), &Transform4D::get_basis);
	ClassDB::bind_method(D_METHOD("set_basis", "b"), &Transform4D::set_basis);
	ClassDB::bind_method(D_METHOD("get_origin"), &Transform4D::get_origin);
	ClassDB::bind_method(D_METHOD("set_origin", "o"), &Transform4D::set_origin);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "basis",  PROPERTY_HINT_RESOURCE_TYPE, "Basis4D"),  "set_basis",  "get_basis");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "origin", PROPERTY_HINT_RESOURCE_TYPE, "Vector4D"), "set_origin", "get_origin");

	ClassDB::bind_method(D_METHOD("multiplied", "b"), &Transform4D::multiplied);
	ClassDB::bind_method(D_METHOD("xform", "v"), &Transform4D::xform);
	ClassDB::bind_method(D_METHOD("xform_inv", "v"), &Transform4D::xform_inv);
	ClassDB::bind_method(D_METHOD("affine_inverse"), &Transform4D::affine_inverse);
	ClassDB::bind_method(D_METHOD("interpolate_with", "to", "weight"), &Transform4D::interpolate_with);
	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &Transform4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("to_string"), &Transform4D::to_string);
}

} // namespace godot
