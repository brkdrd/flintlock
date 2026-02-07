#include <doctest/doctest.h>
#include "godot_compat.h"
#include "collision/gjk_4d.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "shapes/hyper_box_shape_4d.h"

TEST_SUITE("GJK4D") {

TEST_CASE("Separated spheres — no collision") {
	HyperSphereShape4D a(1.0);
	HyperSphereShape4D b(1.0);
	Transform4D ta; // origin
	Transform4D tb = Transform4D::translated(Vector4(5, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	CHECK_FALSE(result.colliding);
}

TEST_CASE("Overlapping spheres — collision detected") {
	HyperSphereShape4D a(2.0);
	HyperSphereShape4D b(2.0);
	Transform4D ta;
	Transform4D tb = Transform4D::translated(Vector4(1, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	CHECK(result.colliding);
}

TEST_CASE("Touching spheres — collision detected") {
	HyperSphereShape4D a(1.0);
	HyperSphereShape4D b(1.0);
	Transform4D ta;
	Transform4D tb = Transform4D::translated(Vector4(2, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	// Touching = overlap at boundary. Implementation may treat as colliding.
	// The key invariant: no false negative for overlapping shapes.
	CHECK(result.colliding);
}

TEST_CASE("Sphere vs box — separated") {
	HyperSphereShape4D sphere(1.0);
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D ts;
	Transform4D tb = Transform4D::translated(Vector4(5, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&sphere, ts, &box, tb);
	CHECK_FALSE(result.colliding);
}

TEST_CASE("Sphere vs box — overlapping") {
	HyperSphereShape4D sphere(1.0);
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D ts;
	Transform4D tb = Transform4D::translated(Vector4(1.5, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&sphere, ts, &box, tb);
	CHECK(result.colliding);
}

TEST_CASE("Box vs box — separated") {
	HyperBoxShape4D a(Vector4(1, 1, 1, 1));
	HyperBoxShape4D b(Vector4(1, 1, 1, 1));
	Transform4D ta;
	Transform4D tb = Transform4D::translated(Vector4(3, 0, 0, 0));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	CHECK_FALSE(result.colliding);
}

TEST_CASE("Box vs box — overlapping") {
	HyperBoxShape4D a(Vector4(1, 1, 1, 1));
	HyperBoxShape4D b(Vector4(1, 1, 1, 1));
	Transform4D ta;
	Transform4D tb = Transform4D::translated(Vector4(0.5, 0.5, 0.5, 0.5));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	CHECK(result.colliding);
}

TEST_CASE("Separated along W axis") {
	HyperSphereShape4D a(1.0);
	HyperSphereShape4D b(1.0);
	Transform4D ta;
	Transform4D tb = Transform4D::translated(Vector4(0, 0, 0, 10));

	GJKResult4D result = GJK4D::intersect(&a, ta, &b, tb);
	CHECK_FALSE(result.colliding);
}

} // TEST_SUITE
