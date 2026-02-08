#include "vector4d.h"

#include <algorithm>

// -- Named constants ---------------------------------------------------------

const Vector4D Vector4D::ZERO = Vector4D(0, 0, 0, 0);
const Vector4D Vector4D::ONE = Vector4D(1, 1, 1, 1);
const Vector4D Vector4D::AXIS_X = Vector4D(1, 0, 0, 0);
const Vector4D Vector4D::AXIS_Y = Vector4D(0, 1, 0, 0);
const Vector4D Vector4D::AXIS_Z = Vector4D(0, 0, 1, 0);
const Vector4D Vector4D::AXIS_W = Vector4D(0, 0, 0, 1);

// -- Constructors ------------------------------------------------------------

Vector4D::Vector4D() :
		x(0), y(0), z(0), w(0) {}

Vector4D::Vector4D(real_t p_x, real_t p_y, real_t p_z, real_t p_w) :
		x(p_x), y(p_y), z(p_z), w(p_w) {}

Vector4D::Vector4D(const Vector4 &p_v) :
		x(p_v.x), y(p_v.y), z(p_v.z), w(p_v.w) {}

// -- Conversion --------------------------------------------------------------

Vector4D::operator Vector4() const {
	return Vector4(x, y, z, w);
}

// -- Element access ----------------------------------------------------------

real_t &Vector4D::operator[](int p_idx) {
	return components[p_idx];
}

const real_t &Vector4D::operator[](int p_idx) const {
	return components[p_idx];
}

// -- Arithmetic operators ----------------------------------------------------

Vector4D Vector4D::operator+(const Vector4D &p_other) const {
	return Vector4D(x + p_other.x, y + p_other.y, z + p_other.z, w + p_other.w);
}

Vector4D Vector4D::operator-(const Vector4D &p_other) const {
	return Vector4D(x - p_other.x, y - p_other.y, z - p_other.z, w - p_other.w);
}

Vector4D Vector4D::operator*(real_t p_scalar) const {
	return Vector4D(x * p_scalar, y * p_scalar, z * p_scalar, w * p_scalar);
}

Vector4D Vector4D::operator/(real_t p_scalar) const {
	return Vector4D(x / p_scalar, y / p_scalar, z / p_scalar, w / p_scalar);
}

Vector4D Vector4D::operator-() const {
	return Vector4D(-x, -y, -z, -w);
}

Vector4D &Vector4D::operator+=(const Vector4D &p_other) {
	x += p_other.x;
	y += p_other.y;
	z += p_other.z;
	w += p_other.w;
	return *this;
}

Vector4D &Vector4D::operator-=(const Vector4D &p_other) {
	x -= p_other.x;
	y -= p_other.y;
	z -= p_other.z;
	w -= p_other.w;
	return *this;
}

Vector4D &Vector4D::operator*=(real_t p_scalar) {
	x *= p_scalar;
	y *= p_scalar;
	z *= p_scalar;
	w *= p_scalar;
	return *this;
}

Vector4D &Vector4D::operator/=(real_t p_scalar) {
	x /= p_scalar;
	y /= p_scalar;
	z /= p_scalar;
	w /= p_scalar;
	return *this;
}

bool Vector4D::operator==(const Vector4D &p_other) const {
	return x == p_other.x && y == p_other.y && z == p_other.z && w == p_other.w;
}

bool Vector4D::operator!=(const Vector4D &p_other) const {
	return !(*this == p_other);
}

// -- Component-wise operations -----------------------------------------------

Vector4D Vector4D::component_mul(const Vector4D &p_other) const {
	return Vector4D(x * p_other.x, y * p_other.y, z * p_other.z, w * p_other.w);
}

Vector4D Vector4D::component_div(const Vector4D &p_other) const {
	return Vector4D(x / p_other.x, y / p_other.y, z / p_other.z, w / p_other.w);
}

Vector4D Vector4D::abs() const {
	return Vector4D(std::abs(x), std::abs(y), std::abs(z), std::abs(w));
}

Vector4D Vector4D::sign() const {
	return Vector4D(
			(x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0),
			(y > 0) ? 1.0 : ((y < 0) ? -1.0 : 0.0),
			(z > 0) ? 1.0 : ((z < 0) ? -1.0 : 0.0),
			(w > 0) ? 1.0 : ((w < 0) ? -1.0 : 0.0));
}

Vector4D Vector4D::floor() const {
	return Vector4D(std::floor(x), std::floor(y), std::floor(z), std::floor(w));
}

Vector4D Vector4D::ceil() const {
	return Vector4D(std::ceil(x), std::ceil(y), std::ceil(z), std::ceil(w));
}

Vector4D Vector4D::round() const {
	return Vector4D(std::round(x), std::round(y), std::round(z), std::round(w));
}

Vector4D Vector4D::clamp(const Vector4D &p_min, const Vector4D &p_max) const {
	return Vector4D(
			std::clamp(x, p_min.x, p_max.x),
			std::clamp(y, p_min.y, p_max.y),
			std::clamp(z, p_min.z, p_max.z),
			std::clamp(w, p_min.w, p_max.w));
}

Vector4D Vector4D::snapped(const Vector4D &p_step) const {
	return Vector4D(
			p_step.x != 0 ? std::floor(x / p_step.x + 0.5) * p_step.x : x,
			p_step.y != 0 ? std::floor(y / p_step.y + 0.5) * p_step.y : y,
			p_step.z != 0 ? std::floor(z / p_step.z + 0.5) * p_step.z : z,
			p_step.w != 0 ? std::floor(w / p_step.w + 0.5) * p_step.w : w);
}

// -- Length and distance -----------------------------------------------------

real_t Vector4D::dot(const Vector4D &p_other) const {
	return x * p_other.x + y * p_other.y + z * p_other.z + w * p_other.w;
}

real_t Vector4D::length_squared() const {
	return dot(*this);
}

real_t Vector4D::length() const {
	return std::sqrt(length_squared());
}

real_t Vector4D::distance_to(const Vector4D &p_other) const {
	return (*this - p_other).length();
}

real_t Vector4D::distance_squared_to(const Vector4D &p_other) const {
	return (*this - p_other).length_squared();
}

// -- Normalization -----------------------------------------------------------

Vector4D Vector4D::normalized() const {
	real_t l = length();
	if (l > CMP_EPSILON) {
		return *this / l;
	}
	return Vector4D();
}

void Vector4D::normalize() {
	real_t l = length();
	if (l > CMP_EPSILON) {
		*this /= l;
	} else {
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}
}

bool Vector4D::is_normalized() const {
	return std::abs(length_squared() - 1.0) < CMP_EPSILON;
}

Vector4D Vector4D::safe_normalized(const Vector4D &p_fallback) const {
	real_t l = length();
	if (l > CMP_EPSILON) {
		return *this / l;
	}
	return p_fallback;
}

// -- Comparison --------------------------------------------------------------

bool Vector4D::is_zero_approx() const {
	return std::abs(x) < CMP_EPSILON && std::abs(y) < CMP_EPSILON &&
			std::abs(z) < CMP_EPSILON && std::abs(w) < CMP_EPSILON;
}

bool Vector4D::is_equal_approx(const Vector4D &p_other) const {
	return std::abs(x - p_other.x) < CMP_EPSILON &&
			std::abs(y - p_other.y) < CMP_EPSILON &&
			std::abs(z - p_other.z) < CMP_EPSILON &&
			std::abs(w - p_other.w) < CMP_EPSILON;
}

bool Vector4D::is_finite() const {
	return std::isfinite(x) && std::isfinite(y) &&
			std::isfinite(z) && std::isfinite(w);
}

// -- Interpolation -----------------------------------------------------------

Vector4D Vector4D::lerp(const Vector4D &p_to, real_t p_weight) const {
	return Vector4D(
			x + (p_to.x - x) * p_weight,
			y + (p_to.y - y) * p_weight,
			z + (p_to.z - z) * p_weight,
			w + (p_to.w - w) * p_weight);
}

Vector4D Vector4D::move_toward(const Vector4D &p_target, real_t p_delta) const {
	Vector4D diff = p_target - *this;
	real_t dist = diff.length();
	if (dist <= p_delta || dist < CMP_EPSILON) {
		return p_target;
	}
	return *this + diff / dist * p_delta;
}

// -- Axis queries ------------------------------------------------------------

int Vector4D::min_axis_index() const {
	int min_idx = 0;
	real_t min_val = x;
	for (int i = 1; i < 4; i++) {
		if (components[i] < min_val) {
			min_val = components[i];
			min_idx = i;
		}
	}
	return min_idx;
}

int Vector4D::max_axis_index() const {
	int max_idx = 0;
	real_t max_val = x;
	for (int i = 1; i < 4; i++) {
		if (components[i] > max_val) {
			max_val = components[i];
			max_idx = i;
		}
	}
	return max_idx;
}

Vector4D Vector4D::direction_to(const Vector4D &p_other) const {
	return (p_other - *this).normalized();
}

// -- 4D-specific: triple cross product ---------------------------------------

Vector4D Vector4D::triple_cross(const Vector4D &p_u, const Vector4D &p_v, const Vector4D &p_w) {
	// The triple cross product in 4D is the Hodge dual of the trivector
	// p_u ^ p_v ^ p_w. It produces a vector perpendicular to all three inputs.
	//
	// Each component is a 3x3 determinant formed by the cofactor expansion
	// of the formal 4x4 determinant with basis vectors in the first row:
	//   | e_x   e_y   e_z   e_w  |
	//   | u_x   u_y   u_z   u_w  |
	//   | v_x   v_y   v_z   v_w  |
	//   | w_x   w_y   w_z   w_w  |

	real_t rx = p_u.y * (p_v.z * p_w.w - p_v.w * p_w.z) -
			p_u.z * (p_v.y * p_w.w - p_v.w * p_w.y) +
			p_u.w * (p_v.y * p_w.z - p_v.z * p_w.y);

	real_t ry = -(p_u.x * (p_v.z * p_w.w - p_v.w * p_w.z) -
			p_u.z * (p_v.x * p_w.w - p_v.w * p_w.x) +
			p_u.w * (p_v.x * p_w.z - p_v.z * p_w.x));

	real_t rz = p_u.x * (p_v.y * p_w.w - p_v.w * p_w.y) -
			p_u.y * (p_v.x * p_w.w - p_v.w * p_w.x) +
			p_u.w * (p_v.x * p_w.y - p_v.y * p_w.x);

	real_t rw = -(p_u.x * (p_v.y * p_w.z - p_v.z * p_w.y) -
			p_u.y * (p_v.x * p_w.z - p_v.z * p_w.x) +
			p_u.z * (p_v.x * p_w.y - p_v.y * p_w.x));

	return Vector4D(rx, ry, rz, rw);
}

// -- 4D-specific: projection and rejection -----------------------------------

Vector4D Vector4D::project(const Vector4D &p_onto) const {
	real_t d = p_onto.length_squared();
	if (d < CMP_EPSILON) {
		return Vector4D();
	}
	return p_onto * (dot(p_onto) / d);
}

Vector4D Vector4D::reject(const Vector4D &p_from) const {
	return *this - project(p_from);
}

// -- 4D-specific: reflection and sliding -------------------------------------

Vector4D Vector4D::reflect(const Vector4D &p_normal) const {
	return *this - p_normal * (2.0 * dot(p_normal));
}

Vector4D Vector4D::slide(const Vector4D &p_normal) const {
	return *this - p_normal * dot(p_normal);
}

// -- 4D-specific: angles -----------------------------------------------------

real_t Vector4D::angle_to(const Vector4D &p_other) const {
	real_t d = std::sqrt(length_squared() * p_other.length_squared());
	if (d < CMP_EPSILON) {
		return 0.0;
	}
	real_t cos_angle = dot(p_other) / d;
	// Clamp to [-1, 1] to handle floating-point imprecision.
	cos_angle = std::clamp(cos_angle, (real_t)-1.0, (real_t)1.0);
	return std::acos(cos_angle);
}

bool Vector4D::is_perpendicular_to(const Vector4D &p_other) const {
	return std::abs(dot(p_other)) < CMP_EPSILON;
}

// -- 4D-specific: plane rotation ---------------------------------------------

Vector4D Vector4D::rotated_in_plane(int p_axis1, int p_axis2, real_t p_angle) const {
	Vector4D result = *this;
	real_t c = std::cos(p_angle);
	real_t s = std::sin(p_angle);
	real_t a1 = components[p_axis1];
	real_t a2 = components[p_axis2];
	result.components[p_axis1] = a1 * c - a2 * s;
	result.components[p_axis2] = a1 * s + a2 * c;
	return result;
}

// -- 4D-specific: perpendicular construction ---------------------------------

Vector4D Vector4D::find_any_perpendicular() const {
	// Pick the coordinate axis most perpendicular to this vector (the one
	// with the smallest absolute dot product), then reject it from this
	// vector and normalize.
	int min_idx = 0;
	real_t min_val = std::abs(x);
	for (int i = 1; i < 4; i++) {
		real_t val = std::abs(components[i]);
		if (val < min_val) {
			min_val = val;
			min_idx = i;
		}
	}
	Vector4D axis;
	axis.components[min_idx] = 1.0;
	Vector4D perp = axis.reject(*this);
	return perp.normalized();
}

// -- Global operator ---------------------------------------------------------

Vector4D operator*(real_t p_scalar, const Vector4D &p_vec) {
	return p_vec * p_scalar;
}
