#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_sphere_shape_4d.h"

TEST_SUITE("HyperSphereShape4D") {

TEST_CASE("Support function returns point on surface") {
	HyperSphereShape4D sphere(2.0);
	Vector4 dir(1, 0, 0, 0);
	Vector4 s = sphere.get_support(dir);
	CHECK(s.is_equal_approx(Vector4(2, 0, 0, 0)));
}

TEST_CASE("Support function for arbitrary direction") {
	HyperSphereShape4D sphere(3.0);
	Vector4 dir(1, 1, 1, 1); // not normalised on purpose
	Vector4 s = sphere.get_support(dir);
	// Support should be on the surface: length == radius.
	CHECK(is_equal_approx(s.length(), 3.0));
	// Should point in the same direction as dir.
	CHECK(s.dot(dir) > 0.0);
}

TEST_CASE("Contains point at centre") {
	HyperSphereShape4D sphere(1.0);
	CHECK(sphere.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains point on boundary") {
	HyperSphereShape4D sphere(5.0);
	CHECK(sphere.contains_point(Vector4(5, 0, 0, 0)));
	CHECK(sphere.contains_point(Vector4(0, 0, 0, -5)));
}

TEST_CASE("Does not contain point outside") {
	HyperSphereShape4D sphere(1.0);
	CHECK_FALSE(sphere.contains_point(Vector4(1, 1, 0, 0)));
}

TEST_CASE("AABB for unit sphere at origin") {
	HyperSphereShape4D sphere(1.0);
	Transform4D t; // identity
	AABB4D aabb = sphere.get_aabb(t);
	CHECK(aabb.position.is_equal_approx(Vector4(-1, -1, -1, -1)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 2, 2, 2)));
}

TEST_CASE("AABB for translated sphere") {
	HyperSphereShape4D sphere(1.0);
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = sphere.get_aabb(t);
	CHECK(aabb.position.is_equal_approx(Vector4(9, 19, 29, 39)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 2, 2, 2)));
}

} // TEST_SUITE
