#include <doctest/doctest.h>
#include "godot_compat.h"
#include "dynamics/gravity_4d.h"

TEST_SUITE("Gravity4D") {

TEST_CASE("Inverse-cube force between two unit masses") {
	// Two unit masses separated by distance 2 along the X axis.
	// F = G * m1 * m2 / r^3  with G = 1 for simplicity.
	Vector4 pos_a(0, 0, 0, 0);
	Vector4 pos_b(2, 0, 0, 0);
	real_t mass_a = 1.0;
	real_t mass_b = 1.0;
	real_t G = 1.0;

	Vector4 force = Gravity4D::compute_force(pos_a, mass_a, pos_b, mass_b, G);
	// Expected magnitude: G * 1 * 1 / 2^3 = 1/8 = 0.125
	CHECK(is_equal_approx(force.length(), 0.125));
	// Direction: from A towards B (positive X).
	CHECK(force.x > 0.0);
	CHECK(is_equal_approx(force.y, 0.0));
	CHECK(is_equal_approx(force.z, 0.0));
	CHECK(is_equal_approx(force.w, 0.0));
}

TEST_CASE("Force direction is from body A towards body B") {
	Vector4 pos_a(0, 0, 0, 0);
	Vector4 pos_b(0, 0, 0, 5);
	Vector4 force = Gravity4D::compute_force(pos_a, 10.0, pos_b, 10.0, 1.0);
	// Force on A due to B should point towards B (positive W).
	CHECK(force.w > 0.0);
}

TEST_CASE("Force is proportional to product of masses") {
	Vector4 pa(0, 0, 0, 0);
	Vector4 pb(1, 0, 0, 0);

	Vector4 f1 = Gravity4D::compute_force(pa, 1.0, pb, 1.0, 1.0);
	Vector4 f2 = Gravity4D::compute_force(pa, 2.0, pb, 3.0, 1.0);
	// f2 should be 6x f1.
	CHECK(is_equal_approx(f2.length(), f1.length() * 6.0));
}

TEST_CASE("Force with softening avoids singularity") {
	Vector4 pa(0, 0, 0, 0);
	Vector4 pb(0.0001, 0, 0, 0); // very close
	real_t softening = 1.0;

	// With softening, force should be finite even at near-zero distance.
	// F = G * m1 * m2 / (r^2 + eps^2)^(3/2)
	Vector4 force = Gravity4D::compute_force_softened(pa, 1.0, pb, 1.0, 1.0, softening);
	CHECK(std::isfinite(force.length()));
	CHECK(force.length() < 10.0); // should be bounded
}

TEST_CASE("Force obeys Newton's third law (equal and opposite)") {
	Vector4 pa(1, 2, 3, 4);
	Vector4 pb(5, 6, 7, 8);
	Vector4 f_ab = Gravity4D::compute_force(pa, 3.0, pb, 7.0, 1.0);
	Vector4 f_ba = Gravity4D::compute_force(pb, 7.0, pa, 3.0, 1.0);
	// f_ab should equal -f_ba.
	CHECK((f_ab + f_ba).is_equal_approx(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Inverse-cube scaling with distance") {
	Vector4 pa(0, 0, 0, 0);
	Vector4 pb1(1, 0, 0, 0);
	Vector4 pb2(2, 0, 0, 0);

	Vector4 f1 = Gravity4D::compute_force(pa, 1.0, pb1, 1.0, 1.0);
	Vector4 f2 = Gravity4D::compute_force(pa, 1.0, pb2, 1.0, 1.0);

	// Distance doubled → force should decrease by factor of 2^3 = 8.
	CHECK(is_equal_approx(f1.length() / f2.length(), 8.0, 0.01));
}

} // TEST_SUITE
