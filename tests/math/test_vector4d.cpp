#include <doctest/doctest.h>
#include "godot_compat.h"
#include "math/vector4d.h"

TEST_SUITE("Vector4D") {

TEST_CASE("Default constructor creates zero vector") {
	Vector4D v;
	CHECK(v.x == 0.0);
	CHECK(v.y == 0.0);
	CHECK(v.z == 0.0);
	CHECK(v.w == 0.0);
}

TEST_CASE("Component constructor") {
	Vector4D v(1.0, 2.0, 3.0, 4.0);
	CHECK(v.x == 1.0);
	CHECK(v.y == 2.0);
	CHECK(v.z == 3.0);
	CHECK(v.w == 4.0);
}

TEST_CASE("Conversion from Vector4") {
	Vector4 gv(5.0, 6.0, 7.0, 8.0);
	Vector4D v(gv);
	CHECK(v.x == 5.0);
	CHECK(v.y == 6.0);
	CHECK(v.z == 7.0);
	CHECK(v.w == 8.0);
}

TEST_CASE("Conversion to Vector4") {
	Vector4D v(1.0, 2.0, 3.0, 4.0);
	Vector4 gv = v;
	CHECK(gv.x == 1.0);
	CHECK(gv.y == 2.0);
	CHECK(gv.z == 3.0);
	CHECK(gv.w == 4.0);
}

TEST_CASE("Subscript operator") {
	Vector4D v(10.0, 20.0, 30.0, 40.0);
	CHECK(v[0] == 10.0);
	CHECK(v[1] == 20.0);
	CHECK(v[2] == 30.0);
	CHECK(v[3] == 40.0);
	v[2] = 99.0;
	CHECK(v.z == 99.0);
}

TEST_CASE("Arithmetic operators") {
	Vector4D a(1, 2, 3, 4);
	Vector4D b(5, 6, 7, 8);

	SUBCASE("Addition") {
		Vector4D c = a + b;
		CHECK(c.is_equal_approx(Vector4D(6, 8, 10, 12)));
	}
	SUBCASE("Subtraction") {
		Vector4D c = a - b;
		CHECK(c.is_equal_approx(Vector4D(-4, -4, -4, -4)));
	}
	SUBCASE("Scalar multiply") {
		Vector4D c = a * 2.0;
		CHECK(c.is_equal_approx(Vector4D(2, 4, 6, 8)));
	}
	SUBCASE("Scalar divide") {
		Vector4D c = a / 2.0;
		CHECK(c.is_equal_approx(Vector4D(0.5, 1.0, 1.5, 2.0)));
	}
	SUBCASE("Negation") {
		Vector4D c = -a;
		CHECK(c.is_equal_approx(Vector4D(-1, -2, -3, -4)));
	}
	SUBCASE("Left scalar multiply") {
		Vector4D c = 3.0 * a;
		CHECK(c.is_equal_approx(Vector4D(3, 6, 9, 12)));
	}
	SUBCASE("Plus-equals") {
		Vector4D c = a;
		c += b;
		CHECK(c.is_equal_approx(Vector4D(6, 8, 10, 12)));
	}
	SUBCASE("Minus-equals") {
		Vector4D c = a;
		c -= b;
		CHECK(c.is_equal_approx(Vector4D(-4, -4, -4, -4)));
	}
	SUBCASE("Times-equals") {
		Vector4D c = a;
		c *= 2.0;
		CHECK(c.is_equal_approx(Vector4D(2, 4, 6, 8)));
	}
	SUBCASE("Divide-equals") {
		Vector4D c = a;
		c /= 2.0;
		CHECK(c.is_equal_approx(Vector4D(0.5, 1.0, 1.5, 2.0)));
	}
}

TEST_CASE("Equality operators") {
	Vector4D a(1, 2, 3, 4);
	Vector4D b(1, 2, 3, 4);
	Vector4D c(1, 2, 3, 5);
	CHECK(a == b);
	CHECK(a != c);
}

TEST_CASE("Dot product") {
	Vector4D a(1, 2, 3, 4);
	Vector4D b(5, 6, 7, 8);
	// 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
	CHECK(is_equal_approx(a.dot(b), 70.0));
}

TEST_CASE("Length") {
	Vector4D v(1, 0, 0, 0);
	CHECK(is_equal_approx(v.length(), 1.0));

	Vector4D v2(1, 1, 1, 1);
	CHECK(is_equal_approx(v2.length(), 2.0));

	CHECK(is_equal_approx(v2.length_squared(), 4.0));
}

TEST_CASE("Normalized") {
	Vector4D v(3, 0, 0, 0);
	Vector4D n = v.normalized();
	CHECK(n.is_equal_approx(Vector4D(1, 0, 0, 0)));
	CHECK(n.is_normalized());

	// Zero vector normalizes to zero.
	Vector4D zero;
	CHECK(zero.normalized().is_zero_approx());
}

TEST_CASE("Normalize in-place") {
	Vector4D v(0, 4, 0, 0);
	v.normalize();
	CHECK(v.is_equal_approx(Vector4D(0, 1, 0, 0)));
}

TEST_CASE("Safe normalized with fallback") {
	Vector4D zero;
	Vector4D fallback(0, 0, 0, 1);
	CHECK(zero.safe_normalized(fallback).is_equal_approx(fallback));

	Vector4D v(2, 0, 0, 0);
	CHECK(v.safe_normalized(fallback).is_equal_approx(Vector4D(1, 0, 0, 0)));
}

TEST_CASE("Distance") {
	Vector4D a(1, 0, 0, 0);
	Vector4D b(0, 0, 0, 0);
	CHECK(is_equal_approx(a.distance_to(b), 1.0));
	CHECK(is_equal_approx(a.distance_squared_to(b), 1.0));
}

TEST_CASE("Direction to") {
	Vector4D a(0, 0, 0, 0);
	Vector4D b(3, 0, 0, 0);
	CHECK(a.direction_to(b).is_equal_approx(Vector4D(1, 0, 0, 0)));
}

TEST_CASE("Component-wise operations") {
	Vector4D a(2, 3, 4, 5);
	Vector4D b(10, 20, 30, 40);

	SUBCASE("Component multiply") {
		CHECK(a.component_mul(b).is_equal_approx(Vector4D(20, 60, 120, 200)));
	}
	SUBCASE("Component divide") {
		CHECK(b.component_div(a).is_equal_approx(Vector4D(5, 20.0 / 3.0, 7.5, 8)));
	}
	SUBCASE("Abs") {
		Vector4D v(-1, 2, -3, 4);
		CHECK(v.abs().is_equal_approx(Vector4D(1, 2, 3, 4)));
	}
	SUBCASE("Sign") {
		Vector4D v(-5, 0, 3, -0.1);
		CHECK(v.sign().is_equal_approx(Vector4D(-1, 0, 1, -1)));
	}
	SUBCASE("Floor") {
		Vector4D v(1.7, -1.3, 2.0, 0.9);
		CHECK(v.floor().is_equal_approx(Vector4D(1, -2, 2, 0)));
	}
	SUBCASE("Ceil") {
		Vector4D v(1.1, -1.9, 2.0, 0.1);
		CHECK(v.ceil().is_equal_approx(Vector4D(2, -1, 2, 1)));
	}
	SUBCASE("Round") {
		Vector4D v(1.4, 1.5, -0.6, 2.0);
		CHECK(v.round().is_equal_approx(Vector4D(1, 2, -1, 2)));
	}
	SUBCASE("Clamp") {
		Vector4D v(5, -3, 0.5, 10);
		Vector4D lo(0, 0, 0, 0);
		Vector4D hi(2, 2, 2, 2);
		CHECK(v.clamp(lo, hi).is_equal_approx(Vector4D(2, 0, 0.5, 2)));
	}
	SUBCASE("Snapped") {
		Vector4D v(1.7, 2.3, -0.8, 5.5);
		Vector4D step(0.5, 1.0, 0.25, 2.0);
		Vector4D s = v.snapped(step);
		CHECK(is_equal_approx(s.x, 1.5));
		CHECK(is_equal_approx(s.y, 2.0));
		CHECK(is_equal_approx(s.z, -0.75));
		CHECK(is_equal_approx(s.w, 6.0));
	}
}

TEST_CASE("Lerp") {
	Vector4D a(0, 0, 0, 0);
	Vector4D b(4, 8, 12, 16);
	CHECK(a.lerp(b, 0.25).is_equal_approx(Vector4D(1, 2, 3, 4)));
	CHECK(a.lerp(b, 0.0).is_equal_approx(a));
	CHECK(a.lerp(b, 1.0).is_equal_approx(b));
}

TEST_CASE("Move toward") {
	Vector4D a(0, 0, 0, 0);
	Vector4D b(4, 0, 0, 0);
	CHECK(a.move_toward(b, 1.0).is_equal_approx(Vector4D(1, 0, 0, 0)));
	CHECK(a.move_toward(b, 10.0).is_equal_approx(b)); // Overshoots -> clamp to target.
}

TEST_CASE("Axis index queries") {
	Vector4D v(3, 1, 5, 2);
	CHECK(v.min_axis_index() == 1);
	CHECK(v.max_axis_index() == 2);
}

TEST_CASE("is_finite") {
	Vector4D v(1, 2, 3, 4);
	CHECK(v.is_finite());
}

TEST_CASE("Named constants") {
	CHECK(Vector4D::ZERO.is_zero_approx());
	CHECK(Vector4D::ONE.is_equal_approx(Vector4D(1, 1, 1, 1)));
	CHECK(Vector4D::AXIS_X.is_equal_approx(Vector4D(1, 0, 0, 0)));
	CHECK(Vector4D::AXIS_Y.is_equal_approx(Vector4D(0, 1, 0, 0)));
	CHECK(Vector4D::AXIS_Z.is_equal_approx(Vector4D(0, 0, 1, 0)));
	CHECK(Vector4D::AXIS_W.is_equal_approx(Vector4D(0, 0, 0, 1)));
}

// ---- 4D-specific operations -----------------------------------------------

TEST_CASE("Triple cross product is perpendicular to all inputs") {
	Vector4D u(1, 0, 0, 0);
	Vector4D v(0, 1, 0, 0);
	Vector4D w(0, 0, 1, 0);
	Vector4D result = Vector4D::triple_cross(u, v, w);

	CHECK(is_equal_approx(result.dot(u), 0.0));
	CHECK(is_equal_approx(result.dot(v), 0.0));
	CHECK(is_equal_approx(result.dot(w), 0.0));

	// For the standard basis, triple_cross(e_x, e_y, e_z) should be +-e_w.
	bool is_plus_or_minus_ew = result.is_equal_approx(Vector4D(0, 0, 0, -1)) ||
			result.is_equal_approx(Vector4D(0, 0, 0, 1));
	CHECK(is_plus_or_minus_ew);
}

TEST_CASE("Triple cross product with arbitrary vectors") {
	Vector4D u(1, 2, 0, 1);
	Vector4D v(0, 1, 3, 0);
	Vector4D w(2, 0, 1, 1);
	Vector4D result = Vector4D::triple_cross(u, v, w);

	// Must be perpendicular to all three.
	CHECK(std::abs(result.dot(u)) < CMP_EPSILON);
	CHECK(std::abs(result.dot(v)) < CMP_EPSILON);
	CHECK(std::abs(result.dot(w)) < CMP_EPSILON);

	// Must be non-zero (inputs are linearly independent).
	CHECK(result.length() > CMP_EPSILON);
}

TEST_CASE("Triple cross product of coplanar vectors is zero") {
	Vector4D u(1, 0, 0, 0);
	Vector4D v(0, 1, 0, 0);
	Vector4D w(1, 1, 0, 0); // Linear combination of u and v.
	Vector4D result = Vector4D::triple_cross(u, v, w);
	CHECK(result.is_zero_approx());
}

TEST_CASE("Projection") {
	Vector4D v(3, 4, 0, 0);
	Vector4D onto(1, 0, 0, 0);
	Vector4D proj = v.project(onto);
	CHECK(proj.is_equal_approx(Vector4D(3, 0, 0, 0)));

	// Project onto non-unit vector.
	Vector4D onto2(2, 0, 0, 0);
	CHECK(v.project(onto2).is_equal_approx(Vector4D(3, 0, 0, 0)));
}

TEST_CASE("Rejection") {
	Vector4D v(3, 4, 0, 0);
	Vector4D from(1, 0, 0, 0);
	Vector4D rej = v.reject(from);
	CHECK(rej.is_equal_approx(Vector4D(0, 4, 0, 0)));

	// Projection + rejection = original.
	Vector4D sum = v.project(from) + v.reject(from);
	CHECK(sum.is_equal_approx(v));
}

TEST_CASE("Reflection") {
	// Reflect (1,0,0,0) across the hyperplane with normal (1,0,0,0).
	Vector4D v(1, 0, 0, 0);
	Vector4D n(1, 0, 0, 0);
	Vector4D r = v.reflect(n);
	CHECK(r.is_equal_approx(Vector4D(-1, 0, 0, 0)));

	// Reflect a vector with no component along the normal: unchanged.
	Vector4D v2(0, 1, 0, 0);
	CHECK(v2.reflect(n).is_equal_approx(v2));
}

TEST_CASE("Slide") {
	Vector4D v(3, 4, 0, 0);
	Vector4D n(0, 1, 0, 0);
	Vector4D s = v.slide(n);
	CHECK(s.is_equal_approx(Vector4D(3, 0, 0, 0)));
}

TEST_CASE("Angle to") {
	Vector4D a(1, 0, 0, 0);
	Vector4D b(0, 1, 0, 0);
	CHECK(is_equal_approx(a.angle_to(b), MATH_PI / 2.0));

	// Parallel vectors: angle = 0.
	CHECK(is_equal_approx(a.angle_to(a), 0.0));

	// Anti-parallel: angle = pi.
	CHECK(is_equal_approx(a.angle_to(-a), MATH_PI));
}

TEST_CASE("Is perpendicular to") {
	Vector4D a(1, 0, 0, 0);
	Vector4D b(0, 1, 0, 0);
	CHECK(a.is_perpendicular_to(b));
	CHECK(!a.is_perpendicular_to(a));
}

TEST_CASE("Rotated in plane") {
	Vector4D v(1, 0, 0, 0);

	SUBCASE("XY plane 90 degrees") {
		Vector4D r = v.rotated_in_plane(0, 1, MATH_PI / 2.0);
		CHECK(r.is_equal_approx(Vector4D(0, 1, 0, 0)));
	}
	SUBCASE("XW plane 90 degrees") {
		Vector4D r = v.rotated_in_plane(0, 3, MATH_PI / 2.0);
		CHECK(r.is_equal_approx(Vector4D(0, 0, 0, 1)));
	}
	SUBCASE("360 degree rotation returns to original") {
		Vector4D v2(1, 2, 3, 4);
		Vector4D r = v2.rotated_in_plane(1, 2, 2.0 * MATH_PI);
		CHECK(r.is_equal_approx(v2));
	}
	SUBCASE("Components outside the plane are unchanged") {
		Vector4D v2(1, 2, 3, 4);
		Vector4D r = v2.rotated_in_plane(0, 1, MATH_PI / 4.0);
		CHECK(is_equal_approx(r.z, v2.z));
		CHECK(is_equal_approx(r.w, v2.w));
	}
}

TEST_CASE("Find any perpendicular") {
	Vector4D v(1, 2, 3, 4);
	Vector4D perp = v.find_any_perpendicular();

	CHECK(perp.is_normalized());
	CHECK(std::abs(v.dot(perp)) < CMP_EPSILON);
}

TEST_CASE("Find any perpendicular to axis-aligned vectors") {
	CHECK(std::abs(Vector4D::AXIS_X.find_any_perpendicular().dot(Vector4D::AXIS_X)) < CMP_EPSILON);
	CHECK(std::abs(Vector4D::AXIS_Y.find_any_perpendicular().dot(Vector4D::AXIS_Y)) < CMP_EPSILON);
	CHECK(std::abs(Vector4D::AXIS_Z.find_any_perpendicular().dot(Vector4D::AXIS_Z)) < CMP_EPSILON);
	CHECK(std::abs(Vector4D::AXIS_W.find_any_perpendicular().dot(Vector4D::AXIS_W)) < CMP_EPSILON);
}

} // TEST_SUITE
