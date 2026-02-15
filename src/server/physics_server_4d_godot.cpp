#include "physics_server_4d_godot.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsServer4DGodot::PhysicsServer4DGodot() {
}

PhysicsServer4DGodot::~PhysicsServer4DGodot() {
}

void PhysicsServer4DGodot::_bind_methods() {
	// Space methods
	ClassDB::bind_method(D_METHOD("space_create"), &PhysicsServer4DGodot::space_create);
	ClassDB::bind_method(D_METHOD("space_set_active", "space", "active"), &PhysicsServer4DGodot::space_set_active);

	// Body methods
	ClassDB::bind_method(D_METHOD("body_create"), &PhysicsServer4DGodot::body_create);
	ClassDB::bind_method(D_METHOD("body_set_space", "body", "space"), &PhysicsServer4DGodot::body_set_space);
	ClassDB::bind_method(D_METHOD("body_set_mode", "body", "mode"), &PhysicsServer4DGodot::body_set_mode);

	// Body state methods
	ClassDB::bind_method(D_METHOD("body_set_state_vector", "body", "state", "value"), &PhysicsServer4DGodot::body_set_state_vector);
	ClassDB::bind_method(D_METHOD("body_get_state_vector", "body", "state"), &PhysicsServer4DGodot::body_get_state_vector);

	// Body parameter methods
	ClassDB::bind_method(D_METHOD("body_set_param", "body", "param", "value"), &PhysicsServer4DGodot::body_set_param);
	ClassDB::bind_method(D_METHOD("body_get_param", "body", "param"), &PhysicsServer4DGodot::body_get_param);

	// Body force methods
	ClassDB::bind_method(D_METHOD("body_apply_impulse", "body", "impulse", "position"), &PhysicsServer4DGodot::body_apply_impulse, DEFVAL(Vector4()));
	ClassDB::bind_method(D_METHOD("body_apply_force", "body", "force", "position"), &PhysicsServer4DGodot::body_apply_force, DEFVAL(Vector4()));

	// Shape methods
	ClassDB::bind_method(D_METHOD("shape_create", "type"), &PhysicsServer4DGodot::shape_create);
	ClassDB::bind_method(D_METHOD("shape_set_data_real", "shape", "data"), &PhysicsServer4DGodot::shape_set_data_real);
	ClassDB::bind_method(D_METHOD("shape_set_data_vector4", "shape", "data"), &PhysicsServer4DGodot::shape_set_data_vector4);

	// Body-shape methods
	ClassDB::bind_method(D_METHOD("body_add_shape", "body", "shape"), &PhysicsServer4DGodot::body_add_shape);
	ClassDB::bind_method(D_METHOD("body_remove_shape", "body", "index"), &PhysicsServer4DGodot::body_remove_shape);

	// Simulation methods
	ClassDB::bind_method(D_METHOD("step", "delta"), &PhysicsServer4DGodot::step);

	// Cleanup methods
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &PhysicsServer4DGodot::free_rid);

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

// RID conversion helpers
flintlock::PhysicsRID PhysicsServer4DGodot::to_internal_rid(const RID &p_godot_rid) {
	int64_t rid_id = p_godot_rid.get_id();
	auto it = rid_map.find(rid_id);
	if (it != rid_map.end()) {
		return it->second;
	}
	return flintlock::PhysicsRID(); // Invalid RID
}

RID PhysicsServer4DGodot::to_godot_rid(const flintlock::PhysicsRID &p_internal_rid) {
	uint64_t internal_id = p_internal_rid.get_id();

	// Check if we already have a stored RID for this internal RID
	auto storage_it = rid_storage.find(internal_id);
	if (storage_it != rid_storage.end()) {
		return storage_it->second;
	}

	// Create a new RID and store it
	RID new_rid;
	int64_t rid_id = new_rid.get_id();

	// Store bidirectional mapping
	rid_map[rid_id] = p_internal_rid;
	rid_storage[internal_id] = new_rid;

	return new_rid;
}

// Space management
RID PhysicsServer4DGodot::space_create() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return RID();

	flintlock::PhysicsRID internal_rid = server->space_create();
	return to_godot_rid(internal_rid);
}

void PhysicsServer4DGodot::space_set_active(const RID &p_space, bool p_active) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID internal_rid = to_internal_rid(p_space);
	server->space_set_active(internal_rid, p_active);
}

// Body management
RID PhysicsServer4DGodot::body_create() {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return RID();

	flintlock::PhysicsRID internal_rid = server->body_create();
	return to_godot_rid(internal_rid);
}

void PhysicsServer4DGodot::body_set_space(const RID &p_body, const RID &p_space) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	flintlock::PhysicsRID space_rid = to_internal_rid(p_space);
	server->body_set_space(body_rid, space_rid);
}

void PhysicsServer4DGodot::body_set_mode(const RID &p_body, BodyMode p_mode) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_set_mode(body_rid, static_cast<PhysicsServer4D::BodyMode>(p_mode));
}

// Body state
void PhysicsServer4DGodot::body_set_state_vector(const RID &p_body, BodyState p_state, const Vector4 &p_value) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_set_state(body_rid, static_cast<PhysicsServer4D::BodyState>(p_state), p_value);
}

Vector4 PhysicsServer4DGodot::body_get_state_vector(const RID &p_body, BodyState p_state) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return Vector4();

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	BodyStateValue value = server->body_get_state(body_rid, static_cast<PhysicsServer4D::BodyState>(p_state));

	// Return the vector value (works for both Vector4 and Transform4D states)
	return value.vector_value;
}

// Body parameters
void PhysicsServer4DGodot::body_set_param(const RID &p_body, BodyParam p_param, real_t p_value) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_set_param(body_rid, static_cast<PhysicsServer4D::BodyParam>(p_param), p_value);
}

real_t PhysicsServer4DGodot::body_get_param(const RID &p_body, BodyParam p_param) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return 0.0;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	return server->body_get_param(body_rid, static_cast<PhysicsServer4D::BodyParam>(p_param));
}

// Body forces
void PhysicsServer4DGodot::body_apply_impulse(const RID &p_body, const Vector4 &p_impulse, const Vector4 &p_position) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_apply_impulse(body_rid, p_impulse, p_position);
}

void PhysicsServer4DGodot::body_apply_force(const RID &p_body, const Vector4 &p_force, const Vector4 &p_position) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_apply_force(body_rid, p_force, p_position);
}

// Shape management
RID PhysicsServer4DGodot::shape_create(ShapeType p_type) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return RID();

	flintlock::PhysicsRID internal_rid = server->shape_create(static_cast<PhysicsServer4D::ShapeType>(p_type));
	return to_godot_rid(internal_rid);
}

void PhysicsServer4DGodot::shape_set_data_real(const RID &p_shape, real_t p_data) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID shape_rid = to_internal_rid(p_shape);
	server->shape_set_data(shape_rid, p_data);
}

void PhysicsServer4DGodot::shape_set_data_vector4(const RID &p_shape, const Vector4 &p_data) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID shape_rid = to_internal_rid(p_shape);
	server->shape_set_data(shape_rid, p_data);
}

// Body-shape attachment
void PhysicsServer4DGodot::body_add_shape(const RID &p_body, const RID &p_shape) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	flintlock::PhysicsRID shape_rid = to_internal_rid(p_shape);
	server->body_add_shape(body_rid, shape_rid, Transform4D()); // Use identity transform
}

void PhysicsServer4DGodot::body_remove_shape(const RID &p_body, int p_index) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID body_rid = to_internal_rid(p_body);
	server->body_remove_shape(body_rid, p_index);
}

// Simulation
void PhysicsServer4DGodot::step(real_t p_delta) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	server->step(p_delta);
}

// Cleanup
void PhysicsServer4DGodot::free_rid(const RID &p_rid) {
	PhysicsServer4D *server = PhysicsServer4D::get_singleton();
	if (!server) return;

	flintlock::PhysicsRID internal_rid = to_internal_rid(p_rid);
	server->free_rid(internal_rid);

	// Remove from our mappings
	int64_t rid_id = p_rid.get_id();
	auto it = rid_map.find(rid_id);
	if (it != rid_map.end()) {
		uint64_t internal_id = it->second.get_id();
		rid_storage.erase(internal_id);
		rid_map.erase(it);
	}
}
