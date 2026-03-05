#include "bivector4d.h"
#include "vector4d.h"
#include <godot_cpp/core/math.hpp>
#include <cmath>

using namespace godot;

void Bivector4D::_bind_methods() {
	ClassDB::bind_static_method("Bivector4D", D_METHOD("create", "xy", "xz", "xw", "yz", "yw", "zw"), &Bivector4D::create);
	ClassDB::bind_static_method("Bivector4D", D_METHOD("zero_bv"), &Bivector4D::zero_bv);
	ClassDB::bind_method(D_METHOD("added", "other"), &Bivector4D::added);
	ClassDB::bind_method(D_METHOD("subtracted", "other"), &Bivector4D::subtracted);
	ClassDB::bind_method(D_METHOD("scaled", "scalar"), &Bivector4D::scaled);
	ClassDB::bind_method(D_METHOD("negated"), &Bivector4D::negated);
	ClassDB::bind_method(D_METHOD("length"), &Bivector4D::length);
	ClassDB::bind_method(D_METHOD("length_squared"), &Bivector4D::length_squared);
	ClassDB::bind_method(D_METHOD("is_zero_approx"), &Bivector4D::is_zero_approx);
	ClassDB::bind_static_method("Bivector4D", D_METHOD("wedge", "r", "f"), &Bivector4D::wedge);
	ClassDB::bind_method(D_METHOD("to_matrix"), &Bivector4D::to_matrix);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xy"), "set_xy", "get_xy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xz"), "set_xz", "get_xz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "xw"), "set_xw", "get_xw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "yz"), "set_yz", "get_yz");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "yw"), "set_yw", "get_yw");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "zw"), "set_zw", "get_zw");

	ClassDB::bind_method(D_METHOD("get_xy"), &Bivector4D::get_xy);
	ClassDB::bind_method(D_METHOD("set_xy", "v"), &Bivector4D::set_xy);
	ClassDB::bind_method(D_METHOD("get_xz"), &Bivector4D::get_xz);
	ClassDB::bind_method(D_METHOD("set_xz", "v"), &Bivector4D::set_xz);
	ClassDB::bind_method(D_METHOD("get_xw"), &Bivector4D::get_xw);
	ClassDB::bind_method(D_METHOD("set_xw", "v"), &Bivector4D::set_xw);
	ClassDB::bind_method(D_METHOD("get_yz"), &Bivector4D::get_yz);
	ClassDB::bind_method(D_METHOD("set_yz", "v"), &Bivector4D::set_yz);
	ClassDB::bind_method(D_METHOD("get_yw"), &Bivector4D::get_yw);
	ClassDB::bind_method(D_METHOD("set_yw", "v"), &Bivector4D::set_yw);
	ClassDB::bind_method(D_METHOD("get_zw"), &Bivector4D::get_zw);
	ClassDB::bind_method(D_METHOD("set_zw", "v"), &Bivector4D::set_zw);
}

Ref<Bivector4D> Bivector4D::create(real_t p_xy, real_t p_xz, real_t p_xw, real_t p_yz, real_t p_yw, real_t p_zw) {
	Ref<Bivector4D> b;
	b.instantiate();
	b->xy = p_xy; b->xz = p_xz; b->xw = p_xw;
	b->yz = p_yz; b->yw = p_yw; b->zw = p_zw;
	return b;
}

Ref<Bivector4D> Bivector4D::zero_bv() {
	Ref<Bivector4D> b;
	b.instantiate();
	return b;
}

Ref<Bivector4D> Bivector4D::added(Ref<Bivector4D> p_other) const {
	return create(xy + p_other->xy, xz + p_other->xz, xw + p_other->xw,
		yz + p_other->yz, yw + p_other->yw, zw + p_other->zw);
}

Ref<Bivector4D> Bivector4D::subtracted(Ref<Bivector4D> p_other) const {
	return create(xy - p_other->xy, xz - p_other->xz, xw - p_other->xw,
		yz - p_other->yz, yw - p_other->yw, zw - p_other->zw);
}

Ref<Bivector4D> Bivector4D::scaled(real_t p_scalar) const {
	return create(xy * p_scalar, xz * p_scalar, xw * p_scalar,
		yz * p_scalar, yw * p_scalar, zw * p_scalar);
}

Ref<Bivector4D> Bivector4D::negated() const {
	return create(-xy, -xz, -xw, -yz, -yw, -zw);
}

real_t Bivector4D::length_squared() const {
	return xy * xy + xz * xz + xw * xw + yz * yz + yw * yw + zw * zw;
}

real_t Bivector4D::length() const {
	return sqrtf(length_squared());
}

bool Bivector4D::is_zero_approx() const {
	return Math::is_zero_approx(xy) && Math::is_zero_approx(xz) && Math::is_zero_approx(xw) &&
		Math::is_zero_approx(yz) && Math::is_zero_approx(yw) && Math::is_zero_approx(zw);
}

// Wedge product r ^ F: produces a bivector from two 4D vectors.
// (r ^ F)_ij = r_i * F_j - r_j * F_i for i < j
// Plane index convention: XY=01, XZ=02, XW=03, YZ=12, YW=13, ZW=23
Ref<Bivector4D> Bivector4D::wedge(Ref<Vector4D> p_r, Ref<Vector4D> p_f) {
	return create(
		p_r->x * p_f->y - p_r->y * p_f->x, // XY
		p_r->x * p_f->z - p_r->z * p_f->x, // XZ
		p_r->x * p_f->w - p_r->w * p_f->x, // XW
		p_r->y * p_f->z - p_r->z * p_f->y, // YZ
		p_r->y * p_f->w - p_r->w * p_f->y, // YW
		p_r->z * p_f->w - p_r->w * p_f->z  // ZW
	);
}

// Returns the antisymmetric 4x4 matrix W from the bivector.
// W_ij for (i,j) pairs: (0,1)=xy, (0,2)=xz, (0,3)=xw, (1,2)=yz, (1,3)=yw, (2,3)=zw
// Stored row-major: W[row*4+col]
PackedFloat32Array Bivector4D::to_matrix() const {
	PackedFloat32Array m;
	m.resize(16);
	// Row 0: [0, xy, xz, xw]
	m[0] = 0.0f;  m[1] = xy;   m[2] = xz;  m[3] = xw;
	// Row 1: [-xy, 0, yz, yw]
	m[4] = -xy;   m[5] = 0.0f; m[6] = yz;  m[7] = yw;
	// Row 2: [-xz, -yz, 0, zw]
	m[8] = -xz;   m[9] = -yz;  m[10] = 0.0f; m[11] = zw;
	// Row 3: [-xw, -yw, -zw, 0]
	m[12] = -xw;  m[13] = -yw; m[14] = -zw;  m[15] = 0.0f;
	return m;
}

String Bivector4D::_to_string() const {
	return vformat("Bivector4D(xy=%g, xz=%g, xw=%g, yz=%g, yw=%g, zw=%g)",
		xy, xz, xw, yz, yw, zw);
}
