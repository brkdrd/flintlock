#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/basis4d.h"

TEST_SUITE("Basis4D") {

TEST_CASE("Identity matrix leaves vectors unchanged") {
	Basis4D identity;
	Vector4 v(1.0, 2.0, 3.0, 4.0);
	Vector4 result = identity.xform(v);
	CHECK(result.is_equal_approx(v));
}

TEST_CASE("Rotation in XY plane by 90 degrees") {
	// Rotation in the XY plane swaps X→Y, Y→-X, leaves Z and W untouched.
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 2.0);

	Vector4 vx(1, 0, 0, 0);
	Vector4 result = rot.xform(vx);
	CHECK(result.is_equal_approx(Vector4(0, 1, 0, 0)));

	Vector4 vy(0, 1, 0, 0);
	result = rot.xform(vy);
	CHECK(result.is_equal_approx(Vector4(-1, 0, 0, 0)));

	// Z and W axes should be unaffected.
	Vector4 vz(0, 0, 1, 0);
	CHECK(rot.xform(vz).is_equal_approx(vz));

	Vector4 vw(0, 0, 0, 1);
	CHECK(rot.xform(vw).is_equal_approx(vw));
}

TEST_CASE("Rotation in XW plane by 90 degrees") {
	// XW rotation swaps X→W, W→-X, leaves Y and Z untouched.
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XW, MATH_PI / 2.0);

	Vector4 vx(1, 0, 0, 0);
	CHECK(rot.xform(vx).is_equal_approx(Vector4(0, 0, 0, 1)));

	Vector4 vw(0, 0, 0, 1);
	CHECK(rot.xform(vw).is_equal_approx(Vector4(-1, 0, 0, 0)));
}

TEST_CASE("Transpose of rotation equals its inverse") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_YZ, 0.7);
	Basis4D inv = rot.inverse();
	Basis4D trans = rot.transposed();

	for (int i = 0; i < 4; i++) {
		CHECK(inv.get_row(i).is_equal_approx(trans.get_row(i)));
	}
}

TEST_CASE("Rotation matrix has determinant 1") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_ZW, 1.23);
	CHECK(is_equal_approx(rot.determinant(), 1.0));
}

TEST_CASE("Matrix multiplication is associative") {
	Basis4D a = Basis4D::from_rotation(Basis4D::PLANE_XY, 0.5);
	Basis4D b = Basis4D::from_rotation(Basis4D::PLANE_ZW, 0.8);
	Basis4D c = Basis4D::from_rotation(Basis4D::PLANE_XW, 0.3);

	Basis4D ab_c = (a * b) * c;
	Basis4D a_bc = a * (b * c);

	for (int i = 0; i < 4; i++) {
		CHECK(ab_c.get_row(i).is_equal_approx(a_bc.get_row(i)));
	}
}

TEST_CASE("Inverse undoes the transformation") {
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_YW, 2.1);
	Basis4D product = rot * rot.inverse();

	Basis4D identity;
	for (int i = 0; i < 4; i++) {
		CHECK(product.get_row(i).is_equal_approx(identity.get_row(i)));
	}
}

} // TEST_SUITE
