#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/rotor4d.h"

TEST_SUITE("Rotor4D") {

TEST_CASE("Identity rotor leaves vector unchanged") {
	Rotor4D r; // identity
	Vector4 v(1, 2, 3, 4);
	CHECK(r.rotate(v).is_equal_approx(v));
}

TEST_CASE("90-degree rotation in XY plane") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI / 2.0);
	Vector4 vx(1, 0, 0, 0);
	CHECK(r.rotate(vx).is_equal_approx(Vector4(0, 1, 0, 0)));
}

TEST_CASE("180-degree rotation reverses components in the plane") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI);
	Vector4 vx(1, 0, 0, 0);
	CHECK(r.rotate(vx).is_equal_approx(Vector4(-1, 0, 0, 0)));

	Vector4 vy(0, 1, 0, 0);
	CHECK(r.rotate(vy).is_equal_approx(Vector4(0, -1, 0, 0)));
}

TEST_CASE("Rotation preserves vector length") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_ZW, 1.37);
	Vector4 v(1, 2, 3, 4);
	real_t before = v.length();
	real_t after = r.rotate(v).length();
	CHECK(is_equal_approx(before, after));
}

TEST_CASE("Composition of two rotations") {
	Rotor4D r1 = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI / 2.0);
	Rotor4D r2 = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI / 2.0);
	Rotor4D combined = r1 * r2; // should be 180 degrees in XY

	Vector4 vx(1, 0, 0, 0);
	CHECK(combined.rotate(vx).is_equal_approx(Vector4(-1, 0, 0, 0)));
}

TEST_CASE("Rotor conjugate gives inverse rotation") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_XW, 0.8);
	Rotor4D r_inv = r.conjugate();

	Vector4 v(3, 1, 4, 1);
	Vector4 round_trip = r_inv.rotate(r.rotate(v));
	CHECK(round_trip.is_equal_approx(v));
}

TEST_CASE("Rotor is normalised after construction") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_YZ, 2.5);
	CHECK(is_equal_approx(r.norm(), 1.0));
}

TEST_CASE("Conversion to Basis4D and back produces same rotation") {
	Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_XZ, 1.0);
	Basis4D mat = r.to_basis();
	Rotor4D r2 = Rotor4D::from_basis(mat);

	// Apply both to the same vector — results should match.
	Vector4 v(1, 2, 3, 4);
	CHECK(r.rotate(v).is_equal_approx(r2.rotate(v)));
}

TEST_CASE("Slerp at t=0 and t=1") {
	Rotor4D a = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, 0.0);
	Rotor4D b = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI / 2.0);

	Vector4 v(1, 0, 0, 0);
	CHECK(a.slerp(b, 0.0).rotate(v).is_equal_approx(a.rotate(v)));
	CHECK(a.slerp(b, 1.0).rotate(v).is_equal_approx(b.rotate(v)));
}

} // TEST_SUITE
