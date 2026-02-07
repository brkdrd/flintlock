#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/hyperplane4d.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "slicer/slicer4d.h"

TEST_SUITE("Sphere Slicing") {

TEST_CASE("Slice through center produces sphere of same radius") {
	HyperSphereShape4D sphere(5.0);
	Transform4D t; // identity — sphere centred at origin
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0); // W = 0

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	CHECK_FALSE(result.is_empty());
	// The cross-section of a 4-sphere at d=0 is a 3-sphere with the same radius.
	CHECK(is_equal_approx(result.bounding_radius, 5.0, 0.1));
}

TEST_CASE("Slice at offset produces smaller cross-section") {
	HyperSphereShape4D sphere(5.0);
	Transform4D t; // origin
	// Hyperplane at W = 3.  Distance from center to plane = 3.
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 3.0);

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	CHECK_FALSE(result.is_empty());
	// Expected 3-sphere radius = sqrt(25 - 9) = 4
	CHECK(is_equal_approx(result.bounding_radius, 4.0, 0.1));
}

TEST_CASE("Slice just touching the surface") {
	HyperSphereShape4D sphere(5.0);
	Transform4D t;
	// Hyperplane at W = 5 (tangent).
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 5.0);

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	// Tangent intersection is a single point — may be empty or degenerate.
	// Implementation may treat this as empty or as a zero-radius result.
	if (!result.is_empty()) {
		CHECK(is_equal_approx(result.bounding_radius, 0.0, 0.1));
	}
}

TEST_CASE("Slice beyond sphere produces empty result") {
	HyperSphereShape4D sphere(5.0);
	Transform4D t;
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 6.0);

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	CHECK(result.is_empty());
}

TEST_CASE("Slice of translated sphere") {
	HyperSphereShape4D sphere(2.0);
	Transform4D t = Transform4D::translated(Vector4(0, 0, 0, 10));
	// Hyperplane at W = 10 → passes through centre of translated sphere.
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 10.0);

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	CHECK_FALSE(result.is_empty());
	CHECK(is_equal_approx(result.bounding_radius, 2.0, 0.1));
}

TEST_CASE("Slice with rotated hyperplane") {
	HyperSphereShape4D sphere(3.0);
	Transform4D t; // origin
	// Hyperplane with normal along X axis instead of W.
	// Distance 0 → cuts through centre.
	Hyperplane4D hp(Vector4(1, 0, 0, 0), 0.0);

	SliceResult result = Slicer4D::slice_shape(&sphere, t, hp);
	CHECK_FALSE(result.is_empty());
	// Still a 3-sphere of radius 3 (sphere is symmetric).
	CHECK(is_equal_approx(result.bounding_radius, 3.0, 0.1));
}

} // TEST_SUITE
