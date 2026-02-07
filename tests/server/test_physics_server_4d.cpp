#include <doctest/doctest.h>
#include "godot_compat.h"
#include "server/physics_server_4d.h"

// These tests exercise the server API at the C++ level.  Full GDScript
// integration tests live in the demo project.

TEST_SUITE("PhysicsServer4D") {

TEST_CASE("Create and free a space") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	CHECK(space.is_valid());

	server.space_set_active(space, true);
	server.free_rid(space);
}

TEST_CASE("Create a body with a sphere shape") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	RID shape = server.shape_create(PhysicsServer4D::SHAPE_HYPER_SPHERE);
	server.shape_set_data(shape, 2.0); // radius

	server.body_add_shape(body, shape, Transform4D());

	server.free_rid(body);
	server.free_rid(shape);
	server.free_rid(space);
}

TEST_CASE("Body at rest stays at rest after stepping") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	Transform4D initial;
	initial.origin = Vector4(0, 0, 0, 0);
	server.body_set_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM, initial);

	server.step(1.0 / 60.0);

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(result.origin.is_equal_approx(Vector4(0, 0, 0, 0)));

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("Body with velocity moves after stepping") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(60, 0, 0, 0));

	server.step(1.0); // 1 second

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	// Should have moved ~60 units along X.
	CHECK(result.origin.x > 59.0);

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("Gravity between two bodies") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body_a = server.body_create();
	server.body_set_space(body_a, space);
	server.body_set_mode(body_a, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D ta;
	ta.origin = Vector4(-5, 0, 0, 0);
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);

	RID body_b = server.body_create();
	server.body_set_space(body_b, space);
	server.body_set_mode(body_b, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_b, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D tb;
	tb.origin = Vector4(5, 0, 0, 0);
	server.body_set_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	// Step several frames.
	for (int i = 0; i < 60; i++) {
		server.step(1.0 / 60.0);
	}

	// Bodies should have moved towards each other.
	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rb = server.body_get_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM);
	real_t initial_dist = 10.0;
	real_t final_dist = (ra.origin - rb.origin).length();
	CHECK(final_dist < initial_dist);

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(space);
}

TEST_CASE("Apply impulse changes velocity") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 1.0);

	server.body_apply_impulse(body, Vector4(0, 0, 0, 10), Vector4());

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(vel.is_equal_approx(Vector4(0, 0, 0, 10)));

	server.free_rid(body);
	server.free_rid(space);
}

} // TEST_SUITE
