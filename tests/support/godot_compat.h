// Minimal Godot type shim for standalone test compilation.
// When building inside the GDExtension these types come from godot-cpp.
// This header provides ONLY enough surface area to let test code compile and
// run without pulling in the full engine.

#pragma once

#include <cmath>
#include <cstdint>

using real_t = double;

constexpr real_t CMP_EPSILON = 1e-6;
constexpr real_t MATH_PI = 3.14159265358979323846;

// ---------------------------------------------------------------------------
// Vector4
// ---------------------------------------------------------------------------
struct Vector4 {
	union {
		struct {
			real_t x, y, z, w;
		};
		real_t components[4] = {};
	};

	Vector4() : x(0), y(0), z(0), w(0) {}
	Vector4(real_t p_x, real_t p_y, real_t p_z, real_t p_w)
			: x(p_x), y(p_y), z(p_z), w(p_w) {}

	real_t &operator[](int idx) { return components[idx]; }
	const real_t &operator[](int idx) const { return components[idx]; }

	Vector4 operator+(const Vector4 &o) const { return { x + o.x, y + o.y, z + o.z, w + o.w }; }
	Vector4 operator-(const Vector4 &o) const { return { x - o.x, y - o.y, z - o.z, w - o.w }; }
	Vector4 operator*(real_t s) const { return { x * s, y * s, z * s, w * s }; }
	Vector4 operator/(real_t s) const { return { x / s, y / s, z / s, w / s }; }
	Vector4 operator-() const { return { -x, -y, -z, -w }; }

	Vector4 &operator+=(const Vector4 &o) {
		x += o.x; y += o.y; z += o.z; w += o.w;
		return *this;
	}
	Vector4 &operator-=(const Vector4 &o) {
		x -= o.x; y -= o.y; z -= o.z; w -= o.w;
		return *this;
	}

	real_t dot(const Vector4 &o) const { return x * o.x + y * o.y + z * o.z + w * o.w; }
	real_t length_squared() const { return dot(*this); }
	real_t length() const { return std::sqrt(length_squared()); }

	Vector4 normalized() const {
		real_t l = length();
		return l > CMP_EPSILON ? *this / l : Vector4();
	}

	bool is_equal_approx(const Vector4 &o, real_t eps = CMP_EPSILON) const {
		return std::abs(x - o.x) < eps && std::abs(y - o.y) < eps &&
			   std::abs(z - o.z) < eps && std::abs(w - o.w) < eps;
	}
};

inline Vector4 operator*(real_t s, const Vector4 &v) { return v * s; }

// ---------------------------------------------------------------------------
// Vector3 (needed for slice results)
// ---------------------------------------------------------------------------
struct Vector3 {
	union {
		struct {
			real_t x, y, z;
		};
		real_t components[3] = {};
	};

	Vector3() : x(0), y(0), z(0) {}
	Vector3(real_t p_x, real_t p_y, real_t p_z) : x(p_x), y(p_y), z(p_z) {}

	Vector3 operator+(const Vector3 &o) const { return { x + o.x, y + o.y, z + o.z }; }
	Vector3 operator-(const Vector3 &o) const { return { x - o.x, y - o.y, z - o.z }; }
	Vector3 operator*(real_t s) const { return { x * s, y * s, z * s }; }

	real_t dot(const Vector3 &o) const { return x * o.x + y * o.y + z * o.z; }
	real_t length_squared() const { return dot(*this); }
	real_t length() const { return std::sqrt(length_squared()); }

	Vector3 normalized() const {
		real_t l = length();
		return l > CMP_EPSILON ? *this / l : Vector3();
	}

	bool is_equal_approx(const Vector3 &o, real_t eps = CMP_EPSILON) const {
		return std::abs(x - o.x) < eps && std::abs(y - o.y) < eps &&
			   std::abs(z - o.z) < eps;
	}

	Vector3 operator/(real_t s) const { return { x / s, y / s, z / s }; }
};

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
inline bool is_equal_approx(real_t a, real_t b, real_t eps = CMP_EPSILON) {
	return std::abs(a - b) < eps;
}
