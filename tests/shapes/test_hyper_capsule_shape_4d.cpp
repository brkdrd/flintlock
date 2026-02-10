#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_capsule_shape_4d.h"
#include "math/basis4d.h"

TEST_SUITE("HyperCapsuleShape4D") {

// ============================================================================
// Support Function Tests
// ============================================================================

TEST_CASE("Support along capsule axis returns hemisphere point") {
	HyperCapsuleShape4D capsule(1.0, 4.0); // radius=1, height=4
	Vector4 s = capsule.get_support(Vector4(0, 0, 0, 1)); // +W direction

	// Should return top hemisphere: center at (0,0,0,2) + radius in direction
	CHECK(s.w >= 2.0 + 0.99); // ~= 3.0
	CHECK(s.w <= 3.0 + CMP_EPSILON);
}

TEST_CASE("Support opposite to capsule axis") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	Vector4 s = capsule.get_support(Vector4(0, 0, 0, -1)); // -W direction

	// Should return bottom hemisphere: center at (0,0,0,-2) - radius
	CHECK(s.w <= -2.0 - 0.99);
	CHECK(s.w >= -3.0 - CMP_EPSILON);
}

TEST_CASE("Support perpendicular to capsule axis") {
	HyperCapsuleShape4D capsule(2.0, 6.0);
	Vector4 s = capsule.get_support(Vector4(1, 0, 0, 0)); // +X direction

	// Should return point on cylindrical surface or hemisphere
	// X component should be ~radius (2.0)
	CHECK(is_equal_approx(s.x, 2.0));
	// W component should be somewhere along the height
	CHECK(s.w >= -3.0 - CMP_EPSILON);
	CHECK(s.w <= 3.0 + CMP_EPSILON);
}

TEST_CASE("Support at diagonal direction") {
	HyperCapsuleShape4D capsule(1.5, 5.0);
	Vector4 dir(1, 1, 1, 1);
	Vector4 s = capsule.get_support(dir);

	// Support should maximize dot product with direction
	// Should be on surface: either hemisphere or cylinder
	real_t dist_from_axis = (s - Vector4(0, 0, 0, s.w)).length();
	CHECK(dist_from_axis <= 1.5 + CMP_EPSILON);
}

TEST_CASE("Support with zero direction returns valid point") {
	HyperCapsuleShape4D capsule(1.0, 2.0);
	Vector4 s = capsule.get_support(Vector4(0, 0, 0, 0));

	// Should return some valid point on the surface
	// Check it's within reasonable bounds
	CHECK(s.length() <= 3.0 + CMP_EPSILON); // max possible: height/2 + radius
}

TEST_CASE("Support function scales with radius") {
	HyperCapsuleShape4D small(1.0, 4.0);
	HyperCapsuleShape4D large(3.0, 4.0); // 3x radius

	Vector4 dir(1, 0, 0, 0);
	Vector4 s_small = small.get_support(dir);
	Vector4 s_large = large.get_support(dir);

	// Perpendicular component should scale with radius
	CHECK(is_equal_approx(s_large.x / s_small.x, 3.0));
}

TEST_CASE("Support function with opposite directions") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	Vector4 dir(1, 1, 0, 0);
	Vector4 s1 = capsule.get_support(dir);
	Vector4 s2 = capsule.get_support(-dir);

	// X and Y components should be opposite
	CHECK(is_equal_approx(s1.x, -s2.x));
	CHECK(is_equal_approx(s1.y, -s2.y));
}

TEST_CASE("Support is maximal in given direction") {
	HyperCapsuleShape4D capsule(2.0, 5.0);
	Vector4 dir(1, 2, -1, 3);
	Vector4 support = capsule.get_support(dir);
	real_t support_dot = support.dot(dir);

	// Test various points on the capsule surface
	// None should have greater dot product
	Vector4 test_points[] = {
		Vector4(2, 0, 0, 2.5),  // Top hemisphere
		Vector4(0, 2, 0, -2.5), // Bottom hemisphere
		Vector4(2, 0, 0, 0),    // Cylindrical surface
		Vector4(0, 0, 2, 1.0)   // Mid-cylinder
	};

	for (const auto& point : test_points) {
		if (capsule.contains_point(point)) {
			CHECK(point.dot(dir) <= support_dot + CMP_EPSILON);
		}
	}
}

// ============================================================================
// Containment Tests
// ============================================================================

TEST_CASE("Contains point at center") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	CHECK(capsule.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains points along central axis") {
	HyperCapsuleShape4D capsule(2.0, 6.0);

	// Points along W axis within height/2 should be contained
	CHECK(capsule.contains_point(Vector4(0, 0, 0, 2.5)));
	CHECK(capsule.contains_point(Vector4(0, 0, 0, -2.5)));
	CHECK(capsule.contains_point(Vector4(0, 0, 0, 0)));
	CHECK(capsule.contains_point(Vector4(0, 0, 0, 1.0)));
}

TEST_CASE("Contains points in cylindrical section") {
	HyperCapsuleShape4D capsule(3.0, 8.0);

	// Points within radius of axis and within height should be contained
	CHECK(capsule.contains_point(Vector4(2.0, 0, 0, 0)));
	CHECK(capsule.contains_point(Vector4(0, 2.5, 0, 3.0)));
	CHECK(capsule.contains_point(Vector4(1, 1, 1, 0)));
}

TEST_CASE("Contains points in hemisphere caps") {
	HyperCapsuleShape4D capsule(2.0, 4.0);

	// Points in top hemisphere (w > height/2)
	CHECK(capsule.contains_point(Vector4(1.0, 0, 0, 2.5)));
	CHECK(capsule.contains_point(Vector4(0, 0, 1.0, 3.5)));

	// Points in bottom hemisphere (w < -height/2)
	CHECK(capsule.contains_point(Vector4(1.0, 0, 0, -2.5)));
	CHECK(capsule.contains_point(Vector4(0, 1.0, 0, -3.5)));
}

TEST_CASE("Does not contain points outside") {
	HyperCapsuleShape4D capsule(1.0, 4.0);

	// Beyond radius perpendicular to axis
	CHECK_FALSE(capsule.contains_point(Vector4(2.0, 0, 0, 0)));
	CHECK_FALSE(capsule.contains_point(Vector4(0, 0, 1.5, 0)));

	// Beyond hemisphere caps
	CHECK_FALSE(capsule.contains_point(Vector4(0, 0, 0, 4.0)));
	CHECK_FALSE(capsule.contains_point(Vector4(0, 0, 0, -4.0)));
	CHECK_FALSE(capsule.contains_point(Vector4(1.0, 0, 0, 3.5)));
}

TEST_CASE("Boundary containment on cylinder surface") {
	HyperCapsuleShape4D capsule(5.0, 10.0);

	// Points exactly on cylinder surface should be contained
	CHECK(capsule.contains_point(Vector4(5, 0, 0, 0)));
	CHECK(capsule.contains_point(Vector4(0, 5, 0, 3)));
	CHECK(capsule.contains_point(Vector4(3, 4, 0, -4))); // 3²+4²=25, radius²=25
}

TEST_CASE("Boundary containment on hemisphere surface") {
	HyperCapsuleShape4D capsule(3.0, 6.0);

	// Point on top hemisphere surface
	Vector4 top_surface = Vector4(0, 0, 0, 3.0) + Vector4(2, 0, 0, 0);
	CHECK(capsule.contains_point(top_surface));

	// Point on bottom hemisphere surface
	Vector4 bottom_surface = Vector4(0, 0, 0, -3.0) + Vector4(0, 2, 0, 0);
	CHECK(capsule.contains_point(bottom_surface));
}

TEST_CASE("Containment with zero radius degenerates to line segment") {
	HyperCapsuleShape4D capsule(0.01, 5.0); // Very thin capsule

	// Points very close to axis should be contained
	CHECK(capsule.contains_point(Vector4(0, 0, 0, 2.0)));
	CHECK(capsule.contains_point(Vector4(0.005, 0, 0, 0)));

	// Points far from axis should not be contained
	CHECK_FALSE(capsule.contains_point(Vector4(1.0, 0, 0, 0)));
}

// ============================================================================
// AABB Tests
// ============================================================================

TEST_CASE("AABB for capsule at origin") {
	HyperCapsuleShape4D capsule(2.0, 6.0); // radius=2, height=6
	Transform4D t; // identity
	AABB4D aabb = capsule.get_aabb(t);

	// Should extend radius (2) in X, Y, Z and height/2 + radius (5) in W
	CHECK(aabb.position.is_equal_approx(Vector4(-2, -2, -2, -5)));
	CHECK(aabb.size.is_equal_approx(Vector4(4, 4, 4, 10)));
}

TEST_CASE("AABB for translated capsule") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = capsule.get_aabb(t);

	// Center at (10, 20, 30, 40), extends 1 in XYZ, 3 in W
	CHECK(aabb.position.is_equal_approx(Vector4(9, 19, 29, 37)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 2, 2, 6)));
}

TEST_CASE("AABB grows after rotation in XW plane") {
	HyperCapsuleShape4D capsule(1.0, 10.0); // Elongated in W
	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_XW, MATH_PI / 2.0);
	AABB4D aabb = capsule.get_aabb(t);

	// After rotating W into X, the AABB X extent should grow
	CHECK(aabb.size.x > 2.0); // Originally just 2*radius
}

TEST_CASE("AABB with non-uniform scaling") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	Transform4D t;
	t.basis = Basis4D::from_scale(Vector4(2, 3, 1, 1));
	AABB4D aabb = capsule.get_aabb(t);

	// X and Y should scale, affecting the radius
	CHECK(aabb.size.x >= 4.0 - CMP_EPSILON); // 2 * (radius * scale_x)
	CHECK(aabb.size.y >= 6.0 - CMP_EPSILON); // 2 * (radius * scale_y)
}

TEST_CASE("AABB contains capsule endpoints") {
	HyperCapsuleShape4D capsule(2.0, 8.0);
	Transform4D t = Transform4D::translated(Vector4(5, 5, 5, 5));
	AABB4D aabb = capsule.get_aabb(t);

	// Top endpoint: (5, 5, 5, 5+4+2) = (5, 5, 5, 11)
	CHECK(aabb.has_point(Vector4(5, 5, 5, 11)));

	// Bottom endpoint: (5, 5, 5, 5-4-2) = (5, 5, 5, -1)
	CHECK(aabb.has_point(Vector4(5, 5, 5, -1)));
}

TEST_CASE("AABB center matches transform origin") {
	HyperCapsuleShape4D capsule(3.0, 6.0);
	Vector4 offset(7, -2, 13, 5);
	Transform4D t = Transform4D::translated(offset);
	AABB4D aabb = capsule.get_aabb(t);

	Vector4 aabb_center = aabb.position + aabb.size * 0.5;
	CHECK(aabb_center.is_equal_approx(offset));
}

// ============================================================================
// Accessor Tests
// ============================================================================

TEST_CASE("Set and get radius") {
	HyperCapsuleShape4D capsule(1.0, 4.0);
	CHECK(is_equal_approx(capsule.get_radius(), 1.0));

	capsule.set_radius(3.5);
	CHECK(is_equal_approx(capsule.get_radius(), 3.5));

	// Verify it affects support function
	Vector4 s = capsule.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s.x, 3.5));
}

TEST_CASE("Set and get height") {
	HyperCapsuleShape4D capsule(2.0, 4.0);
	CHECK(is_equal_approx(capsule.get_height(), 4.0));

	capsule.set_height(10.0);
	CHECK(is_equal_approx(capsule.get_height(), 10.0));

	// Verify it affects AABB
	AABB4D aabb = capsule.get_aabb(Transform4D());
	CHECK(is_equal_approx(aabb.size.w, 14.0)); // height + 2*radius
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_CASE("Support point is always contained") {
	HyperCapsuleShape4D capsule(2.0, 5.0);

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 0, 1),
		Vector4(1, 1, 1, 1),
		Vector4(-1, 2, -3, 4),
		Vector4(5, -5, 5, -5)
	};

	for (const auto& dir : directions) {
		Vector4 support = capsule.get_support(dir);
		CHECK(capsule.contains_point(support));
	}
}

TEST_CASE("AABB contains capsule center") {
	HyperCapsuleShape4D capsule(2.0, 6.0);
	Vector4 center(10, -5, 3, 8);
	Transform4D t = Transform4D::translated(center);
	AABB4D aabb = capsule.get_aabb(t);

	CHECK(aabb.has_point(center));
}

TEST_CASE("AABB contains all support points") {
	HyperCapsuleShape4D capsule(1.5, 4.0);
	Transform4D t = Transform4D::translated(Vector4(2, 3, 4, 5));
	AABB4D aabb = capsule.get_aabb(t);

	// Test support points in all axis directions
	Vector4 axes[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1)
	};

	for (const auto& axis : axes) {
		Vector4 local_support = capsule.get_support(axis);
		Vector4 world_support = t.xform(local_support);
		CHECK(aabb.has_point(world_support));
	}
}

TEST_CASE("Capsule with zero height behaves like sphere") {
	HyperCapsuleShape4D capsule(3.0, 0.0);

	// Support should be spherical
	Vector4 s1 = capsule.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s1.length(), 3.0));

	// Containment should be spherical
	CHECK(capsule.contains_point(Vector4(2, 0, 0, 0)));
	CHECK_FALSE(capsule.contains_point(Vector4(4, 0, 0, 0)));
}

} // TEST_SUITE
