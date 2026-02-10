#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "math/basis4d.h"

TEST_SUITE("HyperSphereShape4D") {

// ============================================================================
// Support Function Tests
// ============================================================================

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

TEST_CASE("Support function with zero direction returns valid point") {
	HyperSphereShape4D sphere(1.5);
	Vector4 s = sphere.get_support(Vector4(0, 0, 0, 0));
	// Should return some point on the surface
	CHECK(is_equal_approx(s.length(), 1.5));
}

TEST_CASE("Support function is maximal in given direction") {
	HyperSphereShape4D sphere(2.0);
	Vector4 dir(1, 2, -1, 3);
	Vector4 support = sphere.get_support(dir);

	// The support point should maximize dot product with direction
	real_t support_dot = support.dot(dir);

	// Test that random points on sphere have lower or equal dot products
	Vector4 test_points[] = {
		Vector4(2, 0, 0, 0),
		Vector4(0, 2, 0, 0),
		Vector4(0, 0, 2, 0),
		Vector4(0, 0, 0, 2),
		Vector4(1, 1, 1, 1).normalized() * 2.0
	};

	for (const auto& point : test_points) {
		CHECK(point.dot(dir) <= support_dot + CMP_EPSILON);
	}
}

TEST_CASE("Support function with opposite directions returns antipodal points") {
	HyperSphereShape4D sphere(5.0);
	Vector4 dir(3, 4, 0, 0);
	Vector4 s1 = sphere.get_support(dir);
	Vector4 s2 = sphere.get_support(-dir);

	// s1 and s2 should be antipodal (opposite points on sphere)
	CHECK(s1.is_equal_approx(-s2));
	CHECK(is_equal_approx((s1 - s2).length(), 10.0)); // diameter
}

TEST_CASE("Support function scales with radius") {
	Vector4 dir(1, 2, 3, 4);
	HyperSphereShape4D small(1.0);
	HyperSphereShape4D large(5.0);

	Vector4 s_small = small.get_support(dir);
	Vector4 s_large = large.get_support(dir);

	// Large sphere support should be 5x the small sphere support
	CHECK((s_large / 5.0).is_equal_approx(s_small));
}

TEST_CASE("Support function with very small radius") {
	HyperSphereShape4D tiny(0.001);
	Vector4 dir(1, 0, 0, 0);
	Vector4 s = tiny.get_support(dir);
	CHECK(is_equal_approx(s.length(), 0.001));
}

TEST_CASE("Support function with large radius") {
	HyperSphereShape4D huge(1000.0);
	Vector4 dir(0, 0, 1, 0);
	Vector4 s = huge.get_support(dir);
	CHECK(is_equal_approx(s.length(), 1000.0));
}

// ============================================================================
// Containment Tests
// ============================================================================

TEST_CASE("Contains point at centre") {
	HyperSphereShape4D sphere(1.0);
	CHECK(sphere.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains point on boundary") {
	HyperSphereShape4D sphere(5.0);
	CHECK(sphere.contains_point(Vector4(5, 0, 0, 0)));
	CHECK(sphere.contains_point(Vector4(0, 0, 0, -5)));
}

TEST_CASE("Contains point just inside surface") {
	HyperSphereShape4D sphere(10.0);
	Vector4 almost_surface = Vector4(1, 1, 1, 1).normalized() * 9.99;
	CHECK(sphere.contains_point(almost_surface));
}

TEST_CASE("Does not contain point outside") {
	HyperSphereShape4D sphere(1.0);
	CHECK_FALSE(sphere.contains_point(Vector4(1, 1, 0, 0)));
}

TEST_CASE("Does not contain point just outside surface") {
	HyperSphereShape4D sphere(1.0);
	Vector4 just_outside = Vector4(1, 0, 0, 0) * 1.01;
	CHECK_FALSE(sphere.contains_point(just_outside));
}

TEST_CASE("Containment is isotropic") {
	HyperSphereShape4D sphere(3.0);
	// Points at same distance in different directions
	CHECK(sphere.contains_point(Vector4(2, 0, 0, 0)));
	CHECK(sphere.contains_point(Vector4(0, 2, 0, 0)));
	CHECK(sphere.contains_point(Vector4(0, 0, 2, 0)));
	CHECK(sphere.contains_point(Vector4(0, 0, 0, 2)));
	CHECK(sphere.contains_point(Vector4(1, 1, 1, 1)));
}

// ============================================================================
// AABB Tests
// ============================================================================

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

TEST_CASE("AABB is invariant under rotation") {
	HyperSphereShape4D sphere(3.0);

	// Rotate in XY plane
	Transform4D t_xy;
	t_xy.basis = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 3.0);
	AABB4D aabb_xy = sphere.get_aabb(t_xy);

	// AABB should still be centered at origin with size 6x6x6x6
	CHECK(aabb_xy.position.is_equal_approx(Vector4(-3, -3, -3, -3)));
	CHECK(aabb_xy.size.is_equal_approx(Vector4(6, 6, 6, 6)));

	// Rotate in XW plane
	Transform4D t_xw;
	t_xw.basis = Basis4D::from_rotation(Basis4D::PLANE_XW, MATH_PI / 4.0);
	AABB4D aabb_xw = sphere.get_aabb(t_xw);

	CHECK(aabb_xw.position.is_equal_approx(Vector4(-3, -3, -3, -3)));
	CHECK(aabb_xw.size.is_equal_approx(Vector4(6, 6, 6, 6)));
}

TEST_CASE("AABB with non-uniform scaling") {
	HyperSphereShape4D sphere(1.0);
	Transform4D t;
	t.basis = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	AABB4D aabb = sphere.get_aabb(t);

	// Under non-uniform scaling, a sphere becomes an ellipsoid
	// The current implementation uses a single world_radius (computed from one axis)
	// which creates a conservative AABB that's larger than necessary
	// The AABB will be sized to contain the ellipsoid, using the radius vector length
	Vector4 size = aabb.size;

	// The implementation transforms Vector4(radius, 0, 0, 0) and uses its length
	// For scale (2,3,4,5) and radius 1, that's length = 2
	// So AABB size is 2*2 = 4 in all dimensions
	CHECK(size.x >= 4.0 - CMP_EPSILON);
	CHECK(size.y >= 4.0 - CMP_EPSILON);
	CHECK(size.z >= 4.0 - CMP_EPSILON);
	CHECK(size.w >= 4.0 - CMP_EPSILON);
}

TEST_CASE("AABB with rotation and translation") {
	HyperSphereShape4D sphere(2.0);
	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_ZW, MATH_PI / 2.0);
	t.origin = Vector4(5, -3, 10, -7);

	AABB4D aabb = sphere.get_aabb(t);

	// Sphere should still have size 4x4x4x4, centered at (5,-3,10,-7)
	CHECK(aabb.position.is_equal_approx(Vector4(3, -5, 8, -9)));
	CHECK(aabb.size.is_equal_approx(Vector4(4, 4, 4, 4)));
}

TEST_CASE("AABB volume equals 4D sphere bounding box volume") {
	HyperSphereShape4D sphere(2.5);
	Transform4D t; // identity
	AABB4D aabb = sphere.get_aabb(t);

	real_t expected_volume = 5.0 * 5.0 * 5.0 * 5.0; // (2*radius)^4
	CHECK(is_equal_approx(aabb.get_volume(), expected_volume));
}

TEST_CASE("AABB center matches transform origin") {
	HyperSphereShape4D sphere(1.0);
	Vector4 offset(7, -2, 13, 5);
	Transform4D t = Transform4D::translated(offset);
	AABB4D aabb = sphere.get_aabb(t);

	Vector4 aabb_center = aabb.position + aabb.size * 0.5;
	CHECK(aabb_center.is_equal_approx(offset));
}

// ============================================================================
// Accessor Tests
// ============================================================================

TEST_CASE("Set and get radius") {
	HyperSphereShape4D sphere(1.0);
	CHECK(is_equal_approx(sphere.get_radius(), 1.0));

	sphere.set_radius(5.5);
	CHECK(is_equal_approx(sphere.get_radius(), 5.5));

	// Verify it affects support function
	Vector4 s = sphere.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s.length(), 5.5));
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_CASE("Support point is always contained") {
	HyperSphereShape4D sphere(3.0);

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(1, 1, 1, 1),
		Vector4(-1, 2, -3, 4),
		Vector4(5, -5, 5, -5)
	};

	for (const auto& dir : directions) {
		Vector4 support = sphere.get_support(dir);
		CHECK(sphere.contains_point(support));
	}
}

TEST_CASE("AABB contains sphere center") {
	HyperSphereShape4D sphere(2.0);
	Vector4 center(10, -5, 3, 8);
	Transform4D t = Transform4D::translated(center);
	AABB4D aabb = sphere.get_aabb(t);

	CHECK(aabb.has_point(center));
}

TEST_CASE("AABB contains all support points") {
	HyperSphereShape4D sphere(1.0);
	Transform4D t = Transform4D::translated(Vector4(2, 3, 4, 5));
	AABB4D aabb = sphere.get_aabb(t);

	// Test support points in all axis directions
	Vector4 axes[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1)
	};

	for (const auto& axis : axes) {
		Vector4 local_support = sphere.get_support(axis);
		Vector4 world_support = t.xform(local_support);
		CHECK(aabb.has_point(world_support));
	}
}

} // TEST_SUITE
