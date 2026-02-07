#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/aabb4d.h"

TEST_SUITE("AABB4D") {

TEST_CASE("Point containment — inside") {
	AABB4D box(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	CHECK(box.has_point(Vector4(1, 1, 1, 1)));
}

TEST_CASE("Point containment — outside") {
	AABB4D box(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	CHECK_FALSE(box.has_point(Vector4(3, 1, 1, 1)));
	CHECK_FALSE(box.has_point(Vector4(1, 1, 1, -1)));
}

TEST_CASE("Point containment — boundary") {
	AABB4D box(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	// Boundary points should be considered inside.
	CHECK(box.has_point(Vector4(0, 0, 0, 0)));
	CHECK(box.has_point(Vector4(2, 2, 2, 2)));
}

TEST_CASE("Overlapping boxes intersect") {
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	AABB4D b(Vector4(1, 1, 1, 1), Vector4(2, 2, 2, 2));
	CHECK(a.intersects(b));
	CHECK(b.intersects(a));
}

TEST_CASE("Separated boxes do not intersect") {
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	AABB4D b(Vector4(5, 5, 5, 5), Vector4(1, 1, 1, 1));
	CHECK_FALSE(a.intersects(b));
}

TEST_CASE("Merge encloses both boxes") {
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	AABB4D b(Vector4(3, 3, 3, 3), Vector4(1, 1, 1, 1));
	AABB4D merged = a.merge(b);

	CHECK(merged.has_point(Vector4(0.5, 0.5, 0.5, 0.5)));
	CHECK(merged.has_point(Vector4(3.5, 3.5, 3.5, 3.5)));
	CHECK(merged.position.is_equal_approx(Vector4(0, 0, 0, 0)));
	CHECK(merged.size.is_equal_approx(Vector4(4, 4, 4, 4)));
}

TEST_CASE("Grow expands the box symmetrically") {
	AABB4D box(Vector4(1, 1, 1, 1), Vector4(2, 2, 2, 2));
	AABB4D grown = box.grow(0.5);
	CHECK(grown.position.is_equal_approx(Vector4(0.5, 0.5, 0.5, 0.5)));
	CHECK(grown.size.is_equal_approx(Vector4(3, 3, 3, 3)));
}

TEST_CASE("Center is at midpoint") {
	AABB4D box(Vector4(2, 4, 6, 8), Vector4(2, 2, 2, 2));
	CHECK(box.get_center().is_equal_approx(Vector4(3, 5, 7, 9)));
}

TEST_CASE("Volume of a unit tesseract") {
	AABB4D box(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	CHECK(is_equal_approx(box.get_volume(), 1.0));

	AABB4D box2(Vector4(0, 0, 0, 0), Vector4(2, 3, 4, 5));
	CHECK(is_equal_approx(box2.get_volume(), 120.0));
}

} // TEST_SUITE
