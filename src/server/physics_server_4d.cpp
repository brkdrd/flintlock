#include "physics_server_4d.h"
#include "space_4d.h"
#include "body_4d.h"
#include "shape_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsServer4D *PhysicsServer4D::singleton = nullptr;

PhysicsServer4D::PhysicsServer4D() {
	singleton = this;
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

	if (singleton == this) {
		singleton = nullptr;
	}
}

PhysicsServer4D *PhysicsServer4D::get_singleton() {
	return singleton;
}

RID PhysicsServer4D::allocate_rid() {
	uint64_t id = next_rid_id++;
	RID rid; // Create default RID - Godot will manage the internal ID
	rid_storage[id] = rid;
	return rid;
}

// Space management
RID PhysicsServer4D::space_create() {
	uint64_t id = next_rid_id++;
	RID rid;
	rid_storage[id] = rid;
	spaces[id] = new Space4D();
	return rid;
}

void PhysicsServer4D::space_set_active(const RID &p_space, bool p_active) {
	Space4D *space = get_space(p_space);
	if (space) {
		space->set_active(p_active);
	}
}

// Body management
RID PhysicsServer4D::body_create() {
	uint64_t id = next_rid_id++;
	RID rid;
	rid_storage[id] = rid;
	bodies[id] = new Body4D();
	return rid;
}

void PhysicsServer4D::body_set_space(const RID &p_body, const RID &p_space) {
	Body4D *body = get_body(p_body);
	Space4D *space = get_space(p_space);
	if (body) {
		body->set_space(space);
	}
}

void PhysicsServer4D::body_set_mode(const RID &p_body, BodyMode p_mode) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->set_mode(static_cast<BodyState4D::Mode>(p_mode));
	}
}

// Body state - Vector4 variant (for GDScript)
void PhysicsServer4D::body_set_state_vector(const RID &p_body, BodyState p_state, const Vector4 &p_value) {
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

Vector4 PhysicsServer4D::body_get_state_vector(const RID &p_body, BodyState p_state) {
	Body4D *body = get_body(p_body);
	if (!body) return Vector4();

	switch (p_state) {
		case BODY_STATE_LINEAR_VELOCITY:
			return body->get_linear_velocity();
		default:
			return Vector4();
	}
}

// Body state - Transform4D variant (internal use)
void PhysicsServer4D::body_set_state(const RID &p_body, BodyState p_state, const Transform4D &p_value) {
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

BodyStateValue PhysicsServer4D::body_get_state(const RID &p_body, BodyState p_state) {
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
void PhysicsServer4D::body_set_param(const RID &p_body, BodyParam p_param, real_t p_value) {
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

real_t PhysicsServer4D::body_get_param(const RID &p_body, BodyParam p_param) {
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
void PhysicsServer4D::body_apply_impulse(const RID &p_body, const Vector4 &p_impulse, const Vector4 &p_position) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->apply_impulse(p_impulse, p_position);
	}
}

void PhysicsServer4D::body_apply_force(const RID &p_body, const Vector4 &p_force, const Vector4 &p_position) {
	Body4D *body = get_body(p_body);
	if (body) {
		body->apply_force(p_force, p_position);
	}
}

// Shape management
RID PhysicsServer4D::shape_create(ShapeType p_type) {
	uint64_t id = next_rid_id++;
	RID rid;
	rid_storage[id] = rid;

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

	shapes[id] = new Shape4DResource(res_type);
	return rid;
}

void PhysicsServer4D::shape_set_data_real(const RID &p_shape, real_t p_data) {
	Shape4DResource *shape = get_shape(p_shape);
	if (shape && shape->get_type() == Shape4DResource::TYPE_HYPER_SPHERE) {
		shape->set_sphere_radius(p_data);
	}
}

void PhysicsServer4D::shape_set_data_vector4(const RID &p_shape, const Vector4 &p_data) {
	Shape4DResource *shape = get_shape(p_shape);
	if (shape && shape->get_type() == Shape4DResource::TYPE_HYPER_BOX) {
		shape->set_box_half_extents(p_data);
	}
}

// Body-shape attachment
void PhysicsServer4D::body_add_shape(const RID &p_body, const RID &p_shape) {
	Body4D *body = get_body(p_body);
	Shape4DResource *shape_res = get_shape(p_shape);

	if (body && shape_res) {
		body->add_shape(shape_res, shape_res->get_shape(), Transform4D());
	}
}

void PhysicsServer4D::body_remove_shape(const RID &p_body, int p_index) {
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
void PhysicsServer4D::free_rid(const RID &p_rid) {
	// Find the ID that corresponds to this RID
	for (auto &pair : rid_storage) {
		if (pair.second == p_rid) {
			uint64_t id = pair.first;

			// Try to free as space
			auto space_it = spaces.find(id);
			if (space_it != spaces.end()) {
				delete space_it->second;
				spaces.erase(space_it);
				rid_storage.erase(id);
				return;
			}

			// Try to free as body
			auto body_it = bodies.find(id);
			if (body_it != bodies.end()) {
				Body4D *body = body_it->second;
				body->set_space(nullptr); // Remove from space
				delete body;
				bodies.erase(body_it);
				rid_storage.erase(id);
				return;
			}

			// Try to free as shape
			auto shape_it = shapes.find(id);
			if (shape_it != shapes.end()) {
				delete shape_it->second;
				shapes.erase(shape_it);
				rid_storage.erase(id);
				return;
			}
			break;
		}
	}
}

// Lookup helpers
Space4D *PhysicsServer4D::get_space(const RID &p_rid) {
	// Find the ID that corresponds to this RID
	for (auto &pair : rid_storage) {
		if (pair.second == p_rid) {
			auto it = spaces.find(pair.first);
			return (it != spaces.end()) ? it->second : nullptr;
		}
	}
	return nullptr;
}

Body4D *PhysicsServer4D::get_body(const RID &p_rid) {
	// Find the ID that corresponds to this RID
	for (auto &pair : rid_storage) {
		if (pair.second == p_rid) {
			auto it = bodies.find(pair.first);
			return (it != bodies.end()) ? it->second : nullptr;
		}
	}
	return nullptr;
}

Shape4DResource *PhysicsServer4D::get_shape(const RID &p_rid) {
	// Find the ID that corresponds to this RID
	for (auto &pair : rid_storage) {
		if (pair.second == p_rid) {
			auto it = shapes.find(pair.first);
			return (it != shapes.end()) ? it->second : nullptr;
		}
	}
	return nullptr;
}

// Bind methods for GDScript
void PhysicsServer4D::_bind_methods() {
	// Space management
	ClassDB::bind_method(D_METHOD("space_create"), &PhysicsServer4D::space_create);
	ClassDB::bind_method(D_METHOD("space_set_active", "space", "active"), &PhysicsServer4D::space_set_active);

	// Body management
	ClassDB::bind_method(D_METHOD("body_create"), &PhysicsServer4D::body_create);
	ClassDB::bind_method(D_METHOD("body_set_space", "body", "space"), &PhysicsServer4D::body_set_space);
	ClassDB::bind_method(D_METHOD("body_set_mode", "body", "mode"), &PhysicsServer4D::body_set_mode);

	// Body state (Vector4 only for GDScript)
	ClassDB::bind_method(D_METHOD("body_set_state_vector", "body", "state", "value"), &PhysicsServer4D::body_set_state_vector);
	ClassDB::bind_method(D_METHOD("body_get_state_vector", "body", "state"), &PhysicsServer4D::body_get_state_vector);

	// Body parameters
	ClassDB::bind_method(D_METHOD("body_set_param", "body", "param", "value"), &PhysicsServer4D::body_set_param);
	ClassDB::bind_method(D_METHOD("body_get_param", "body", "param"), &PhysicsServer4D::body_get_param);

	// Body forces
	ClassDB::bind_method(D_METHOD("body_apply_impulse", "body", "impulse", "position"), &PhysicsServer4D::body_apply_impulse, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("body_apply_force", "body", "force", "position"), &PhysicsServer4D::body_apply_force, DEFVAL(Vector4()));

	// Shape management
	ClassDB::bind_method(D_METHOD("shape_create", "type"), &PhysicsServer4D::shape_create);
	ClassDB::bind_method(D_METHOD("shape_set_data_real", "shape", "data"), &PhysicsServer4D::shape_set_data_real);
	ClassDB::bind_method(D_METHOD("shape_set_data_vector4", "shape", "data"), &PhysicsServer4D::shape_set_data_vector4);

	// Body-shape attachment
	ClassDB::bind_method(D_METHOD("body_add_shape", "body", "shape"), &PhysicsServer4D::body_add_shape);
	ClassDB::bind_method(D_METHOD("body_remove_shape", "body", "index"), &PhysicsServer4D::body_remove_shape);

	// Simulation
	ClassDB::bind_method(D_METHOD("step", "delta"), &PhysicsServer4D::step);

	// Cleanup
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &PhysicsServer4D::free_rid);

	// Bind enums
	BIND_ENUM_CONSTANT(BODY_MODE_STATIC);
	BIND_ENUM_CONSTANT(BODY_MODE_KINEMATIC);
	BIND_ENUM_CONSTANT(BODY_MODE_RIGID);

	BIND_ENUM_CONSTANT(BODY_STATE_TRANSFORM);
	BIND_ENUM_CONSTANT(BODY_STATE_LINEAR_VELOCITY);
	BIND_ENUM_CONSTANT(BODY_STATE_ANGULAR_VELOCITY);
	BIND_ENUM_CONSTANT(BODY_STATE_SLEEPING);
	BIND_ENUM_CONSTANT(BODY_STATE_CAN_SLEEP);

	BIND_ENUM_CONSTANT(BODY_PARAM_MASS);
	BIND_ENUM_CONSTANT(BODY_PARAM_GRAVITY_SCALE);
	BIND_ENUM_CONSTANT(BODY_PARAM_LINEAR_DAMP);
	BIND_ENUM_CONSTANT(BODY_PARAM_ANGULAR_DAMP);

	BIND_ENUM_CONSTANT(SHAPE_HYPER_SPHERE);
	BIND_ENUM_CONSTANT(SHAPE_HYPER_BOX);
	BIND_ENUM_CONSTANT(SHAPE_HYPER_CAPSULE);
	BIND_ENUM_CONSTANT(SHAPE_HYPER_ELLIPSOID);
	BIND_ENUM_CONSTANT(SHAPE_CONVEX_HULL);
}
