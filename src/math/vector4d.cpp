#include "vector4d.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <cmath>

namespace godot {

// ─── Static factory ───────────────────────────────────────────────────────────

Ref<Vector4D> Vector4D::create(real_t p_x, real_t p_y, real_t p_z, real_t p_w) {
	return _make(p_x, p_y, p_z, p_w);
}

Ref<Vector4D> Vector4D::from_vector4(const Vector4 &p_v) {
	return _make(p_v.x, p_v.y, p_v.z, p_v.w);
}

Ref<Vector4D> Vector4D::_make(real_t px, real_t py, real_t pz, real_t pw) {
	Ref<Vector4D> v;
	v.instantiate();
	v->x = px; v->y = py; v->z = pz; v->w = pw;
	return v;
}

void Vector4D::set_xyzw(real_t p_x, real_t p_y, real_t p_z, real_t p_w) {
	x = p_x; y = p_y; z = p_z; w = p_w;
}

// ─── Component access by index ────────────────────────────────────────────────

real_t Vector4D::get_axis(int p_axis) const {
	switch (p_axis) {
		case 0: return x;
		case 1: return y;
		case 2: return z;
		case 3: return w;
		default: return 0.0f;
	}
}

void Vector4D::set_axis(int p_axis, real_t p_value) {
	switch (p_axis) {
		case 0: x = p_value; break;
		case 1: y = p_value; break;
		case 2: z = p_value; break;
		case 3: w = p_value; break;
	}
}

// ─── Arithmetic ───────────────────────────────────────────────────────────────

Ref<Vector4D> Vector4D::added(const Ref<Vector4D> &p_b) const {
	return _make(x + p_b->x, y + p_b->y, z + p_b->z, w + p_b->w);
}
Ref<Vector4D> Vector4D::subtracted(const Ref<Vector4D> &p_b) const {
	return _make(x - p_b->x, y - p_b->y, z - p_b->z, w - p_b->w);
}
Ref<Vector4D> Vector4D::multiplied_scalar(real_t p_s) const {
	return _make(x * p_s, y * p_s, z * p_s, w * p_s);
}
Ref<Vector4D> Vector4D::divided_scalar(real_t p_s) const {
	real_t inv = 1.0f / p_s;
	return _make(x * inv, y * inv, z * inv, w * inv);
}
Ref<Vector4D> Vector4D::negated() const {
	return _make(-x, -y, -z, -w);
}

// ─── Linear algebra ───────────────────────────────────────────────────────────

real_t Vector4D::dot(const Ref<Vector4D> &p_b) const {
	return x * p_b->x + y * p_b->y + z * p_b->z + w * p_b->w;
}
real_t Vector4D::length_squared() const {
	return x * x + y * y + z * z + w * w;
}
real_t Vector4D::length() const {
	return std::sqrt(length_squared());
}
Ref<Vector4D> Vector4D::normalized() const {
	real_t len = length();
	if (len < 1e-8f) return _make(0, 0, 0, 0);
	return divided_scalar(len);
}
bool Vector4D::is_normalized() const {
	return std::abs(length_squared() - 1.0f) < 1e-5f;
}
real_t Vector4D::distance_squared_to(const Ref<Vector4D> &p_to) const {
	return subtracted(p_to)->length_squared();
}
real_t Vector4D::distance_to(const Ref<Vector4D> &p_to) const {
	return std::sqrt(distance_squared_to(p_to));
}

Ref<Vector4D> Vector4D::lerp(const Ref<Vector4D> &p_to, real_t t) const {
	return _make(
		x + (p_to->x - x) * t,
		y + (p_to->y - y) * t,
		z + (p_to->z - z) * t,
		w + (p_to->w - w) * t
	);
}

Ref<Vector4D> Vector4D::slerp(const Ref<Vector4D> &p_to, real_t t) const {
	real_t dv = dot(p_to);
	dv = dv < -1.0f ? -1.0f : (dv > 1.0f ? 1.0f : dv);
	real_t theta = std::acos(dv);
	if (std::abs(theta) < 1e-8f) return lerp(p_to, t);
	real_t st = std::sin(theta);
	real_t wa = std::sin((1.0f - t) * theta) / st;
	real_t wb = std::sin(t * theta) / st;
	return _make(x * wa + p_to->x * wb, y * wa + p_to->y * wb,
				 z * wa + p_to->z * wb, w * wa + p_to->w * wb);
}

// ─── Component-wise ───────────────────────────────────────────────────────────

Ref<Vector4D> Vector4D::abs() const {
	return _make(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}
Ref<Vector4D> Vector4D::sign() const {
	auto sgn = [](real_t v) -> real_t { return v > 0 ? 1.0f : v < 0 ? -1.0f : 0.0f; };
	return _make(sgn(x), sgn(y), sgn(z), sgn(w));
}
Ref<Vector4D> Vector4D::floor() const {
	return _make(std::floor(x), std::floor(y), std::floor(z), std::floor(w));
}
Ref<Vector4D> Vector4D::ceil() const {
	return _make(std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w));
}
Ref<Vector4D> Vector4D::round() const {
	return _make(std::round(x), std::round(y), std::round(z), std::round(w));
}
Ref<Vector4D> Vector4D::clamp(const Ref<Vector4D> &p_min, const Ref<Vector4D> &p_max) const {
	auto clamp1 = [](real_t v, real_t lo, real_t hi) { return v < lo ? lo : (v > hi ? hi : v); };
	return _make(
		clamp1(x, p_min->x, p_max->x),
		clamp1(y, p_min->y, p_max->y),
		clamp1(z, p_min->z, p_max->z),
		clamp1(w, p_min->w, p_max->w)
	);
}

int Vector4D::min_axis_index() const {
	int idx = 0;
	real_t min_v = x;
	if (y < min_v) { min_v = y; idx = 1; }
	if (z < min_v) { min_v = z; idx = 2; }
	if (w < min_v) { idx = 3; }
	return idx;
}
int Vector4D::max_axis_index() const {
	int idx = 0;
	real_t max_v = x;
	if (y > max_v) { max_v = y; idx = 1; }
	if (z > max_v) { max_v = z; idx = 2; }
	if (w > max_v) { idx = 3; }
	return idx;
}

bool Vector4D::is_equal_approx(const Ref<Vector4D> &p_b) const {
	return std::abs(x - p_b->x) < 1e-5f &&
		   std::abs(y - p_b->y) < 1e-5f &&
		   std::abs(z - p_b->z) < 1e-5f &&
		   std::abs(w - p_b->w) < 1e-5f;
}
bool Vector4D::is_zero_approx() const {
	return length_squared() < 1e-10f;
}

// ─── Interop ──────────────────────────────────────────────────────────────────

Vector4 Vector4D::to_vector4() const {
	return Vector4(x, y, z, w);
}

String Vector4D::to_string() const {
	return vformat("Vector4D(%g, %g, %g, %g)", x, y, z, w);
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void Vector4D::_bind_methods() {
	ClassDB::bind_static_method("Vector4D", D_METHOD("create", "x", "y", "z", "w"), &Vector4D::create);
	ClassDB::bind_static_method("Vector4D", D_METHOD("from_vector4", "v"), &Vector4D::from_vector4);

	ClassDB::bind_method(D_METHOD("set_xyzw", "x", "y", "z", "w"), &Vector4D::set_xyzw);

	ClassDB::bind_method(D_METHOD("get_axis", "axis"), &Vector4D::get_axis);
	ClassDB::bind_method(D_METHOD("set_axis", "axis", "value"), &Vector4D::set_axis);

	ClassDB::bind_method(D_METHOD("added", "b"), &Vector4D::added);
	ClassDB::bind_method(D_METHOD("subtracted", "b"), &Vector4D::subtracted);
	ClassDB::bind_method(D_METHOD("multiplied_scalar", "s"), &Vector4D::multiplied_scalar);
	ClassDB::bind_method(D_METHOD("divided_scalar", "s"), &Vector4D::divided_scalar);
	ClassDB::bind_method(D_METHOD("negated"), &Vector4D::negated);

	ClassDB::bind_method(D_METHOD("dot", "b"), &Vector4D::dot);
	ClassDB::bind_method(D_METHOD("length"), &Vector4D::length);
	ClassDB::bind_method(D_METHOD("length_squared"), &Vector4D::length_squared);
	ClassDB::bind_method(D_METHOD("normalized"), &Vector4D::normalized);
	ClassDB::bind_method(D_METHOD("is_normalized"), &Vector4D::is_normalized);
	ClassDB::bind_method(D_METHOD("distance_to", "to"), &Vector4D::distance_to);
	ClassDB::bind_method(D_METHOD("distance_squared_to", "to"), &Vector4D::distance_squared_to);

	ClassDB::bind_method(D_METHOD("lerp", "to", "weight"), &Vector4D::lerp);
	ClassDB::bind_method(D_METHOD("slerp", "to", "weight"), &Vector4D::slerp);

	ClassDB::bind_method(D_METHOD("abs"), &Vector4D::abs);
	ClassDB::bind_method(D_METHOD("sign"), &Vector4D::sign);
	ClassDB::bind_method(D_METHOD("floor"), &Vector4D::floor);
	ClassDB::bind_method(D_METHOD("ceil"), &Vector4D::ceil);
	ClassDB::bind_method(D_METHOD("round"), &Vector4D::round);
	ClassDB::bind_method(D_METHOD("clamp", "min", "max"), &Vector4D::clamp);

	ClassDB::bind_method(D_METHOD("min_axis_index"), &Vector4D::min_axis_index);
	ClassDB::bind_method(D_METHOD("max_axis_index"), &Vector4D::max_axis_index);

	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &Vector4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("is_zero_approx"), &Vector4D::is_zero_approx);

	ClassDB::bind_method(D_METHOD("to_vector4"), &Vector4D::to_vector4);
	ClassDB::bind_method(D_METHOD("to_string"), &Vector4D::to_string);

	ClassDB::bind_method(D_METHOD("get_x"), &Vector4D::get_x);
	ClassDB::bind_method(D_METHOD("get_y"), &Vector4D::get_y);
	ClassDB::bind_method(D_METHOD("get_z"), &Vector4D::get_z);
	ClassDB::bind_method(D_METHOD("get_w"), &Vector4D::get_w);
	ClassDB::bind_method(D_METHOD("set_x", "v"), &Vector4D::set_x);
	ClassDB::bind_method(D_METHOD("set_y", "v"), &Vector4D::set_y);
	ClassDB::bind_method(D_METHOD("set_z", "v"), &Vector4D::set_z);
	ClassDB::bind_method(D_METHOD("set_w", "v"), &Vector4D::set_w);

	// Properties (must come after setter bind_method calls)
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "x"), "set_x", "get_x");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "y"), "set_y", "get_y");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "z"), "set_z", "get_z");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w"), "set_w", "get_w");

	// PlaneAxis enum
}

} // namespace godot
