#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/hyper_box_shape_4d.h"
#include "math/basis4d.h"

TEST_SUITE("HyperBoxShape4D") {

// ============================================================================
// Support Function Tests
// ============================================================================

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

TEST_CASE("Support function with mixed direction signs") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));

	Vector4 s1 = box.get_support(Vector4(1, -1, 1, -1));
	CHECK(s1.is_equal_approx(Vector4(2, -3, 4, -5)));

	Vector4 s2 = box.get_support(Vector4(-1, 1, -1, 1));
	CHECK(s2.is_equal_approx(Vector4(-2, 3, -4, 5)));
}

TEST_CASE("Support function is maximal in given direction") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));
	Vector4 dir(1, 1, -1, 1);
	Vector4 support = box.get_support(dir);

	real_t support_dot = support.dot(dir);

	// All 16 vertices should have dot product <= support_dot
	for (int i = 0; i < 16; i++) {
		Vector4 vertex(
			(i & 1) ? 1.0 : -1.0,
			(i & 2) ? 2.0 : -2.0,
			(i & 4) ? 3.0 : -3.0,
			(i & 8) ? 4.0 : -4.0
		);
		CHECK(vertex.dot(dir) <= support_dot + CMP_EPSILON);
	}
}

TEST_CASE("Support with opposite directions returns opposite corners") {
	HyperBoxShape4D box(Vector4(3, 4, 5, 6));
	Vector4 dir(1, 1, 1, 1);
	Vector4 s1 = box.get_support(dir);
	Vector4 s2 = box.get_support(-dir);

	CHECK(s1.is_equal_approx(-s2));
}

TEST_CASE("Support for axis-aligned directions") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));

	CHECK(box.get_support(Vector4(1, 0, 0, 0)).is_equal_approx(Vector4(1, -2, -3, -4)));
	CHECK(box.get_support(Vector4(0, 1, 0, 0)).is_equal_approx(Vector4(-1, 2, -3, -4)));
	CHECK(box.get_support(Vector4(0, 0, 1, 0)).is_equal_approx(Vector4(-1, -2, 3, -4)));
	CHECK(box.get_support(Vector4(0, 0, 0, 1)).is_equal_approx(Vector4(-1, -2, -3, 4)));
}

TEST_CASE("Support function with very small extents") {
	HyperBoxShape4D tiny(Vector4(0.01, 0.02, 0.03, 0.04));
	Vector4 s = tiny.get_support(Vector4(1, 1, 1, 1));
	CHECK(s.is_equal_approx(Vector4(0.01, 0.02, 0.03, 0.04)));
}

TEST_CASE("Support function with large extents") {
	HyperBoxShape4D huge(Vector4(100, 200, 300, 400));
	Vector4 s = huge.get_support(Vector4(1, 1, 1, 1));
	CHECK(s.is_equal_approx(Vector4(100, 200, 300, 400)));
}

TEST_CASE("Support function with non-uniform extents") {
	HyperBoxShape4D box(Vector4(10, 1, 0.5, 0.1));
	Vector4 s1 = box.get_support(Vector4(1, 0, 0, 0));
	CHECK(s1.x > 0.0); // Should pick positive X
	CHECK(is_equal_approx(s1.x, 10.0));

	Vector4 s2 = box.get_support(Vector4(0, 0, 0, 1));
	CHECK(s2.w > 0.0);
	CHECK(is_equal_approx(s2.w, 0.1));
}

// ============================================================================
// Containment Tests
// ============================================================================

TEST_CASE("Contains point at centre") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	CHECK(box.contains_point(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Contains point inside") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));
	CHECK(box.contains_point(Vector4(1, 2, 3, 4)));
	CHECK(box.contains_point(Vector4(-1, -2, -3, -4)));
	CHECK(box.contains_point(Vector4(0.5, 0.5, 0.5, 0.5)));
}

TEST_CASE("Contains point on boundary") {
	HyperBoxShape4D box(Vector4(5, 5, 5, 5));
	CHECK(box.contains_point(Vector4(5, 0, 0, 0)));
	CHECK(box.contains_point(Vector4(-5, 0, 0, 0)));
	CHECK(box.contains_point(Vector4(0, 5, 0, 0)));
	CHECK(box.contains_point(Vector4(0, 0, 0, -5)));
}

TEST_CASE("Contains all vertices") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));

	// All 16 vertices should be contained
	for (int i = 0; i < 16; i++) {
		Vector4 vertex(
			(i & 1) ? 2.0 : -2.0,
			(i & 2) ? 3.0 : -3.0,
			(i & 4) ? 4.0 : -4.0,
			(i & 8) ? 5.0 : -5.0
		);
		CHECK(box.contains_point(vertex));
	}
}

TEST_CASE("Does not contain point outside") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	CHECK_FALSE(box.contains_point(Vector4(2, 0, 0, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, 0, 0, 1.5)));
	CHECK_FALSE(box.contains_point(Vector4(1, 1, 1, 1.1)));
}

TEST_CASE("Does not contain point just outside on one axis") {
	HyperBoxShape4D box(Vector4(3, 3, 3, 3));
	CHECK_FALSE(box.contains_point(Vector4(3.01, 0, 0, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, -3.01, 0, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, 0, 3.01, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, 0, 0, -3.01)));
}

TEST_CASE("Containment respects non-uniform extents") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));

	CHECK(box.contains_point(Vector4(0.9, 1.9, 2.9, 3.9)));
	CHECK_FALSE(box.contains_point(Vector4(1.1, 0, 0, 0)));
	CHECK_FALSE(box.contains_point(Vector4(0, 2.1, 0, 0)));
}

// ============================================================================
// AABB Tests
// ============================================================================

TEST_CASE("AABB for axis-aligned box at origin") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));
	Transform4D t; // identity
	AABB4D aabb = box.get_aabb(t);
	CHECK(aabb.position.is_equal_approx(Vector4(-1, -2, -3, -4)));
	CHECK(aabb.size.is_equal_approx(Vector4(2, 4, 6, 8)));
}

TEST_CASE("AABB for translated box") {
	HyperBoxShape4D box(Vector4(2, 2, 2, 2));
	Transform4D t = Transform4D::translated(Vector4(5, -3, 7, -1));
	AABB4D aabb = box.get_aabb(t);

	CHECK(aabb.position.is_equal_approx(Vector4(3, -5, 5, -3)));
	CHECK(aabb.size.is_equal_approx(Vector4(4, 4, 4, 4)));
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

TEST_CASE("AABB after 90-degree rotation in XY plane") {
	HyperBoxShape4D box(Vector4(3, 1, 1, 1));
	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 2.0);
	AABB4D aabb = box.get_aabb(t);

	// After 90° rotation in XY, X and Y axes swap
	// The large X extent (6) and small Y extent (2) swap places
	// Total extents in XY should sum to 8
	real_t xy_sum = aabb.size.x + aabb.size.y;
	CHECK(is_equal_approx(xy_sum, 8.0));
	// Z and W unaffected
	CHECK(is_equal_approx(aabb.size.z, 2.0));
	CHECK(is_equal_approx(aabb.size.w, 2.0));
}

TEST_CASE("AABB after rotation in multiple planes") {
	HyperBoxShape4D box(Vector4(5, 1, 1, 1));

	// Rotate in XW plane
	Transform4D t_xw;
	t_xw.basis = Basis4D::from_rotation(Basis4D::PLANE_XW, MATH_PI / 4.0);
	AABB4D aabb_xw = box.get_aabb(t_xw);

	// Both X and W should grow
	CHECK(aabb_xw.size.x > 2.0);
	CHECK(aabb_xw.size.w > 2.0);

	// Rotate in YZ plane (shouldn't affect X)
	Transform4D t_yz;
	t_yz.basis = Basis4D::from_rotation(Basis4D::PLANE_YZ, MATH_PI / 3.0);
	AABB4D aabb_yz = box.get_aabb(t_yz);

	// X should remain close to original (10), Y and Z should mix
	CHECK(is_equal_approx(aabb_yz.size.x, 10.0));
}

TEST_CASE("AABB with non-uniform scaling") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	Transform4D t;
	t.basis = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	AABB4D aabb = box.get_aabb(t);

	// Each extent should be scaled by the corresponding scale factor
	CHECK(aabb.size.is_equal_approx(Vector4(4, 6, 8, 10)));
}

TEST_CASE("AABB with rotation and non-uniform scaling") {
	HyperBoxShape4D box(Vector4(2, 1, 1, 1));

	// First scale, then rotate
	Basis4D scale = Basis4D::from_scale(Vector4(2, 1, 1, 1));
	Basis4D rot = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 4.0);

	Transform4D t;
	t.basis = rot * scale;
	AABB4D aabb = box.get_aabb(t);

	// AABB should encompass the transformed box
	// The scaled X dimension (4) rotated into XY plane affects both X and Y
	real_t min_extent = 2.0; // At least the original Y extent
	CHECK(aabb.size.x >= min_extent - CMP_EPSILON);
	CHECK(aabb.size.y >= min_extent - CMP_EPSILON);
}

TEST_CASE("AABB contains all transformed vertices") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));
	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_XZ, 0.7);
	t.origin = Vector4(5, -2, 10, 3);

	AABB4D aabb = box.get_aabb(t);

	// Generate all 16 vertices and transform them
	for (int i = 0; i < 16; i++) {
		Vector4 vertex(
			(i & 1) ? 1.0 : -1.0,
			(i & 2) ? 2.0 : -2.0,
			(i & 4) ? 3.0 : -3.0,
			(i & 8) ? 4.0 : -4.0
		);
		Vector4 transformed = t.xform(vertex);
		CHECK(aabb.has_point(transformed));
	}
}

TEST_CASE("AABB volume increases with rotation for non-uniform box") {
	HyperBoxShape4D box(Vector4(10, 1, 1, 1));

	Transform4D identity;
	real_t volume_before = box.get_aabb(identity).get_volume();

	Transform4D rotated;
	rotated.basis = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 4.0);
	real_t volume_after = box.get_aabb(rotated).get_volume();

	CHECK(volume_after > volume_before);
}

TEST_CASE("AABB center matches transform origin") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));
	Vector4 offset(10, -5, 7, -3);
	Transform4D t = Transform4D::translated(offset);
	AABB4D aabb = box.get_aabb(t);

	Vector4 aabb_center = aabb.position + aabb.size * 0.5;
	CHECK(aabb_center.is_equal_approx(offset));
}

// ============================================================================
// Accessor Tests
// ============================================================================

TEST_CASE("Set and get half extents") {
	HyperBoxShape4D box(Vector4(1, 1, 1, 1));
	CHECK(box.get_half_extents().is_equal_approx(Vector4(1, 1, 1, 1)));

	box.set_half_extents(Vector4(5, 10, 15, 20));
	CHECK(box.get_half_extents().is_equal_approx(Vector4(5, 10, 15, 20)));

	// Verify it affects support function
	Vector4 s = box.get_support(Vector4(1, 1, 1, 1));
	CHECK(s.is_equal_approx(Vector4(5, 10, 15, 20)));
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_CASE("Support point is always contained") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(1, 1, 1, 1),
		Vector4(-1, 2, -3, 4),
		Vector4(5, -5, 5, -5),
		Vector4(0, 0, 0, 0) // Zero direction
	};

	for (const auto& dir : directions) {
		Vector4 support = box.get_support(dir);
		CHECK(box.contains_point(support));
	}
}

TEST_CASE("AABB contains box center") {
	HyperBoxShape4D box(Vector4(3, 3, 3, 3));
	Vector4 center(10, -5, 3, 8);
	Transform4D t = Transform4D::translated(center);
	AABB4D aabb = box.get_aabb(t);

	CHECK(aabb.has_point(center));
}

TEST_CASE("AABB contains all support points") {
	HyperBoxShape4D box(Vector4(2, 3, 4, 5));
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = box.get_aabb(t);

	// Test support points in all axis directions
	Vector4 axes[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1)
	};

	for (const auto& axis : axes) {
		Vector4 local_support = box.get_support(axis);
		Vector4 world_support = t.xform(local_support);
		CHECK(aabb.has_point(world_support));
	}
}

TEST_CASE("All 16 vertices produce unique support points") {
	HyperBoxShape4D box(Vector4(1, 2, 3, 4));

	// For each of the 16 vertices, there exists a direction that selects it
	for (int i = 0; i < 16; i++) {
		Vector4 vertex(
			(i & 1) ? 1.0 : -1.0,
			(i & 2) ? 2.0 : -2.0,
			(i & 4) ? 3.0 : -3.0,
			(i & 8) ? 4.0 : -4.0
		);

		// Direction matching the vertex signs
		Vector4 dir(
			(i & 1) ? 1.0 : -1.0,
			(i & 2) ? 1.0 : -1.0,
			(i & 4) ? 1.0 : -1.0,
			(i & 8) ? 1.0 : -1.0
		);

		Vector4 support = box.get_support(dir);
		CHECK(support.is_equal_approx(vertex));
	}
}

} // TEST_SUITE
