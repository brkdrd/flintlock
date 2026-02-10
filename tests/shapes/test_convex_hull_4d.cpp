#include <doctest/doctest.h>
#include "godot_compat.h"
#include "shapes/convex_hull_4d.h"
#include "math/basis4d.h"
#include <vector>

TEST_SUITE("ConvexHull4D") {

// Helper function to create a tesseract (hypercube) hull
std::vector<Vector4> make_tesseract(real_t half_extent) {
	std::vector<Vector4> vertices;
	for (int i = 0; i < 16; i++) {
		vertices.push_back(Vector4(
			(i & 1) ? half_extent : -half_extent,
			(i & 2) ? half_extent : -half_extent,
			(i & 4) ? half_extent : -half_extent,
			(i & 8) ? half_extent : -half_extent
		));
	}
	return vertices;
}

// Helper to create a 4D simplex (5 vertices)
std::vector<Vector4> make_simplex() {
	return {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, 0, 1),
		Vector4(0, 0, 0, 0)
	};
}

// ============================================================================
// Construction Tests
// ============================================================================

TEST_CASE("Construct from tesseract vertices") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);

	CHECK(hull.get_vertex_count() == 16);
}

TEST_CASE("Construct from simplex") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);

	CHECK(hull.get_vertex_count() == 5);
}

TEST_CASE("Construct from single point") {
	std::vector<Vector4> vertices = {Vector4(1, 2, 3, 4)};
	ConvexHull4D hull(vertices);

	CHECK(hull.get_vertex_count() == 1);
}

TEST_CASE("Construct from collinear points") {
	// Points along a line
	std::vector<Vector4> vertices = {
		Vector4(0, 0, 0, 0),
		Vector4(1, 0, 0, 0),
		Vector4(2, 0, 0, 0),
		Vector4(3, 0, 0, 0)
	};
	ConvexHull4D hull(vertices);

	// Implementation may reduce to minimal set
	CHECK(hull.get_vertex_count() >= 2); // At least endpoints
}

TEST_CASE("Construct from duplicate vertices") {
	std::vector<Vector4> vertices = {
		Vector4(1, 0, 0, 0),
		Vector4(1, 0, 0, 0), // Duplicate
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0)
	};
	ConvexHull4D hull(vertices);

	// Should remove duplicates
	CHECK(hull.get_vertex_count() <= 4);
}

// ============================================================================
// Support Function Tests
// ============================================================================

TEST_CASE("Support for tesseract matches corner selection") {
	auto vertices = make_tesseract(3.0);
	ConvexHull4D hull(vertices);

	Vector4 s = hull.get_support(Vector4(1, 1, 1, 1));
	CHECK(s.is_equal_approx(Vector4(3, 3, 3, 3)));

	Vector4 s2 = hull.get_support(Vector4(-1, -1, -1, -1));
	CHECK(s2.is_equal_approx(Vector4(-3, -3, -3, -3)));
}

TEST_CASE("Support along single axis") {
	auto vertices = make_tesseract(5.0);
	ConvexHull4D hull(vertices);

	CHECK(hull.get_support(Vector4(1, 0, 0, 0)).x >= 5.0 - CMP_EPSILON);
	CHECK(hull.get_support(Vector4(0, 1, 0, 0)).y >= 5.0 - CMP_EPSILON);
	CHECK(hull.get_support(Vector4(0, 0, 1, 0)).z >= 5.0 - CMP_EPSILON);
	CHECK(hull.get_support(Vector4(0, 0, 0, 1)).w >= 5.0 - CMP_EPSILON);
}

TEST_CASE("Support is maximal in given direction") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);

	Vector4 dir(1, 2, -1, 1);
	Vector4 support = hull.get_support(dir);
	real_t support_dot = support.dot(dir);

	// All vertices should have dot product <= support_dot
	for (const auto& vertex : vertices) {
		CHECK(vertex.dot(dir) <= support_dot + CMP_EPSILON);
	}
}

TEST_CASE("Support with zero direction returns valid vertex") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);

	Vector4 s = hull.get_support(Vector4(0, 0, 0, 0));

	// Should return one of the vertices
	bool found = false;
	for (const auto& vertex : vertices) {
		if (s.is_equal_approx(vertex)) {
			found = true;
			break;
		}
	}
	CHECK(found);
}

TEST_CASE("Support with opposite directions") {
	auto vertices = make_tesseract(4.0);
	ConvexHull4D hull(vertices);

	Vector4 dir(1, 1, 1, 1);
	Vector4 s1 = hull.get_support(dir);
	Vector4 s2 = hull.get_support(-dir);

	// For symmetric hull, should be opposite
	CHECK(s1.is_equal_approx(-s2));
}

TEST_CASE("Support for irregular convex hull") {
	// Non-symmetric vertices
	std::vector<Vector4> vertices = {
		Vector4(5, 0, 0, 0),
		Vector4(0, 3, 0, 0),
		Vector4(0, 0, 2, 0),
		Vector4(0, 0, 0, 1),
		Vector4(-1, -1, -1, -1)
	};
	ConvexHull4D hull(vertices);

	Vector4 s_x = hull.get_support(Vector4(1, 0, 0, 0));
	CHECK(is_equal_approx(s_x.x, 5.0));

	Vector4 s_y = hull.get_support(Vector4(0, 1, 0, 0));
	CHECK(is_equal_approx(s_y.y, 3.0));
}

TEST_CASE("Support returns actual vertex") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(1, 1, 1, 1)
	};

	for (const auto& dir : directions) {
		Vector4 support = hull.get_support(dir);

		// Support should be one of the input vertices
		bool is_vertex = false;
		for (const auto& v : vertices) {
			if (support.is_equal_approx(v)) {
				is_vertex = true;
				break;
			}
		}
		CHECK(is_vertex);
	}
}

// ============================================================================
// Containment Tests
// ============================================================================

TEST_CASE("Contains all input vertices") {
	auto vertices = make_tesseract(3.0);
	ConvexHull4D hull(vertices);

	for (const auto& vertex : vertices) {
		CHECK(hull.contains_point(vertex));
	}
}

TEST_CASE("Contains center of simplex") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);

	// Centroid
	Vector4 center(0.2, 0.2, 0.2, 0.2);
	CHECK(hull.contains_point(center));
}

TEST_CASE("Contains interior point of tesseract") {
	auto vertices = make_tesseract(5.0);
	ConvexHull4D hull(vertices);

	CHECK(hull.contains_point(Vector4(0, 0, 0, 0))); // Center
	CHECK(hull.contains_point(Vector4(2, 2, 2, 2))); // Inside
	CHECK(hull.contains_point(Vector4(-3, 1, -2, 4))); // Inside
}

TEST_CASE("Does not contain points outside tesseract") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);

	CHECK_FALSE(hull.contains_point(Vector4(3, 0, 0, 0)));
	CHECK_FALSE(hull.contains_point(Vector4(0, 0, 0, 5)));
	CHECK_FALSE(hull.contains_point(Vector4(3, 3, 3, 3)));
}

TEST_CASE("Containment for irregular hull") {
	std::vector<Vector4> vertices = {
		Vector4(2, 0, 0, 0),
		Vector4(0, 2, 0, 0),
		Vector4(0, 0, 2, 0),
		Vector4(0, 0, 0, 2),
		Vector4(-1, -1, -1, -1)
	};
	ConvexHull4D hull(vertices);

	// Centroid should be inside
	Vector4 centroid = (vertices[0] + vertices[1] + vertices[2] + vertices[3] + vertices[4]) / 5.0;
	CHECK(hull.contains_point(centroid));

	// Point far outside
	CHECK_FALSE(hull.contains_point(Vector4(10, 10, 10, 10)));
}

TEST_CASE("Boundary containment") {
	auto vertices = make_tesseract(4.0);
	ConvexHull4D hull(vertices);

	// Points on faces should be contained
	CHECK(hull.contains_point(Vector4(4, 0, 0, 0)));
	CHECK(hull.contains_point(Vector4(4, 4, 0, 0)));
	CHECK(hull.contains_point(Vector4(4, 4, 4, 0)));
}

TEST_CASE("Containment after vertex modification") {
	auto vertices = make_tesseract(3.0);
	ConvexHull4D hull(vertices);

	CHECK(hull.contains_point(Vector4(0, 0, 0, 0)));

	// Modify and rebuild
	vertices[0] = Vector4(10, 0, 0, 0);
	hull = ConvexHull4D(vertices);

	// Should still contain center
	CHECK(hull.contains_point(Vector4(0, 0, 0, 0)));
}

// ============================================================================
// AABB Tests
// ============================================================================

TEST_CASE("AABB for tesseract at origin") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);
	Transform4D t; // identity
	AABB4D aabb = hull.get_aabb(t);

	CHECK(aabb.position.is_equal_approx(Vector4(-2, -2, -2, -2)));
	CHECK(aabb.size.is_equal_approx(Vector4(4, 4, 4, 4)));
}

TEST_CASE("AABB for translated hull") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);
	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = hull.get_aabb(t);

	// AABB should be translated
	Vector4 min_vertex(0, 0, 0, 0);
	Vector4 max_vertex(1, 1, 1, 1);
	Vector4 min_transformed = t.xform(min_vertex);
	Vector4 max_transformed = t.xform(max_vertex);

	CHECK(aabb.has_point(min_transformed));
	CHECK(aabb.has_point(max_transformed));
}

TEST_CASE("AABB grows after rotation") {
	auto vertices = {
		Vector4(5, 0, 0, 0),
		Vector4(-5, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1),
		Vector4(0, 0, 0, -1)
	};
	ConvexHull4D hull(vertices);

	Transform4D identity;
	real_t volume_before = hull.get_aabb(identity).get_volume();

	Transform4D rotated;
	rotated.basis = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 4.0);
	real_t volume_after = hull.get_aabb(rotated).get_volume();

	CHECK(volume_after >= volume_before - CMP_EPSILON);
}

TEST_CASE("AABB contains all transformed vertices") {
	auto vertices = make_tesseract(3.0);
	ConvexHull4D hull(vertices);

	Transform4D t;
	t.basis = Basis4D::from_rotation(Basis4D::PLANE_ZW, 0.8);
	t.origin = Vector4(5, -2, 10, 3);

	AABB4D aabb = hull.get_aabb(t);

	for (const auto& vertex : vertices) {
		Vector4 transformed = t.xform(vertex);
		CHECK(aabb.has_point(transformed));
	}
}

TEST_CASE("AABB with non-uniform scaling") {
	auto vertices = make_tesseract(1.0);
	ConvexHull4D hull(vertices);

	Transform4D t;
	t.basis = Basis4D::from_scale(Vector4(2, 3, 4, 5));
	AABB4D aabb = hull.get_aabb(t);

	CHECK(aabb.size.x >= 4.0 - CMP_EPSILON);
	CHECK(aabb.size.y >= 6.0 - CMP_EPSILON);
	CHECK(aabb.size.z >= 8.0 - CMP_EPSILON);
	CHECK(aabb.size.w >= 10.0 - CMP_EPSILON);
}

TEST_CASE("AABB center matches transform origin for symmetric hull") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);

	Vector4 offset(7, -2, 13, 5);
	Transform4D t = Transform4D::translated(offset);
	AABB4D aabb = hull.get_aabb(t);

	Vector4 aabb_center = aabb.position + aabb.size * 0.5;
	CHECK(aabb_center.is_equal_approx(offset));
}

// ============================================================================
// Accessor Tests
// ============================================================================

TEST_CASE("Get vertex count") {
	auto vertices = make_tesseract(1.0);
	ConvexHull4D hull(vertices);

	CHECK(hull.get_vertex_count() == 16);
}

TEST_CASE("Get vertex by index") {
	std::vector<Vector4> vertices = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, 0, 1)
	};
	ConvexHull4D hull(vertices);

	for (int i = 0; i < hull.get_vertex_count(); i++) {
		Vector4 v = hull.get_vertex(i);

		// Should match one of the input vertices
		bool found = false;
		for (const auto& input : vertices) {
			if (v.is_equal_approx(input)) {
				found = true;
				break;
			}
		}
		CHECK(found);
	}
}

TEST_CASE("Set vertices updates hull") {
	auto vertices1 = make_simplex();
	ConvexHull4D hull(vertices1);

	CHECK(hull.get_vertex_count() == 5);

	auto vertices2 = make_tesseract(1.0);
	hull.set_vertices(vertices2);

	CHECK(hull.get_vertex_count() == 16);
}

// ============================================================================
// Consistency Tests
// ============================================================================

TEST_CASE("Support point is always contained") {
	auto vertices = make_tesseract(2.0);
	ConvexHull4D hull(vertices);

	Vector4 directions[] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(1, 1, 1, 1),
		Vector4(-1, 2, -3, 4),
		Vector4(5, -5, 5, -5)
	};

	for (const auto& dir : directions) {
		Vector4 support = hull.get_support(dir);
		CHECK(hull.contains_point(support));
	}
}

TEST_CASE("AABB contains hull center") {
	auto vertices = make_tesseract(3.0);
	ConvexHull4D hull(vertices);

	Vector4 center(10, -5, 3, 8);
	Transform4D t = Transform4D::translated(center);
	AABB4D aabb = hull.get_aabb(t);

	CHECK(aabb.has_point(center));
}

TEST_CASE("AABB contains all support points") {
	auto vertices = make_simplex();
	ConvexHull4D hull(vertices);

	Transform4D t = Transform4D::translated(Vector4(10, 20, 30, 40));
	AABB4D aabb = hull.get_aabb(t);

	Vector4 axes[] = {
		Vector4(1, 0, 0, 0), Vector4(-1, 0, 0, 0),
		Vector4(0, 1, 0, 0), Vector4(0, -1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, -1, 0),
		Vector4(0, 0, 0, 1), Vector4(0, 0, 0, -1)
	};

	for (const auto& axis : axes) {
		Vector4 local_support = hull.get_support(axis);
		Vector4 world_support = t.xform(local_support);
		CHECK(aabb.has_point(world_support));
	}
}

TEST_CASE("Empty hull returns reasonable defaults") {
	std::vector<Vector4> empty;
	ConvexHull4D hull(empty);

	CHECK(hull.get_vertex_count() == 0);

	// Support should return zero or handle gracefully
	Vector4 s = hull.get_support(Vector4(1, 0, 0, 0));
	CHECK(s.length() < CMP_EPSILON);
}

TEST_CASE("Convex hull is actually convex") {
	auto vertices = make_tesseract(5.0);
	ConvexHull4D hull(vertices);

	// Sample random points on edges and faces
	// If hull is convex, all such points should be contained
	Vector4 v1 = vertices[0];
	Vector4 v2 = vertices[15];

	// Point on edge between v1 and v2
	for (real_t t = 0.0; t <= 1.0; t += 0.1) {
		Vector4 edge_point = v1 * (1.0 - t) + v2 * t;
		CHECK(hull.contains_point(edge_point));
	}
}

} // TEST_SUITE
