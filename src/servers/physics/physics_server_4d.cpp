#include "physics_server_4d.h"
#include "physics_direct_space_state_4d.h"
#include "physics_direct_body_state_4d.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsServer4D *PhysicsServer4D::singleton = nullptr;

PhysicsServer4D::PhysicsServer4D() { singleton = this; }
PhysicsServer4D::~PhysicsServer4D() { singleton = nullptr; }
PhysicsServer4D *PhysicsServer4D::get_singleton() { return singleton; }

void PhysicsServer4D::_bind_methods() {
	// Enums
	BIND_ENUM_CONSTANT(BODY_MODE_STATIC);
	BIND_ENUM_CONSTANT(BODY_MODE_KINEMATIC);
	BIND_ENUM_CONSTANT(BODY_MODE_RIGID);
	BIND_ENUM_CONSTANT(BODY_MODE_RIGID_LINEAR);

	BIND_ENUM_CONSTANT(BODY_PARAM_BOUNCE);
	BIND_ENUM_CONSTANT(BODY_PARAM_FRICTION);
	BIND_ENUM_CONSTANT(BODY_PARAM_MASS);
	BIND_ENUM_CONSTANT(BODY_PARAM_GRAVITY_SCALE);
	BIND_ENUM_CONSTANT(BODY_PARAM_LINEAR_DAMP);
	BIND_ENUM_CONSTANT(BODY_PARAM_ANGULAR_DAMP);

	BIND_ENUM_CONSTANT(BODY_STATE_TRANSFORM);
	BIND_ENUM_CONSTANT(BODY_STATE_LINEAR_VELOCITY);
	BIND_ENUM_CONSTANT(BODY_STATE_ANGULAR_VELOCITY);
	BIND_ENUM_CONSTANT(BODY_STATE_SLEEPING);
	BIND_ENUM_CONSTANT(BODY_STATE_CAN_SLEEP);

	// Space API
	ClassDB::bind_method(D_METHOD("space_create"), &PhysicsServer4D::space_create);
	ClassDB::bind_method(D_METHOD("space_set_active", "space", "active"), &PhysicsServer4D::space_set_active);
	ClassDB::bind_method(D_METHOD("space_is_active", "space"), &PhysicsServer4D::space_is_active);
	ClassDB::bind_method(D_METHOD("space_get_direct_state", "space"), &PhysicsServer4D::space_get_direct_state);

	// Body API
	ClassDB::bind_method(D_METHOD("body_create"), &PhysicsServer4D::body_create);
	ClassDB::bind_method(D_METHOD("body_set_space", "body", "space"), &PhysicsServer4D::body_set_space);
	ClassDB::bind_method(D_METHOD("body_get_space", "body"), &PhysicsServer4D::body_get_space);
	ClassDB::bind_method(D_METHOD("body_set_mode", "body", "mode"), &PhysicsServer4D::body_set_mode);
	ClassDB::bind_method(D_METHOD("body_get_mode", "body"), &PhysicsServer4D::body_get_mode);
	ClassDB::bind_method(D_METHOD("body_add_shape", "body", "shape", "transform", "disabled"), &PhysicsServer4D::body_add_shape, DEFVAL(PackedFloat32Array()), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("body_remove_shape", "body", "shape_idx"), &PhysicsServer4D::body_remove_shape);
	ClassDB::bind_method(D_METHOD("body_clear_shapes", "body"), &PhysicsServer4D::body_clear_shapes);
	ClassDB::bind_method(D_METHOD("body_set_param", "body", "param", "value"), &PhysicsServer4D::body_set_param);
	ClassDB::bind_method(D_METHOD("body_get_param", "body", "param"), &PhysicsServer4D::body_get_param);
	ClassDB::bind_method(D_METHOD("body_set_state", "body", "state", "value"), &PhysicsServer4D::body_set_state);
	ClassDB::bind_method(D_METHOD("body_get_state", "body", "state"), &PhysicsServer4D::body_get_state);
	ClassDB::bind_method(D_METHOD("body_set_state_sync_callback", "body", "callable"), &PhysicsServer4D::body_set_state_sync_callback);
	ClassDB::bind_method(D_METHOD("body_apply_central_force", "body", "force"), &PhysicsServer4D::body_apply_central_force);
	ClassDB::bind_method(D_METHOD("body_apply_force", "body", "force", "position"), &PhysicsServer4D::body_apply_force);
	ClassDB::bind_method(D_METHOD("body_apply_central_impulse", "body", "impulse"), &PhysicsServer4D::body_apply_central_impulse);
	ClassDB::bind_method(D_METHOD("body_apply_impulse", "body", "impulse", "position"), &PhysicsServer4D::body_apply_impulse);
	ClassDB::bind_method(D_METHOD("body_set_object_instance_id", "body", "id"), &PhysicsServer4D::body_set_object_instance_id);

	// Area API
	ClassDB::bind_method(D_METHOD("area_create"), &PhysicsServer4D::area_create);
	ClassDB::bind_method(D_METHOD("area_set_space", "area", "space"), &PhysicsServer4D::area_set_space);
	ClassDB::bind_method(D_METHOD("area_set_param", "area", "param", "value"), &PhysicsServer4D::area_set_param);
	ClassDB::bind_method(D_METHOD("area_get_param", "area", "param"), &PhysicsServer4D::area_get_param);
	ClassDB::bind_method(D_METHOD("area_set_transform", "area", "transform"), &PhysicsServer4D::area_set_transform);
	ClassDB::bind_method(D_METHOD("area_set_monitor_callback", "area", "callback"), &PhysicsServer4D::area_set_monitor_callback);
	ClassDB::bind_method(D_METHOD("area_set_area_monitor_callback", "area", "callback"), &PhysicsServer4D::area_set_area_monitor_callback);
	ClassDB::bind_method(D_METHOD("area_set_monitorable", "area", "monitorable"), &PhysicsServer4D::area_set_monitorable);
	ClassDB::bind_method(D_METHOD("area_set_object_instance_id", "area", "id"), &PhysicsServer4D::area_set_object_instance_id);

	// Shape API
	ClassDB::bind_method(D_METHOD("shape_create", "type"), &PhysicsServer4D::shape_create);
	ClassDB::bind_method(D_METHOD("shape_set_data", "shape", "data"), &PhysicsServer4D::shape_set_data);
	ClassDB::bind_method(D_METHOD("shape_get_data", "shape"), &PhysicsServer4D::shape_get_data);

	// Step
	ClassDB::bind_method(D_METHOD("step", "delta"), &PhysicsServer4D::step);
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &PhysicsServer4D::free_rid);
}

// ---- Space ----
RID PhysicsServer4D::space_create() {
	RID rid = _make_rid();
	SpaceData sd;
	sd.direct_state.instantiate();
	sd.direct_state->_space_rid = rid;
	_spaces[rid] = sd;
	return rid;
}

void PhysicsServer4D::space_set_active(RID p_space, bool p_active) {
	if (!_spaces.has(p_space)) return;
	if (p_active && !_active_spaces.has(p_space)) {
		_active_spaces.push_back(p_space);
	} else if (!p_active) {
		_active_spaces.erase(p_space);
	}
}

bool PhysicsServer4D::space_is_active(RID p_space) const {
	return _active_spaces.has(p_space);
}

void PhysicsServer4D::space_set_param(RID p_space, SpaceParam p_param, real_t p_value) {
	// No-op for stub
}

real_t PhysicsServer4D::space_get_param(RID p_space, SpaceParam p_param) const {
	return 0.0f;
}

Ref<PhysicsDirectSpaceState4D> PhysicsServer4D::space_get_direct_state(RID p_space) {
	if (!_spaces.has(p_space)) return Ref<PhysicsDirectSpaceState4D>();
	return _spaces[p_space].direct_state;
}

// ---- Body ----
RID PhysicsServer4D::body_create() {
	RID rid = _make_rid();
	_bodies[rid] = BodyData();
	// Initialize transform as identity (20 floats: 4 cols of 4 + 4 origin)
	PackedFloat32Array t;
	t.resize(20);
	// Identity basis (column-major): col0=(1,0,0,0), col1=(0,1,0,0), col2=(0,0,1,0), col3=(0,0,0,1), origin=(0,0,0,0)
	t[0]=1; t[1]=0; t[2]=0; t[3]=0;  // col0
	t[4]=0; t[5]=1; t[6]=0; t[7]=0;  // col1
	t[8]=0; t[9]=0; t[10]=1; t[11]=0; // col2
	t[12]=0; t[13]=0; t[14]=0; t[15]=1; // col3
	t[16]=0; t[17]=0; t[18]=0; t[19]=0; // origin
	_bodies[rid].transform = t;
	_bodies[rid].angular_velocity.resize(6);
	return rid;
}

void PhysicsServer4D::body_set_space(RID p_body, RID p_space) {
	if (!_bodies.has(p_body)) return;
	// Remove from old space
	RID old_space = _bodies[p_body].space;
	if (old_space.is_valid() && _spaces.has(old_space)) {
		_spaces[old_space].bodies.erase(p_body);
	}
	_bodies[p_body].space = p_space;
	if (p_space.is_valid() && _spaces.has(p_space)) {
		_spaces[p_space].bodies.push_back(p_body);
	}
}

RID PhysicsServer4D::body_get_space(RID p_body) const {
	if (!_bodies.has(p_body)) return RID();
	return _bodies[p_body].space;
}

void PhysicsServer4D::body_set_mode(RID p_body, BodyMode p_mode) {
	if (_bodies.has(p_body)) _bodies[p_body].mode = p_mode;
}

PhysicsServer4D::BodyMode PhysicsServer4D::body_get_mode(RID p_body) const {
	if (!_bodies.has(p_body)) return BODY_MODE_STATIC;
	return _bodies[p_body].mode;
}

void PhysicsServer4D::body_add_shape(RID p_body, RID p_shape, const PackedFloat32Array &p_transform, bool p_disabled) {
	if (_bodies.has(p_body)) _bodies[p_body].shapes.push_back(p_shape);
}

void PhysicsServer4D::body_remove_shape(RID p_body, int p_shape_idx) {
	if (_bodies.has(p_body) && p_shape_idx < _bodies[p_body].shapes.size()) {
		_bodies[p_body].shapes.remove_at(p_shape_idx);
	}
}

void PhysicsServer4D::body_clear_shapes(RID p_body) {
	if (_bodies.has(p_body)) _bodies[p_body].shapes.clear();
}

int PhysicsServer4D::body_get_shape_count(RID p_body) const {
	if (!_bodies.has(p_body)) return 0;
	return _bodies[p_body].shapes.size();
}

void PhysicsServer4D::body_set_shape_disabled(RID p_body, int p_shape_idx, bool p_disabled) {}

void PhysicsServer4D::body_set_param(RID p_body, BodyParam p_param, const Variant &p_value) {
	if (!_bodies.has(p_body)) return;
	BodyData &bd = _bodies[p_body];
	switch (p_param) {
		case BODY_PARAM_MASS: bd.mass = p_value; break;
		case BODY_PARAM_FRICTION: bd.friction = p_value; break;
		case BODY_PARAM_BOUNCE: bd.bounce = p_value; break;
		case BODY_PARAM_GRAVITY_SCALE: bd.gravity_scale = p_value; break;
		case BODY_PARAM_LINEAR_DAMP: bd.linear_damp = p_value; break;
		case BODY_PARAM_ANGULAR_DAMP: bd.angular_damp = p_value; break;
		default: break;
	}
}

Variant PhysicsServer4D::body_get_param(RID p_body, BodyParam p_param) const {
	if (!_bodies.has(p_body)) return Variant();
	const BodyData &bd = _bodies[p_body];
	switch (p_param) {
		case BODY_PARAM_MASS: return bd.mass;
		case BODY_PARAM_FRICTION: return bd.friction;
		case BODY_PARAM_BOUNCE: return bd.bounce;
		case BODY_PARAM_GRAVITY_SCALE: return bd.gravity_scale;
		case BODY_PARAM_LINEAR_DAMP: return bd.linear_damp;
		case BODY_PARAM_ANGULAR_DAMP: return bd.angular_damp;
		default: return Variant();
	}
}

void PhysicsServer4D::body_reset_mass_properties(RID p_body) {}

void PhysicsServer4D::body_set_state(RID p_body, BodyState p_state, const Variant &p_value) {
	if (!_bodies.has(p_body)) return;
	BodyData &bd = _bodies[p_body];
	switch (p_state) {
		case BODY_STATE_TRANSFORM: bd.transform = p_value; break;
		case BODY_STATE_LINEAR_VELOCITY: bd.linear_velocity = p_value; break;
		case BODY_STATE_ANGULAR_VELOCITY: bd.angular_velocity = p_value; break;
		case BODY_STATE_SLEEPING: bd.sleeping = p_value; break;
		case BODY_STATE_CAN_SLEEP: bd.can_sleep = p_value; break;
	}
}

Variant PhysicsServer4D::body_get_state(RID p_body, BodyState p_state) const {
	if (!_bodies.has(p_body)) return Variant();
	const BodyData &bd = _bodies[p_body];
	switch (p_state) {
		case BODY_STATE_TRANSFORM: return bd.transform;
		case BODY_STATE_LINEAR_VELOCITY: return bd.linear_velocity;
		case BODY_STATE_ANGULAR_VELOCITY: return bd.angular_velocity;
		case BODY_STATE_SLEEPING: return bd.sleeping;
		case BODY_STATE_CAN_SLEEP: return bd.can_sleep;
		default: return Variant();
	}
}

void PhysicsServer4D::body_set_object_instance_id(RID p_body, uint64_t p_id) {
	if (_bodies.has(p_body)) _bodies[p_body].object_instance_id = p_id;
}

uint64_t PhysicsServer4D::body_get_object_instance_id(RID p_body) const {
	if (!_bodies.has(p_body)) return 0;
	return _bodies[p_body].object_instance_id;
}

void PhysicsServer4D::body_set_state_sync_callback(RID p_body, const Callable &p_callable) {
	if (_bodies.has(p_body)) _bodies[p_body].state_sync_callback = p_callable;
}

void PhysicsServer4D::body_apply_central_force(RID p_body, const Vector4 &p_force) {
	if (!_bodies.has(p_body)) return;
	BodyData &bd = _bodies[p_body];
	if (bd.mode != BODY_MODE_RIGID) return;
	// Accumulate linear velocity (simplified: F = ma, a = F/m)
	if (bd.mass > 0.0f) {
		bd.linear_velocity += p_force / bd.mass;
	}
}

void PhysicsServer4D::body_apply_force(RID p_body, const Vector4 &p_force, const Vector4 &p_position) {
	body_apply_central_force(p_body, p_force);
}

void PhysicsServer4D::body_apply_central_impulse(RID p_body, const Vector4 &p_impulse) {
	if (!_bodies.has(p_body)) return;
	BodyData &bd = _bodies[p_body];
	if (bd.mass > 0.0f) bd.linear_velocity += p_impulse / bd.mass;
}

void PhysicsServer4D::body_apply_impulse(RID p_body, const Vector4 &p_impulse, const Vector4 &p_position) {
	body_apply_central_impulse(p_body, p_impulse);
}

void PhysicsServer4D::body_apply_torque(RID p_body, const PackedFloat32Array &p_torque) {}
void PhysicsServer4D::body_apply_torque_impulse(RID p_body, const PackedFloat32Array &p_torque) {}
void PhysicsServer4D::body_set_axis_lock(RID p_body, int p_axis, bool p_lock) {}
bool PhysicsServer4D::body_is_axis_locked(RID p_body, int p_axis) const { return false; }

// ---- Area ----
RID PhysicsServer4D::area_create() {
	RID rid = _make_rid();
	AreaData ad;
	PackedFloat32Array t;
	t.resize(20);
	t[0]=1; t[5]=1; t[10]=1; t[15]=1;
	ad.transform = t;
	_areas[rid] = ad;
	return rid;
}

void PhysicsServer4D::area_set_space(RID p_area, RID p_space) {
	if (!_areas.has(p_area)) return;
	_areas[p_area].space = p_space;
	if (p_space.is_valid() && _spaces.has(p_space)) {
		_spaces[p_space].areas.push_back(p_area);
	}
}

RID PhysicsServer4D::area_get_space(RID p_area) const {
	if (!_areas.has(p_area)) return RID();
	return _areas[p_area].space;
}

void PhysicsServer4D::area_add_shape(RID p_area, RID p_shape, const PackedFloat32Array &p_transform, bool p_disabled) {
	if (_areas.has(p_area)) _areas[p_area].shapes.push_back(p_shape);
}

void PhysicsServer4D::area_remove_shape(RID p_area, int p_shape_idx) {
	if (_areas.has(p_area) && p_shape_idx < _areas[p_area].shapes.size()) {
		_areas[p_area].shapes.remove_at(p_shape_idx);
	}
}

void PhysicsServer4D::area_clear_shapes(RID p_area) {
	if (_areas.has(p_area)) _areas[p_area].shapes.clear();
}

int PhysicsServer4D::area_get_shape_count(RID p_area) const {
	if (!_areas.has(p_area)) return 0;
	return _areas[p_area].shapes.size();
}

void PhysicsServer4D::area_set_param(RID p_area, AreaParam p_param, const Variant &p_value) {
	if (!_areas.has(p_area)) return;
	AreaData &ad = _areas[p_area];
	switch (p_param) {
		case AREA_PARAM_GRAVITY: ad.gravity = p_value; break;
		case AREA_PARAM_GRAVITY_VECTOR: ad.gravity_vector = p_value; break;
		case AREA_PARAM_GRAVITY_IS_POINT: ad.gravity_is_point = p_value; break;
		case AREA_PARAM_PRIORITY: ad.priority = p_value; break;
		case AREA_PARAM_LINEAR_DAMP: ad.linear_damp = p_value; break;
		case AREA_PARAM_ANGULAR_DAMP: ad.angular_damp = p_value; break;
		default: break;
	}
}

Variant PhysicsServer4D::area_get_param(RID p_area, AreaParam p_param) const {
	if (!_areas.has(p_area)) return Variant();
	const AreaData &ad = _areas[p_area];
	switch (p_param) {
		case AREA_PARAM_GRAVITY: return ad.gravity;
		case AREA_PARAM_GRAVITY_VECTOR: return ad.gravity_vector;
		case AREA_PARAM_GRAVITY_IS_POINT: return ad.gravity_is_point;
		case AREA_PARAM_PRIORITY: return ad.priority;
		default: return Variant();
	}
}

void PhysicsServer4D::area_set_transform(RID p_area, const PackedFloat32Array &p_transform) {
	if (_areas.has(p_area)) _areas[p_area].transform = p_transform;
}

PackedFloat32Array PhysicsServer4D::area_get_transform(RID p_area) const {
	if (!_areas.has(p_area)) return PackedFloat32Array();
	return _areas[p_area].transform;
}

void PhysicsServer4D::area_set_monitor_callback(RID p_area, const Callable &p_callback) {
	if (_areas.has(p_area)) _areas[p_area].body_monitor_callback = p_callback;
}

void PhysicsServer4D::area_set_area_monitor_callback(RID p_area, const Callable &p_callback) {
	if (_areas.has(p_area)) _areas[p_area].area_monitor_callback = p_callback;
}

void PhysicsServer4D::area_set_monitorable(RID p_area, bool p_monitorable) {
	if (_areas.has(p_area)) _areas[p_area].monitorable = p_monitorable;
}

void PhysicsServer4D::area_set_object_instance_id(RID p_area, uint64_t p_id) {}

// ---- Shapes ----
RID PhysicsServer4D::shape_create(int p_type) {
	RID rid = _make_rid();
	ShapeData sd;
	sd.type = p_type;
	_shapes[rid] = sd;
	return rid;
}

void PhysicsServer4D::shape_set_data(RID p_shape, const PackedFloat32Array &p_data) {
	if (_shapes.has(p_shape)) _shapes[p_shape].data = p_data;
}

PackedFloat32Array PhysicsServer4D::shape_get_data(RID p_shape) const {
	if (!_shapes.has(p_shape)) return PackedFloat32Array();
	return _shapes[p_shape].data;
}

// ---- Physics step ----
Vector4 PhysicsServer4D::_compute_gravity(RID p_space, RID p_body) {
	const SpaceData &sd = _spaces[p_space];
	// Default gravity
	return sd.gravity_direction * sd.gravity_magnitude;
}

void PhysicsServer4D::_step_space(RID p_space, real_t p_delta) {
	SpaceData &sd = _spaces[p_space];

	for (RID body_rid : sd.bodies) {
		if (!_bodies.has(body_rid)) continue;
		BodyData &bd = _bodies[body_rid];

		if (bd.mode != BODY_MODE_RIGID) continue;
		if (bd.sleeping) continue;

		// Apply gravity
		Vector4 gravity = _compute_gravity(p_space, body_rid);
		bd.linear_velocity += gravity * bd.gravity_scale * p_delta;

		// Apply linear damping
		if (bd.linear_damp > 0.0f) {
			bd.linear_velocity *= (1.0f - bd.linear_damp * p_delta);
		}

		// Integrate position (update W component of transform origin)
		if (bd.transform.size() >= 20) {
			bd.transform[16] += bd.linear_velocity.x * p_delta;
			bd.transform[17] += bd.linear_velocity.y * p_delta;
			bd.transform[18] += bd.linear_velocity.z * p_delta;
			bd.transform[19] += bd.linear_velocity.w * p_delta;
		}

		// Notify state sync callback
		if (bd.state_sync_callback.is_valid()) {
			Ref<PhysicsDirectBodyState4D> state;
			state.instantiate();
			state->_body_rid = body_rid;
			state->_server = this;
			bd.state_sync_callback.call(state);
		}
	}
}

void PhysicsServer4D::step(real_t p_delta) {
	for (RID space_rid : _active_spaces) {
		if (_spaces.has(space_rid)) {
			_step_space(space_rid, p_delta);
		}
	}
}

void PhysicsServer4D::free_rid(RID p_rid) {
	if (_bodies.has(p_rid)) {
		RID space = _bodies[p_rid].space;
		if (space.is_valid() && _spaces.has(space)) {
			_spaces[space].bodies.erase(p_rid);
		}
		_bodies.erase(p_rid);
	} else if (_areas.has(p_rid)) {
		_areas.erase(p_rid);
	} else if (_shapes.has(p_rid)) {
		_shapes.erase(p_rid);
	} else if (_spaces.has(p_rid)) {
		_active_spaces.erase(p_rid);
		_spaces.erase(p_rid);
	}
}
