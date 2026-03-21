#include "physics_direct_body_state_4d.h"
#include "physics_server_4d.h"
#include "core/space_4d_internal.h"
#include "core/rigid_body_4d_internal.h"

void PhysicsDirectBodyState4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_transform"), &PhysicsDirectBodyState4D::get_transform);
	ClassDB::bind_method(D_METHOD("set_transform", "transform"), &PhysicsDirectBodyState4D::set_transform);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &PhysicsDirectBodyState4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "velocity"), &PhysicsDirectBodyState4D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_angular_velocity"), &PhysicsDirectBodyState4D::get_angular_velocity);
	ClassDB::bind_method(D_METHOD("set_angular_velocity", "angular_vel"), &PhysicsDirectBodyState4D::set_angular_velocity);
	ClassDB::bind_method(D_METHOD("get_step"), &PhysicsDirectBodyState4D::get_step);
	ClassDB::bind_method(D_METHOD("get_total_gravity_magnitude"), &PhysicsDirectBodyState4D::get_total_gravity_magnitude);
	ClassDB::bind_method(D_METHOD("is_sleeping"), &PhysicsDirectBodyState4D::is_sleeping);
	ClassDB::bind_method(D_METHOD("apply_central_force", "force"), &PhysicsDirectBodyState4D::apply_central_force);
	ClassDB::bind_method(D_METHOD("apply_force", "force", "position"), &PhysicsDirectBodyState4D::apply_force);
	ClassDB::bind_method(D_METHOD("apply_central_impulse", "impulse"), &PhysicsDirectBodyState4D::apply_central_impulse);
	ClassDB::bind_method(D_METHOD("apply_impulse", "impulse", "position"), &PhysicsDirectBodyState4D::apply_impulse);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "transform"), "set_transform", "get_transform");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity"), "set_linear_velocity", "get_linear_velocity");
}

PackedFloat32Array PhysicsDirectBodyState4D::get_transform() const {
	if (!_server || !_body_rid.is_valid()) return PackedFloat32Array();
	return _server->body_get_state(_body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM);
}

void PhysicsDirectBodyState4D::set_transform(const PackedFloat32Array &p_transform) {
	if (_server && _body_rid.is_valid()) {
		_server->body_set_state(_body_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, p_transform);
	}
}

Vector4 PhysicsDirectBodyState4D::get_linear_velocity() const {
	if (!_server || !_body_rid.is_valid()) return Vector4();
	return _server->body_get_state(_body_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY);
}

void PhysicsDirectBodyState4D::set_linear_velocity(const Vector4 &p_velocity) {
	if (_server && _body_rid.is_valid()) {
		_server->body_set_state(_body_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, p_velocity);
	}
}

PackedFloat32Array PhysicsDirectBodyState4D::get_angular_velocity() const {
	if (!_server || !_body_rid.is_valid()) return PackedFloat32Array();
	return _server->body_get_state(_body_rid, PhysicsServer4D::BODY_STATE_ANGULAR_VELOCITY);
}

void PhysicsDirectBodyState4D::set_angular_velocity(const PackedFloat32Array &p_angular_vel) {
	if (_server && _body_rid.is_valid()) {
		_server->body_set_state(_body_rid, PhysicsServer4D::BODY_STATE_ANGULAR_VELOCITY, p_angular_vel);
	}
}

real_t PhysicsDirectBodyState4D::get_total_gravity_magnitude() const {
	if (!_server || !_body_rid.is_valid()) return 9.8f;
	// Get the body's space, compute gravity for it
	RID space_rid = _server->body_get_space(_body_rid);
	Space4DInternal *space = _server->get_space_internal(space_rid);
	if (!space) return 9.8f;
	return space->gravity_magnitude;
}

Vector4 PhysicsDirectBodyState4D::get_total_linear_damp() const {
	if (!_server || !_body_rid.is_valid()) return Vector4();
	RigidBody4DInternal *body = _server->get_body_internal(_body_rid);
	if (!body) return Vector4();
	float damp = body->linear_damp;
	return Vector4(damp, damp, damp, damp);
}

real_t PhysicsDirectBodyState4D::get_step() const {
	if (!_server || !_body_rid.is_valid()) return 1.0f / 60.0f;
	RID space_rid = _server->body_get_space(_body_rid);
	Space4DInternal *space = _server->get_space_internal(space_rid);
	if (!space) return 1.0f / 60.0f;
	return space->last_dt;
}

void PhysicsDirectBodyState4D::apply_central_force(const Vector4 &p_force) {
	if (_server && _body_rid.is_valid()) _server->body_apply_central_force(_body_rid, p_force);
}

void PhysicsDirectBodyState4D::apply_force(const Vector4 &p_force, const Vector4 &p_position) {
	if (_server && _body_rid.is_valid()) _server->body_apply_force(_body_rid, p_force, p_position);
}

void PhysicsDirectBodyState4D::apply_central_impulse(const Vector4 &p_impulse) {
	if (_server && _body_rid.is_valid()) _server->body_apply_central_impulse(_body_rid, p_impulse);
}

void PhysicsDirectBodyState4D::apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position) {
	if (_server && _body_rid.is_valid()) _server->body_apply_impulse(_body_rid, p_impulse, p_position);
}

bool PhysicsDirectBodyState4D::is_sleeping() const {
	if (!_server || !_body_rid.is_valid()) return false;
	return _server->body_get_state(_body_rid, PhysicsServer4D::BODY_STATE_SLEEPING);
}
