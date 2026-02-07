#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/hyperplane4d.h"
#include "shapes/hyper_box_shape_4d.h"
#include "slicer/slicer4d.h"

TEST_SUITE("Box Slicing") {

TEST_CASE("Axis-aligned slice through tesseract center produces a cube") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1)); // unit half-extents
	Transform4D t; // identity
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.0); // W = 0

	SliceResult result = Slicer4D::slice_shape(&box, t, hp);
	CHECK_FALSE(result.is_empty());
	// Cross-section of a tesseract at W=0 is a cube with the same XYZ extents.
	// All 3D vertices should have coordinates in [-1, 1].
	for (int i = 0; i < result.vertex_count(); i++) {
		Vector3 v = result.get_vertex(i);
		CHECK(v.x >= -1.0 - CMP_EPSILON);
		CHECK(v.x <= 1.0 + CMP_EPSILON);
		CHECK(v.y >= -1.0 - CMP_EPSILON);
		CHECK(v.y <= 1.0 + CMP_EPSILON);
		CHECK(v.z >= -1.0 - CMP_EPSILON);
		CHECK(v.z <= 1.0 + CMP_EPSILON);
	}
}

TEST_CASE("Slice at W = 0.5 still intersects") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D t;
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 0.5);

	SliceResult result = Slicer4D::slice_shape(&box, t, hp);
	CHECK_FALSE(result.is_empty());
	// Still a cube: the box extends from W=-1 to W=1, so W=0.5 is inside.
}

TEST_CASE("Slice beyond the box produces empty result") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D t;
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 2.0);

	SliceResult result = Slicer4D::slice_shape(&box, t, hp);
	CHECK(result.is_empty());
}

TEST_CASE("Diagonal slice produces a non-cubic cross-section") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D t;
	// Hyperplane with a tilted normal: (0, 0, 1, 1) normalised.
	Vector4 normal = Vector4(0, 0, 1, 1).normalized();
	Hyperplane4D hp(normal, 0.0);

	SliceResult result = Slicer4D::slice_shape(&box, t, hp);
	CHECK_FALSE(result.is_empty());
	// The cross-section should be non-empty and non-degenerate.
	CHECK(result.vertex_count() >= 4);
}

TEST_CASE("Slice of translated box") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D t = Transform4D::translated(Vector4(0, 0, 0, 5));
	Hyperplane4D hp(Vector4(0, 0, 0, 1), 5.0);

	SliceResult result = Slicer4D::slice_shape(&box, t, hp);
	CHECK_FALSE(result.is_empty());
}

} // TEST_SUITE
