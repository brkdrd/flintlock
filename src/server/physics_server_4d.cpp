#include "physics_server_4d.h"
#include "space_4d.h"
#include "body_4d.h"
#include "shape_4d_resource.h"

PhysicsServer4D *PhysicsServer4D::singleton = nullptr;

PhysicsServer4D::PhysicsServer4D() {
}

PhysicsServer4D::~PhysicsServer4D() {
	// Cleanup all resources
	for (auto &pair : bodies) {
		delete pair.second;
	}
	for (auto &pair : shapes) {
		delete pair.second;
	}
	for (auto &pair : spaces) {
		delete pair.second;
	}
}

PhysicsServer4D *PhysicsServer4D::get_singleton() {
	return singleton;
}

void PhysicsServer4D::initialize() {
	if (!singleton) {
		singleton = new PhysicsServer4D();
	}
}

void PhysicsServer4D::finalize() {
	if (singleton) {
		delete singleton;
		singleton = nullptr;
	}
}

// Space management
RID PhysicsServer4D::space_create() {
	RID rid = allocate_rid();
	spaces[rid] = new Space4D();
	return rid;
}

void PhysicsServer4D::space_set_active(RID p_space, bool p_active) {
	Space4D *space = get_space(p_space);
	if (space) {
		space->set_active(p_active);
	}
}

// Body management
RID PhysicsServer4D::body_create() {
	RID rid = allocate_rid();
	bodies[rid] = new Body4D();
	return rid;
}

void PhysicsServer4D::body_set_space(RID p_body, RID p_space) {
	Body4D *body = get_body(p_body);
	Space4D *space = get_space(p_space);
	if (body) {
		body->set_space(space);
	}
}

void PhysicsServer4D::body_set_mode(RID p_body, BodyMode p_mode) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->set_mode(static_cast<BodyState4D::Mode>(p_mode));
	}
}

// Body state
void PhysicsServer4D::body_set_state(RID p_body, BodyState p_state, const Vector4 &p_value) {
	Body4D *body = get_body(p_body);
	if (!body) return;

	switch (p_state) {
		case BODY_STATE_LINEAR_VELOCITY:
			body->set_linear_velocity(p_value);
			break;
		default:
			break;
	}
}

void PhysicsServer4D::body_set_state(RID p_body, BodyState p_state, const Transform4D &p_value) {
	Body4D *body = get_body(p_body);
	if (!body) return;

	switch (p_state) {
		case BODY_STATE_TRANSFORM:
			body->set_transform(p_value);
			break;
		default:
			break;
	}
}

BodyStateValue PhysicsServer4D::body_get_state(RID p_body, BodyState p_state) {
	Body4D *body = get_body(p_body);
	if (!body) return BodyStateValue();

	switch (p_state) {
		case BODY_STATE_TRANSFORM:
			return BodyStateValue(body->get_transform());
		case BODY_STATE_LINEAR_VELOCITY:
			return BodyStateValue(body->get_linear_velocity());
		default:
			return BodyStateValue();
	}
}

// Body parameters
void PhysicsServer4D::body_set_param(RID p_body, BodyParam p_param, real_t p_value) {
	Body4D *body = get_body(p_body);
	if (!body) return;

	switch (p_param) {
		case BODY_PARAM_MASS:
			body->set_mass(p_value);
			break;
		case BODY_PARAM_LINEAR_DAMP:
			body->set_linear_damp(p_value);
			break;
		default:
			break;
	}
}

real_t PhysicsServer4D::body_get_param(RID p_body, BodyParam p_param) {
	Body4D *body = get_body(p_body);
	if (!body) return 0.0;

	switch (p_param) {
		case BODY_PARAM_MASS:
			return body->get_mass();
		case BODY_PARAM_LINEAR_DAMP:
			return body->get_linear_damp();
		default:
			return 0.0;
	}
}

// Body forces
void PhysicsServer4D::body_apply_impulse(RID p_body, const Vector4 &p_impulse, const Vector4 &p_position) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->apply_impulse(p_impulse, p_position);
	}
}

void PhysicsServer4D::body_apply_force(RID p_body, const Vector4 &p_force, const Vector4 &p_position) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->apply_force(p_force, p_position);
	}
}

// Shape management
RID PhysicsServer4D::shape_create(ShapeType p_type) {
	RID rid = allocate_rid();
	Shape4DResource::Type res_type;

	switch (p_type) {
		case SHAPE_HYPER_SPHERE:
			res_type = Shape4DResource::TYPE_HYPER_SPHERE;
			break;
		case SHAPE_HYPER_BOX:
			res_type = Shape4DResource::TYPE_HYPER_BOX;
			break;
		default:
			res_type = Shape4DResource::TYPE_HYPER_SPHERE;
			break;
	}

	shapes[rid] = new Shape4DResource(res_type);
	return rid;
}

void PhysicsServer4D::shape_set_data(RID p_shape, real_t p_data) {
	Shape4DResource *shape = get_shape(p_shape);
	if (shape && shape->get_type() == Shape4DResource::TYPE_HYPER_SPHERE) {
		shape->set_sphere_radius(p_data);
	}
}

void PhysicsServer4D::shape_set_data(RID p_shape, const Vector4 &p_data) {
	Shape4DResource *shape = get_shape(p_shape);
	if (shape && shape->get_type() == Shape4DResource::TYPE_HYPER_BOX) {
		shape->set_box_half_extents(p_data);
	}
}

// Body-shape attachment
void PhysicsServer4D::body_add_shape(RID p_body, RID p_shape, const Transform4D &p_transform) {
	Body4D *body = get_body(p_body);
	Shape4DResource *shape_res = get_shape(p_shape);

	if (body && shape_res) {
		body->add_shape(shape_res, shape_res->get_shape(), p_transform);
	}
}

void PhysicsServer4D::body_remove_shape(RID p_body, int p_index) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->remove_shape(p_index);
	}
}

// Simulation
void PhysicsServer4D::step(real_t p_delta) {
	// Step all active spaces
	for (auto &pair : spaces) {
		pair.second->step(p_delta);
	}
}

// Cleanup
void PhysicsServer4D::free_rid(RID p_rid) {
	// Try to free as space
	auto space_it = spaces.find(p_rid);
	if (space_it != spaces.end()) {
		delete space_it->second;
		spaces.erase(space_it);
		return;
	}

	// Try to free as body
	auto body_it = bodies.find(p_rid);
	if (body_it != bodies.end()) {
		Body4D *body = body_it->second;
		body->set_space(nullptr); // Remove from space
		delete body;
		bodies.erase(body_it);
		return;
	}

	// Try to free as shape
	auto shape_it = shapes.find(p_rid);
	if (shape_it != shapes.end()) {
		delete shape_it->second;
		shapes.erase(shape_it);
		return;
	}
}

// Lookup helpers
Space4D *PhysicsServer4D::get_space(RID p_rid) {
	auto it = spaces.find(p_rid);
	return (it != spaces.end()) ? it->second : nullptr;
}

Body4D *PhysicsServer4D::get_body(RID p_rid) {
	auto it = bodies.find(p_rid);
	return (it != bodies.end()) ? it->second : nullptr;
}

Shape4DResource *PhysicsServer4D::get_shape(RID p_rid) {
	auto it = shapes.find(p_rid);
	return (it != shapes.end()) ? it->second : nullptr;
}
