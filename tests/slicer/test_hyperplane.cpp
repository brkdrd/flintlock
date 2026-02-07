#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/hyperplane4d.h"

TEST_SUITE("Hyperplane4D") {

TEST_CASE("Signed distance — point on plane is zero") {
	// Hyperplane at W = 0 (normal along W, through origin).
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	Vector4 on_plane(5, 3, -2, 0);
	CHECK(is_equal_approx(hp.distance_to(on_plane), 0.0));
}

TEST_CASE("Signed distance — point in front and behind") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	CHECK(hp.distance_to(Vector4(0, 0, 0, 3)) > 0.0);
	CHECK(hp.distance_to(Vector4(0, 0, 0, -2)) < 0.0);
}

TEST_CASE("is_point_over") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	CHECK(hp.is_point_over(Vector4(0, 0, 0, 1)));
	CHECK_FALSE(hp.is_point_over(Vector4(0, 0, 0, -1)));
}

TEST_CASE("Project point onto plane") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	Vector4 p(1, 2, 3, 7);
	Vector4 proj = hp.project(p);
	CHECK(proj.is_equal_approx(Vector4(1, 2, 3, 0)));
}

TEST_CASE("Segment intersection — crossing segment") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	Vector4 a(0, 0, 0, -1);
	Vector4 b(0, 0, 0, 1);
	Vector4 hit;
	bool intersects = hp.intersects_segment(a, b, &hit);
	CHECK(intersects);
	CHECK(hit.is_equal_approx(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Segment intersection — both on same side") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	Vector4 a(0, 0, 0, 1);
	Vector4 b(0, 0, 0, 5);
	Vector4 hit;
	CHECK_FALSE(hp.intersects_segment(a, b, &hit));
}

TEST_CASE("Offset hyperplane") {
	// Hyperplane at W = 3 (normal along W, distance 3 from origin).
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 3.0);
	CHECK(is_equal_approx(hp.distance_to(Vector4(0, 0, 0, 3)), 0.0));
	CHECK(hp.distance_to(Vector4(0, 0, 0, 5)) > 0.0);
}

TEST_CASE("Tangent basis spans the plane") {
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0);
	Vector4 t1, t2, t3;
	hp.get_tangent_basis(&t1, &t2, &t3);

	// Tangent vectors should be orthogonal to the normal.
	CHECK(is_equal_approx(t1.dot(Vector4(0, 0, 0, 1)), 0.0));
	CHECK(is_equal_approx(t2.dot(Vector4(0, 0, 0, 1)), 0.0));
	CHECK(is_equal_approx(t3.dot(Vector4(0, 0, 0, 1)), 0.0));

	// Tangent vectors should be mutually orthogonal and unit length.
	CHECK(is_equal_approx(t1.length(), 1.0));
	CHECK(is_equal_approx(t2.length(), 1.0));
	CHECK(is_equal_approx(t3.length(), 1.0));
	CHECK(is_equal_approx(t1.dot(t2), 0.0));
	CHECK(is_equal_approx(t1.dot(t3), 0.0));
	CHECK(is_equal_approx(t2.dot(t3), 0.0));
}

} // TEST_SUITE
