#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/transform4d.h"

TEST_SUITE("Transform4D") {

TEST_CASE("Identity transform leaves point unchanged") {
	Transform4D t;
	Vector4 p(3.0, -1.0, 0.5, 7.0);
	CHECK(t.xform(p).is_equal_approx(p));
}

TEST_CASE("Pure translation") {
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	Vector4 p(1, 2, 3, 4);
	CHECK(t.xform(p).is_equal_approx(Vector4(11, 22, 33, 44)));
}

TEST_CASE("basis_xform ignores translation") {
	Transform4D t = Transform4D::translated(Vector4(100, 200, 300, 400));
	Vector4 dir(1, 0, 0, 0);
	// Direction transform should not include origin.
	CHECK(t.basis_xform(dir).is_equal_approx(dir));
}

TEST_CASE("Composition of two translations") {
	Transform4D t1 = Transform4D::translated(Vector4(1, 0, 0, 0));
	Transform4D t2 = Transform4D::translated(Vector4(0, 0, 0, 5));
	Transform4D combined = t1 * t2;

	Vector4 origin(0, 0, 0, 0);
	CHECK(combined.xform(origin).is_equal_approx(Vector4(1, 0, 0, 5)));
}

TEST_CASE("Rotation + translation composition") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 2.0);
	Transform4D t;
	t.basis = rot;
	t.origin = Vector4(0, 0, 0, 10);

	// First rotates, then translates.
	Vector4 p(1, 0, 0, 0);
	Vector4 result = t.xform(p);
	CHECK(result.is_equal_approx(Vector4(0, 1, 0, 10)));
}

TEST_CASE("Inverse undoes the transform") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XZ, 0.9);
	Transform4D t;
	t.basis = rot;
	t.origin = Vector4(5, -3, 2, 1);

	Vector4 p(1, 2, 3, 4);
	Vector4 round_trip = t.inverse().xform(t.xform(p));
	CHECK(round_trip.is_equal_approx(p));
}

TEST_CASE("xform_inv is consistent with inverse().xform()") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_YW, 1.1);
	Transform4D t;
	t.basis = rot;
	t.origin = Vector4(2, 4, 6, 8);

	Vector4 p(7, 3, -1, 5);
	Vector4 a = t.xform_inv(p);
	Vector4 b = t.inverse().xform(p);
	CHECK(a.is_equal_approx(b));
}

} // TEST_SUITE
