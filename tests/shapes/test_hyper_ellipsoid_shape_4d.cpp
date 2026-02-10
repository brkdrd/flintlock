#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_ellipsoid_shape_4d.h"
#include "math/basis4d.h"

TEST_SUITE("HyperEllipsoidShape4D") {

// ============================================================================
// Support Function Tests
// ============================================================================

TEST_CASE("Support along axis-aligned directions") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));

	CHECK(ellipsoid.get_support(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(2, 0, 0, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, 1, 0, 0)).is_equal_approx(Vector4(0, 3, 0, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, 0, 1, 0)).is_equal_approx(Vector4(0, 0, 4, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, 0, 0, 1)).is_equal_approx(Vector4(0, 0, 0, 5)));
}

TEST_CASE("Support in negative axis directions") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));

	CHECK(ellipsoid.get_support(Vector4(-1, 0, 0, 0)).is_equal_approx(Vector4(-1, 0, 0, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, -1, 0, 0)).is_equal_approx(Vector4(0, -2, 0, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, 0, -1, 0)).is_equal_approx(Vector4(0, 0, -3, 0)));
	CHECK(ellipsoid.get_support(Vector4(0, 0, 0, -1)).is_equal_approx(Vector4(0, 0, 0, -4)));
}

TEST_CASE("Support for diagonal direction") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Vector4 dir(1, 1, 1, 1);
	Vector4 s = ellipsoid.get_support(dir);

	// Support should maximize dot product
	real_t support_dot = s.dot(dir);

	// The support point should be on the ellipsoid surface
	// x²/a² + y²/b² + z²/c² + w²/d² = 1
	real_t surface_test =
		(s.x * s.x) / (2.0 * 2.0) +
		(s.y * s.y) / (3.0 * 3.0) +
		(s.z * s.z) / (4.0 * 4.0) +
		(s.w * s.w) / (5.0 * 5.0);

	CHECK(is_equal_approx(surface_test, 1.0));
}

TEST_CASE("Support function with zero direction") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));
	Vector4 s = ellipsoid.get_support(Vector4(0, 0, 0, 0));

	// Should return some valid point on the surface
	real_t surface_test =
		(s.x * s.x) / (1.0 * 1.0) +
		(s.y * s.y) / (2.0 * 2.0) +
		(s.z * s.z) / (3.0 * 3.0) +
		(s.w * s.w) / (4.0 * 4.0);

	CHECK(is_equal_approx(surface_test, 1.0));
}

TEST_CASE("Support function scales correctly with radii") {
	HyperEllipsoidShape4D small(Vector4(1, 1, 1, 1));
	HyperEllipsoidShape4D large(Vector4(3, 3, 3, 3));

	Vector4 dir(1, 1, 1, 1);
	Vector4 s_small = small.get_support(dir);
	Vector4 s_large = large.get_support(dir);

	// Large ellipsoid support should be 3x the small one
	CHECK((s_large / 3.0).is_equal_approx(s_small));
}

TEST_CASE("Support is maximal in given direction") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Vector4 dir(1, 2, -1, 1);
	Vector4 support = ellipsoid.get_support(dir);
	real_t support_dot = support.dot(dir);

	// Test points on ellipsoid surface in different directions
	// Should all have dot product <= support_dot
	Vector4 surface_points[] = {
		Vector4(2, 0, 0, 0),
		Vector4(0, 3, 0, 0),
		Vector4(0, 0, 4, 0),
		Vector4(0, 0, 0, 5),
		Vector4(1.4, 2.1, 0, 0) // On surface (approx)
	};

	for (const auto& point : surface_points) {
		CHECK(point.dot(dir) <= support_dot + CMP_EPSILON);
	}
}

TEST_CASE("Support with opposite directions returns opposite points") {
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 4, 5, 6));
	Vector4 dir(1, 1, 1, 1);
	Vector4 s1 = ellipsoid.get_support(dir);
	Vector4 s2 = ellipsoid.get_support(-dir);

	CHECK(s1.is_equal_approx(-s2));
}

TEST_CASE("Support with highly non-uniform radii") {
	HyperEllipsoidShape4D ellipsoid(Vector4(10, 1, 0.5, 0.1));

	Vector4 s_x = ellipsoid.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s_x.x, 10.0));

	Vector4 s_w = ellipsoid.get_support(Vector4(0, 0, 0, 1));
	CHECK(is_equal_approx(s_w.w, 0.1));
}

TEST_CASE("Support function for oblate ellipsoid") {
	// Flattened in W direction
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 3, 3, 1));

	Vector4 s = ellipsoid.get_support(Vector4(1, 1, 1, 1));

	// X, Y, Z components should be larger than W due to radii
	CHECK(abs(s.x) > abs(s.w));
	CHECK(abs(s.y) > abs(s.w));
	CHECK(abs(s.z) > abs(s.w));
}

TEST_CASE("Support function for prolate ellipsoid") {
	// Elongated in W direction
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 1, 1, 5));

	Vector4 s = ellipsoid.get_support(Vector4(1, 1, 1, 1));

	// W component should dominate due to large W radius
	CHECK(abs(s.w) > abs(s.x));
	CHECK(abs(s.w) > abs(s.y));
	CHECK(abs(s.w) > abs(s.z));
}

// ============================================================================
// Containment Tests
// ============================================================================

TEST_CASE("Contains point at center") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));
	CHECK(ellipsoid.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains points on axes within radii") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));

	CHECK(ellipsoid.contains_point(Vector4(1.9, 0, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 2.9, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 0, 3.9, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 0, 0, 4.9)));
}

TEST_CASE("Contains points on boundary") {
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 4, 5, 6));

	// Points exactly on the surface
	CHECK(ellipsoid.contains_point(Vector4(3, 0, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 4, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 0, 5, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 0, 0, 6)));
}

TEST_CASE("Contains points inside oblique to axes") {
	HyperEllipsoidShape4D ellipsoid(Vector4(4, 5, 6, 7));

	// Point that satisfies x²/a² + y²/b² + z²/c² + w²/d² < 1
	Vector4 p(2, 2, 2, 2);
	real_t test =
		(p.x * p.x) / (4.0 * 4.0) +
		(p.y * p.y) / (5.0 * 5.0) +
		(p.z * p.z) / (6.0 * 6.0) +
		(p.w * p.w) / (7.0 * 7.0);

	if (test <= 1.0) {
		CHECK(ellipsoid.contains_point(p));
	}
}

TEST_CASE("Does not contain points outside on axes") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));

	CHECK_FALSE(ellipsoid.contains_point(Vector4(2.1, 0, 0, 0)));
	CHECK_FALSE(ellipsoid.contains_point(Vector4(0, 3.1, 0, 0)));
	CHECK_FALSE(ellipsoid.contains_point(Vector4(0, 0, 4.1, 0)));
	CHECK_FALSE(ellipsoid.contains_point(Vector4(0, 0, 0, 5.1)));
}

TEST_CASE("Does not contain points outside ellipsoid surface") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));

	// Point beyond the ellipsoid surface
	Vector4 p(1, 2, 3, 4);
	real_t test =
		(p.x * p.x) / (1.0 * 1.0) +
		(p.y * p.y) / (2.0 * 2.0) +
		(p.z * p.z) / (3.0 * 3.0) +
		(p.w * p.w) / (4.0 * 4.0);

	// This should be > 1, so outside
	CHECK(test > 1.0);
	CHECK_FALSE(ellipsoid.contains_point(p));
}

TEST_CASE("Containment respects non-uniform radii") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 5, 1, 1));

	// Large radius in Y allows larger Y values
	CHECK(ellipsoid.contains_point(Vector4(0, 4, 0, 0)));

	// But not in other directions
	CHECK_FALSE(ellipsoid.contains_point(Vector4(4, 0, 0, 0)));
	CHECK_FALSE(ellipsoid.contains_point(Vector4(0, 0, 4, 0)));
}

TEST_CASE("Spherical ellipsoid behaves like sphere") {
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 3, 3, 3));

	// Should have spherical containment
	CHECK(ellipsoid.contains_point(Vector4(2, 0, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(0, 2, 0, 0)));
	CHECK(ellipsoid.contains_point(Vector4(1.5, 1.5, 1.5, 0)));

	// All points at distance 3 should be on boundary
	real_t r = 3.0 / std::sqrt(4.0); // Distance for (1,1,1,1)
	Vector4 corner = Vector4(1, 1, 1, 1).normalized() * 3.0;
	CHECK(ellipsoid.contains_point(corner));
}

// ============================================================================
// AABB Tests
// ============================================================================

TEST_CASE("AABB for ellipsoid at origin") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Transform4D t; // identity
	AABB4D aabb = ellipsoid.get_aabb(t);

	// AABB should extend to radii in each direction
	CHECK(aabb.position.is_equal_approx(Vector4(-2, -3, -4, -5)));
	CHECK(aabb.size.is_equal_approx(Vector4(4, 6, 8, 10)));
}

TEST_CASE("AABB for translated ellipsoid") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = ellipsoid.get_aabb(t);

	CHECK(aabb.position.is_equal_approx(Vector4(9, 18, 27, 36)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 4, 6, 8)));
}

TEST_CASE("AABB grows after rotation") {
	HyperEllipsoidShape4D ellipsoid(Vector4(5, 1, 1, 1));

	// Rotate elongated axis
	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 4.0);
	AABB4D aabb = ellipsoid.get_aabb(t);

	// Both X and Y should grow due to rotation
	CHECK(aabb.size.x > 2.0); // Originally just 2*1
	CHECK(aabb.size.y > 2.0);
}

TEST_CASE("AABB with non-uniform scaling") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 1, 1, 1));
	Transform4D t;
	t.basis = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	AABB4D aabb = ellipsoid.get_aabb(t);

	// Each dimension should scale with corresponding scale factor
	CHECK(aabb.size.x >= 4.0 - CMP_EPSILON);
	CHECK(aabb.size.y >= 6.0 - CMP_EPSILON);
	CHECK(aabb.size.z >= 8.0 - CMP_EPSILON);
	CHECK(aabb.size.w >= 10.0 - CMP_EPSILON);
}

TEST_CASE("AABB contains ellipsoid extrema") {
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 4, 5, 6));
	Transform4D t = Transform4D::translated(Vector4(10, 10, 10, 10));
	AABB4D aabb = ellipsoid.get_aabb(t);

	// Should contain all extremal points
	CHECK(aabb.has_point(Vector4(13, 10, 10, 10))); // +X extremum
	CHECK(aabb.has_point(Vector4(7, 10, 10, 10)));  // -X extremum
	CHECK(aabb.has_point(Vector4(10, 14, 10, 10))); // +Y extremum
	CHECK(aabb.has_point(Vector4(10, 6, 10, 10)));  // -Y extremum
}

TEST_CASE("AABB center matches transform origin") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Vector4 offset(7, -2, 13, 5);
	Transform4D t = Transform4D::translated(offset);
	AABB4D aabb = ellipsoid.get_aabb(t);

	Vector4 aabb_center = aabb.position + aabb.size * 0.5;
	CHECK(aabb_center.is_equal_approx(offset));
}

// ============================================================================
// Accessor Tests
// ============================================================================

TEST_CASE("Set and get radii") {
	HyperEllipsoidShape4D ellipsoid(Vector4(1, 2, 3, 4));
	CHECK(ellipsoid.get_radii().is_equal_approx(Vector4(1, 2, 3, 4)));

	ellipsoid.set_radii(Vector4(5, 10, 15, 20));
	CHECK(ellipsoid.get_radii().is_equal_approx(Vector4(5, 10, 15, 20)));

	// Verify it affects support function
	Vector4 s = ellipsoid.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s.x, 5.0));
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_CASE("Support point is always contained") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(1, 1, 1, 1),
		Vector4(-1, 2, -3, 4),
		Vector4(5, -5, 5, -5)
	};

	for (const auto& dir : directions) {
		Vector4 support = ellipsoid.get_support(dir);
		CHECK(ellipsoid.contains_point(support));
	}
}

TEST_CASE("AABB contains ellipsoid center") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Vector4 center(10, -5, 3, 8);
	Transform4D t = Transform4D::translated(center);
	AABB4D aabb = ellipsoid.get_aabb(t);

	CHECK(aabb.has_point(center));
}

TEST_CASE("AABB contains all support points") {
	HyperEllipsoidShape4D ellipsoid(Vector4(2, 3, 4, 5));
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = ellipsoid.get_aabb(t);

	// Test support points in all axis directions
	Vector4 axes[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1)
	};

	for (const auto& axis : axes) {
		Vector4 local_support = ellipsoid.get_support(axis);
		Vector4 world_support = t.xform(local_support);
		CHECK(aabb.has_point(world_support));
	}
}

TEST_CASE("Support point lies on ellipsoid surface") {
	HyperEllipsoidShape4D ellipsoid(Vector4(3, 4, 5, 6));

	Vector4 directions[] = {
		Vector4(1, 1, 1, 1),
		Vector4(1, 2, -1, 3),
		Vector4(-1, -1, 1, 1)
	};

	for (const auto& dir : directions) {
		Vector4 s = ellipsoid.get_support(dir);

		// Check surface equation: sum of (component/radius)² = 1
		real_t surface_test =
			(s.x * s.x) / (3.0 * 3.0) +
			(s.y * s.y) / (4.0 * 4.0) +
			(s.z * s.z) / (5.0 * 5.0) +
			(s.w * s.w) / (6.0 * 6.0);

		CHECK(is_equal_approx(surface_test, 1.0));
	}
}

} // TEST_SUITE
