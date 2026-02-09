#include "transform4d.h"

// -- Constructors ------------------------------------------------------------

Transform4D::Transform4D() :
		origin(Vector4(0, 0, 0, 0)) {}

Transform4D::Transform4D(const Basis4D &p_basis, const Vector4 &p_origin) :
		basis(p_basis), origin(p_origin) {}

// -- Transform points and directions -----------------------------------------

Vector4 Transform4D::xform(const Vector4 &p_point) const {
	return basis.xform(p_point) + origin;
}

Vector4 Transform4D::xform_inv(const Vector4 &p_point) const {
	Vector4 shifted = p_point - origin;
	return basis.xform_inv(shifted);
}

Vector4 Transform4D::basis_xform(const Vector4 &p_dir) const {
	return basis.xform(p_dir);
}

// -- Composition -------------------------------------------------------------

Transform4D Transform4D::operator*(const Transform4D &p_other) const {
	Transform4D result;
	result.basis = basis * p_other.basis;
	result.origin = xform(p_other.origin);
	return result;
}

// -- Inverse -----------------------------------------------------------------

Transform4D Transform4D::inverse() const {
	Basis4D inv_basis = basis.inverse();
	return Transform4D(inv_basis, inv_basis.xform(-origin));
}

Transform4D Transform4D::affine_inverse() const {
	return inverse();
}

// -- Builders ----------------------------------------------------------------

Transform4D Transform4D::translated(const Vector4 &p_offset) {
	Transform4D t;
	t.origin = p_offset;
	return t;
}

Transform4D Transform4D::rotated(const Basis4D &p_rotation) const {
	Transform4D t = *this;
	t.basis = p_rotation * basis;
	t.origin = p_rotation.xform(origin);
	return t;
}

Transform4D Transform4D::scaled(const Vector4 &p_scale) const {
	Transform4D t = *this;
	t.basis = Basis4D::from_scale(p_scale) * basis;
	return t;
}
