#include <doctest/doctest.h>
#include "godot_compat.h"
#include "dynamics/body_state_4d.h"
#include "dynamics/integrator_4d.h"

TEST_SUITE("Integrator4D") {

TEST_CASE("Stationary body stays stationary") {
	BodyState4D body;
	body.mass = 1.0;
	body.position = Vector4(1, 2, 3, 4);
	body.linear_velocity = Vector4(0, 0, 0, 0);

	Integrator4D::step(body, 1.0 / 60.0);
	CHECK(body.position.is_equal_approx(Vector4(1, 2, 3, 4)));
}

TEST_CASE("Constant velocity produces linear motion") {
	BodyState4D body;
	body.mass = 1.0;
	body.position = Vector4(0, 0, 0, 0);
	body.linear_velocity = Vector4(1, 0, 0, 0);

	real_t dt = 1.0; // one second
	Integrator4D::step(body, dt);

	CHECK(body.position.is_equal_approx(Vector4(1, 0, 0, 0)));
}

TEST_CASE("Constant force produces acceleration") {
	BodyState4D body;
	body.mass = 2.0;
	body.position = Vector4(0, 0, 0, 0);
	body.linear_velocity = Vector4(0, 0, 0, 0);
	body.force = Vector4(10, 0, 0, 0); // F = 10 → a = 5

	real_t dt = 1.0;
	Integrator4D::step(body, dt);

	// Semi-implicit Euler: v += a*dt = 5, then x += v*dt = 5.
	CHECK(body.linear_velocity.is_equal_approx(Vector4(5, 0, 0, 0)));
	CHECK(body.position.is_equal_approx(Vector4(5, 0, 0, 0)));
}

TEST_CASE("Force accumulator is cleared after step") {
	BodyState4D body;
	body.mass = 1.0;
	body.force = Vector4(100, 0, 0, 0);

	Integrator4D::step(body, 1.0 / 60.0);
	CHECK(body.force.is_equal_approx(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Linear damping reduces velocity") {
	BodyState4D body;
	body.mass = 1.0;
	body.position = Vector4(0, 0, 0, 0);
	body.linear_velocity = Vector4(10, 0, 0, 0);
	body.linear_damp = 0.5;

	real_t dt = 1.0;
	Integrator4D::step(body, dt);

	// After damping, velocity should be less than 10.
	CHECK(body.linear_velocity.length() < 10.0);
	CHECK(body.linear_velocity.x > 0.0); // still positive, just reduced
}

TEST_CASE("Static body (zero inverse mass) does not move under force") {
	BodyState4D body;
	body.mass = 0.0; // static — inverse mass = 0
	body.position = Vector4(5, 5, 5, 5);
	body.force = Vector4(1000, 1000, 1000, 1000);

	Integrator4D::step(body, 1.0);
	CHECK(body.position.is_equal_approx(Vector4(5, 5, 5, 5)));
	CHECK(body.linear_velocity.is_equal_approx(Vector4(0, 0, 0, 0)));
}

TEST_CASE("Motion along W axis works correctly") {
	BodyState4D body;
	body.mass = 1.0;
	body.position = Vector4(0, 0, 0, 0);
	body.linear_velocity = Vector4(0, 0, 0, 3);

	Integrator4D::step(body, 2.0);
	CHECK(body.position.is_equal_approx(Vector4(0, 0, 0, 6)));
}

} // TEST_SUITE
