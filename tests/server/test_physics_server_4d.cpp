#include <doctest/doctest.h>
#include "godot_compat.h"
#include "server/physics_server_4d.h"

// These tests exercise the server API at the C++ level.  Full GDScript
// integration tests live in the demo project.

TEST_SUITE("PhysicsServer4D") {

// ============================================================================
// SECTION: Basic Resource Management
// ============================================================================

TEST_CASE("Create and free a space") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	CHECK(space.is_valid());

	server.space_set_active(space, true);
	server.free_rid(space);
}

TEST_CASE("Create multiple spaces") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space1 = server.space_create();
	RID space2 = server.space_create();
	RID space3 = server.space_create();

	CHECK(space1.is_valid());
	CHECK(space2.is_valid());
	CHECK(space3.is_valid());
	CHECK(space1 != space2);
	CHECK(space2 != space3);

	server.free_rid(space1);
	server.free_rid(space2);
	server.free_rid(space3);
}

TEST_CASE("Create and free a body") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID body = server.body_create();
	CHECK(body.is_valid());
	server.free_rid(body);
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

TEST_CASE("Create body with box shape") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID shape = server.shape_create(PhysicsServer4D::SHAPE_HYPER_BOX);
	server.shape_set_data(shape, Vector4(1, 2, 3, 4));

	RID body = server.body_create();
	server.body_add_shape(body, shape, Transform4D());

	server.free_rid(body);
	server.free_rid(shape);
}

TEST_CASE("Create body with multiple shapes") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();

	RID body = server.body_create();

	RID sphere = server.shape_create(PhysicsServer4D::SHAPE_HYPER_SPHERE);
	server.shape_set_data(sphere, 1.0);

	RID box = server.shape_create(PhysicsServer4D::SHAPE_HYPER_BOX);
	server.shape_set_data(box, Vector4(0.5, 0.5, 0.5, 0.5));

	Transform4D t1, t2;
	t1.origin = Vector4(0, 0, 0, 0);
	t2.origin = Vector4(1, 0, 0, 0);

	server.body_add_shape(body, sphere, t1);
	server.body_add_shape(body, box, t2);

	server.free_rid(body);
	server.free_rid(sphere);
	server.free_rid(box);
}

// ============================================================================
// SECTION: Body State Management
// ============================================================================

TEST_CASE("Set and get body transform") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID body = server.body_create();

	Transform4D t;
	t.origin = Vector4(1, 2, 3, 4);
	server.body_set_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM, t);

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(result.origin.is_equal_approx(Vector4(1, 2, 3, 4)));

	server.free_rid(body);
}

TEST_CASE("Set and get body velocity") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID body = server.body_create();

	Vector4 vel(10, 20, 30, 40);
	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, vel);

	Vector4 result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(result.is_equal_approx(Vector4(10, 20, 30, 40)));

	server.free_rid(body);
}

TEST_CASE("Set and get body mass") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID body = server.body_create();

	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 42.0);
	real_t mass = server.body_get_param(body, PhysicsServer4D::BODY_PARAM_MASS);

	CHECK(is_equal_approx(mass, 42.0));

	server.free_rid(body);
}

TEST_CASE("Set and get body damping") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID body = server.body_create();

	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, 0.5);
	real_t damp = server.body_get_param(body, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP);

	CHECK(is_equal_approx(damp, 0.5));

	server.free_rid(body);
}

// ============================================================================
// SECTION: Integration and Motion
// ============================================================================

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

TEST_CASE("Body moves in all 4 dimensions") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(1, 2, 3, 4));

	server.step(10.0);

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(is_equal_approx(result.origin.x, 10.0));
	CHECK(is_equal_approx(result.origin.y, 20.0));
	CHECK(is_equal_approx(result.origin.z, 30.0));
	CHECK(is_equal_approx(result.origin.w, 40.0));

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("Static body does not move") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_STATIC);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 0.0);

	Transform4D initial;
	initial.origin = Vector4(5, 5, 5, 5);
	server.body_set_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM, initial);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(100, 100, 100, 100));

	server.step(1.0);

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(result.origin.is_equal_approx(Vector4(5, 5, 5, 5)));

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("Inactive space does not simulate") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, false); // Inactive!

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(100, 0, 0, 0));

	server.step(1.0);

	Transform4D result = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	// Should not have moved (space inactive)
	CHECK(result.origin.is_equal_approx(Vector4(0, 0, 0, 0)));

	server.free_rid(body);
	server.free_rid(space);
}

// ============================================================================
// SECTION: Forces and Impulses
// ============================================================================

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

TEST_CASE("Impulse respects mass") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();

	RID body = server.body_create();
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 2.0);

	// Impulse = 20, Mass = 2, so ΔV = 10
	server.body_apply_impulse(body, Vector4(20, 0, 0, 0), Vector4());

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(is_equal_approx(vel.x, 10.0));

	server.free_rid(body);
}

TEST_CASE("Multiple impulses accumulate") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();

	RID body = server.body_create();
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 1.0);

	server.body_apply_impulse(body, Vector4(5, 0, 0, 0), Vector4());
	server.body_apply_impulse(body, Vector4(3, 0, 0, 0), Vector4());
	server.body_apply_impulse(body, Vector4(0, 2, 0, 0), Vector4());

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(is_equal_approx(vel.x, 8.0));
	CHECK(is_equal_approx(vel.y, 2.0));

	server.free_rid(body);
}

TEST_CASE("Force application integrates over time") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 1.0);

	// Apply force F=10, mass=1, so a=10
	// After 1 second: v = a*t = 10
	server.body_apply_force(body, Vector4(10, 0, 0, 0), Vector4());
	server.step(1.0);

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(is_equal_approx(vel.x, 10.0, 0.01));

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("Forces are cleared after step") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_MASS, 1.0);

	server.body_apply_force(body, Vector4(100, 0, 0, 0), Vector4());
	server.step(0.1);

	Vector4 vel1 = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);

	// Step again without applying new force - velocity should not increase
	server.step(0.1);

	Vector4 vel2 = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(is_equal_approx(vel1.x, vel2.x, 0.01));

	server.free_rid(body);
	server.free_rid(space);
}

// ============================================================================
// SECTION: Damping
// ============================================================================

TEST_CASE("Linear damping reduces velocity") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, 0.5);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(100, 0, 0, 0));

	server.step(1.0);

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	// With damp=0.5, after 1s: v *= (1-0.5*1) = 0.5, so v = 50
	CHECK(vel.x < 100.0);
	CHECK(vel.x > 0.0);

	server.free_rid(body);
	server.free_rid(space);
}

TEST_CASE("High damping stops body quickly") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, 10.0);

	server.body_set_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(100, 0, 0, 0));

	// With high damping, velocity should drop significantly
	for (int i = 0; i < 10; i++) {
		server.step(0.1);
	}

	Vector4 vel = server.body_get_state(body, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(vel.length() < 10.0);

	server.free_rid(body);
	server.free_rid(space);
}

// ============================================================================
// SECTION: Gravity Simulation
// ============================================================================

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

TEST_CASE("Gravity is symmetric (Newton's third law)") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body_a = server.body_create();
	server.body_set_space(body_a, space);
	server.body_set_mode(body_a, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_MASS, 500.0);
	Transform4D ta;
	ta.origin = Vector4(-10, 0, 0, 0);
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);

	RID body_b = server.body_create();
	server.body_set_space(body_b, space);
	server.body_set_mode(body_b, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_b, PhysicsServer4D::BODY_PARAM_MASS, 500.0);
	Transform4D tb;
	tb.origin = Vector4(10, 0, 0, 0);
	server.body_set_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	for (int i = 0; i < 100; i++) {
		server.step(1.0 / 60.0);
	}

	// Equal mass, symmetric positions → should meet in the middle
	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rb = server.body_get_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM);

	// Center of mass should remain at origin
	Vector4 center_of_mass = (ra.origin + rb.origin) * 0.5;
	CHECK(center_of_mass.length() < 0.1);

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(space);
}

TEST_CASE("More massive body accelerates less") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	// Heavy body
	RID heavy = server.body_create();
	server.body_set_space(heavy, space);
	server.body_set_mode(heavy, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(heavy, PhysicsServer4D::BODY_PARAM_MASS, 10000.0);
	Transform4D th;
	th.origin = Vector4(-10, 0, 0, 0);
	server.body_set_state(heavy, PhysicsServer4D::BODY_STATE_TRANSFORM, th);

	// Light body
	RID light = server.body_create();
	server.body_set_space(light, space);
	server.body_set_mode(light, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(light, PhysicsServer4D::BODY_PARAM_MASS, 100.0);
	Transform4D tl;
	tl.origin = Vector4(10, 0, 0, 0);
	server.body_set_state(light, PhysicsServer4D::BODY_STATE_TRANSFORM, tl);

	Vector4 initial_heavy = th.origin;
	Vector4 initial_light = tl.origin;

	for (int i = 0; i < 100; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D rh = server.body_get_state(heavy, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rl = server.body_get_state(light, PhysicsServer4D::BODY_STATE_TRANSFORM);

	real_t heavy_moved = (rh.origin - initial_heavy).length();
	real_t light_moved = (rl.origin - initial_light).length();

	// Light body should move much more than heavy body
	CHECK(light_moved > heavy_moved * 10.0);

	server.free_rid(heavy);
	server.free_rid(light);
	server.free_rid(space);
}

TEST_CASE("Three-body gravity interaction") {
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

	RID body_c = server.body_create();
	server.body_set_space(body_c, space);
	server.body_set_mode(body_c, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_c, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D tc;
	tc.origin = Vector4(0, 5, 0, 0);
	server.body_set_state(body_c, PhysicsServer4D::BODY_STATE_TRANSFORM, tc);

	Vector4 initial_com = (ta.origin + tb.origin + tc.origin) / 3.0;

	for (int i = 0; i < 50; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rb = server.body_get_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rc = server.body_get_state(body_c, PhysicsServer4D::BODY_STATE_TRANSFORM);

	// Center of mass should be conserved
	Vector4 final_com = (ra.origin + rb.origin + rc.origin) / 3.0;
	CHECK((initial_com - final_com).length() < 0.5);

	// All bodies should have moved (influenced by all others)
	CHECK((ra.origin - ta.origin).length() > 0.01);
	CHECK((rb.origin - tb.origin).length() > 0.01);
	CHECK((rc.origin - tc.origin).length() > 0.01);

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(body_c);
	server.free_rid(space);
}

TEST_CASE("Gravity along W axis") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body_a = server.body_create();
	server.body_set_space(body_a, space);
	server.body_set_mode(body_a, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D ta;
	ta.origin = Vector4(0, 0, 0, -5);
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);

	RID body_b = server.body_create();
	server.body_set_space(body_b, space);
	server.body_set_mode(body_b, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_b, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D tb;
	tb.origin = Vector4(0, 0, 0, 5);
	server.body_set_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	for (int i = 0; i < 60; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D rb = server.body_get_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM);

	// Bodies should move towards each other along W
	real_t initial_w_dist = 10.0;
	real_t final_w_dist = std::abs(ra.origin.w - rb.origin.w);
	CHECK(final_w_dist < initial_w_dist);

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(space);
}

// ============================================================================
// SECTION: Space Isolation
// ============================================================================

TEST_CASE("Bodies in different spaces do not interact") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space1 = server.space_create();
	RID space2 = server.space_create();
	server.space_set_active(space1, true);
	server.space_set_active(space2, true);

	// Body in space 1
	RID body1 = server.body_create();
	server.body_set_space(body1, space1);
	server.body_set_mode(body1, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body1, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D t1;
	t1.origin = Vector4(-1, 0, 0, 0);
	server.body_set_state(body1, PhysicsServer4D::BODY_STATE_TRANSFORM, t1);

	// Body in space 2
	RID body2 = server.body_create();
	server.body_set_space(body2, space2);
	server.body_set_mode(body2, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body2, PhysicsServer4D::BODY_PARAM_MASS, 1000.0);
	Transform4D t2;
	t2.origin = Vector4(1, 0, 0, 0);
	server.body_set_state(body2, PhysicsServer4D::BODY_STATE_TRANSFORM, t2);

	for (int i = 0; i < 60; i++) {
		server.step(1.0 / 60.0);
	}

	// Bodies should not have moved (no gravity between different spaces)
	Transform4D r1 = server.body_get_state(body1, PhysicsServer4D::BODY_STATE_TRANSFORM);
	Transform4D r2 = server.body_get_state(body2, PhysicsServer4D::BODY_STATE_TRANSFORM);

	CHECK(r1.origin.is_equal_approx(Vector4(-1, 0, 0, 0), 0.01));
	CHECK(r2.origin.is_equal_approx(Vector4(1, 0, 0, 0), 0.01));

	server.free_rid(body1);
	server.free_rid(body2);
	server.free_rid(space1);
	server.free_rid(space2);
}

TEST_CASE("Move body between spaces") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space1 = server.space_create();
	RID space2 = server.space_create();
	server.space_set_active(space1, true);
	server.space_set_active(space2, true);

	RID body = server.body_create();
	server.body_set_space(body, space1);
	server.body_set_mode(body, PhysicsServer4D::BODY_MODE_RIGID);

	// Create a heavy attractor in space2
	RID attractor = server.body_create();
	server.body_set_space(attractor, space2);
	server.body_set_mode(attractor, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(attractor, PhysicsServer4D::BODY_PARAM_MASS, 100000.0);

	Transform4D tb;
	tb.origin = Vector4(0, 0, 0, 0);
	server.body_set_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	Transform4D ta;
	ta.origin = Vector4(5, 0, 0, 0);
	server.body_set_state(attractor, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);

	// Step - body should not be affected by attractor (different space)
	for (int i = 0; i < 30; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D r1 = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(r1.origin.is_equal_approx(Vector4(0, 0, 0, 0), 0.01));

	// Move body to space2
	server.body_set_space(body, space2);

	// Now it should be affected by gravity
	for (int i = 0; i < 30; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D r2 = server.body_get_state(body, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(r2.origin.x > 0.1); // Should have moved towards attractor

	server.free_rid(body);
	server.free_rid(attractor);
	server.free_rid(space1);
	server.free_rid(space2);
}

// ============================================================================
// SECTION: Combined Effects
// ============================================================================

TEST_CASE("Gravity plus initial velocity") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body_a = server.body_create();
	server.body_set_space(body_a, space);
	server.body_set_mode(body_a, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_MASS, 100.0);
	Transform4D ta;
	ta.origin = Vector4(0, 0, 0, 0);
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);
	// Give it initial velocity away from B
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
			Vector4(-10, 0, 0, 0));

	RID body_b = server.body_create();
	server.body_set_space(body_b, space);
	server.body_set_mode(body_b, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_b, PhysicsServer4D::BODY_PARAM_MASS, 10000.0);
	Transform4D tb;
	tb.origin = Vector4(10, 0, 0, 0);
	server.body_set_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	// Step simulation
	for (int i = 0; i < 120; i++) {
		server.step(1.0 / 60.0);
	}

	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);

	// Body A should have moved left initially, but gravity should eventually pull it back
	// At minimum, velocity should have changed direction
	Vector4 vel = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
	CHECK(vel.x > -10.0); // Velocity should be less negative (or positive)

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(space);
}

TEST_CASE("Damping does not affect gravity") {
	PhysicsServer4D &server = *PhysicsServer4D::get_singleton();
	RID space = server.space_create();
	server.space_set_active(space, true);

	RID body_a = server.body_create();
	server.body_set_space(body_a, space);
	server.body_set_mode(body_a, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_MASS, 100.0);
	server.body_set_param(body_a, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, 2.0);
	Transform4D ta;
	ta.origin = Vector4(-5, 0, 0, 0);
	server.body_set_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM, ta);

	RID body_b = server.body_create();
	server.body_set_space(body_b, space);
	server.body_set_mode(body_b, PhysicsServer4D::BODY_MODE_RIGID);
	server.body_set_param(body_b, PhysicsServer4D::BODY_PARAM_MASS, 10000.0);
	Transform4D tb;
	tb.origin = Vector4(5, 0, 0, 0);
	server.body_set_state(body_b, PhysicsServer4D::BODY_STATE_TRANSFORM, tb);

	for (int i = 0; i < 60; i++) {
		server.step(1.0 / 60.0);
	}

	// Despite damping, gravity should still attract the bodies
	Transform4D ra = server.body_get_state(body_a, PhysicsServer4D::BODY_STATE_TRANSFORM);
	CHECK(ra.origin.x > -5.0); // Should have moved towards B

	server.free_rid(body_a);
	server.free_rid(body_b);
	server.free_rid(space);
}

} // TEST_SUITE
