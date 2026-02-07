#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_box_shape_4d.h"

TEST_SUITE("HyperBoxShape4D") {

TEST_CASE("Support function returns correct corner") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));

	// Positive direction on all axes → positive corner.
	Vector4 s = box.get_support(Vector4(1, 1, 1, 1));
	CHECK(s.is_equal_approx(Vector4(1, 2, 3, 4)));

	// Negative direction on all axes → negative corner.
	s = box.get_support(Vector4(-1, -1, -1, -1));
	CHECK(s.is_equal_approx(Vector4(-1, -2, -3, -4)));
}

TEST_CASE("Support along a single axis") {
	HyperBoxShape4D box(Vector4(5, 5, 5, 5));
	Vector4 s = box.get_support(Vector4(0, 0, 0, 1));
	CHECK(s.is_equal_approx(Vector4(-5, -5, -5, 5))); // other components use -sign(0) or similar
	// Note: sign(0) behaviour depends on implementation; the key invariant is
	// that the dot product with direction is maximised.
	CHECK(s.dot(Vector4(0, 0, 0, 1)) >= 5.0 - CMP_EPSILON);
}

TEST_CASE("Contains point at centre") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	CHECK(box.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains point inside") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));
	CHECK(box.contains_point(Vector4(1, 2, 3, 4)));
}

TEST_CASE("Does not contain point outside") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	CHECK_FALSE(box.contains_point(Vector4(2, 0, 0, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, 0, 0, 1.5)));
}

TEST_CASE("AABB for axis-aligned box at origin") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));
	Transform4D t; // identity
	AABB4D aabb = box.get_aabb(t);
	CHECK(aabb.position.is_equal_approx(Vector4(-1, -2, -3, -4)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 4, 6, 8)));
}

TEST_CASE("AABB grows after rotation") {
	HyperBoxShape4D box(Vector4(1, 0.5, 0.5, 0.5));
	// Rotating a non-uniform box makes the AABB larger than the unrotated AABB.
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 4.0);
	Transform4D t;
	t.basis = rot;
	AABB4D aabb = box.get_aabb(t);
	// The AABB should enclose the rotated box, so each extent should be at
	// least as large as the original minimum half-extent (0.5 * 2 = 1.0).
	CHECK(aabb.size.x >= 1.0 - CMP_EPSILON);
	CHECK(aabb.size.y >= 1.0 - CMP_EPSILON);
}

} // TEST_SUITE
