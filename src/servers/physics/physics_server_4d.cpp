#include "physics_server_4d.h"
#include "physics_direct_space_state_4d.h"
#include "physics_direct_body_state_4d.h"
#include <godot_cpp/core/class_db.hpp>

PhysicsServer4D *PhysicsServer4D::singleton = nullptr;

PhysicsServer4D::PhysicsServer4D() { singleton = this; }
PhysicsServer4D::~PhysicsServer4D() { singleton = nullptr; }
PhysicsServer4D *PhysicsServer4D::get_singleton() { return singleton; }

// ============================================================================
// Internal helpers
// ============================================================================

RigidBody4DInternal *PhysicsServer4D::_get_body(RID p_rid) {
	auto it = _rid_map.find(p_rid);
	if (it == _rid_map.end() || it->value.type != RIDData::BODY) return nullptr;
	RID space_rid = it->value.space_rid;
	auto sp_it = _spaces.find(space_rid);
	if (sp_it == _spaces.end()) return nullptr;
	int idx = it->value.internal_id;
	if (idx < 0 || idx >= (int)sp_it->value->bodies.size()) return nullptr;
	return &sp_it->value->bodies[idx];
}

const RigidBody4DInternal *PhysicsServer4D::_get_body(RID p_rid) const {
	return const_cast<PhysicsServer4D*>(this)->_get_body(p_rid);
}

Area4DInternal *PhysicsServer4D::_get_area(RID p_rid) {
	auto it = _rid_map.find(p_rid);
	if (it == _rid_map.end() || it->value.type != RIDData::AREA) return nullptr;
	RID space_rid = it->value.space_rid;
	auto sp_it = _spaces.find(space_rid);
	if (sp_it == _spaces.end()) return nullptr;
	int idx = it->value.internal_id;
	if (idx < 0 || idx >= (int)sp_it->value->areas.size()) return nullptr;
	return &sp_it->value->areas[idx];
}

Space4DInternal *PhysicsServer4D::_get_space(RID p_rid) {
	auto it = _spaces.find(p_rid);
	if (it == _spaces.end()) return nullptr;
	return it->value.get();
}

const Space4DInternal *PhysicsServer4D::_get_space(RID p_rid) const {
	return const_cast<PhysicsServer4D*>(this)->_get_space(p_rid);
}

RID PhysicsServer4D::get_default_space() {
	if (!_default_space.is_valid()) {
		_default_space = space_create();
		space_set_active(_default_space, true);
	}
	return _default_space;
}

Space4DInternal *PhysicsServer4D::get_space_internal(RID p_space) {
	return _get_space(p_space);
}

RigidBody4DInternal *PhysicsServer4D::get_body_internal(RID p_body) {
	return _get_body(p_body);
}

std::shared_ptr<Shape4DInternal> PhysicsServer4D::_create_internal_shape(int p_type, const PackedFloat32Array &p_data) {
	switch (p_type) {
		case 0: { // Box
			auto s = std::make_shared<BoxShape4DInternal>();
			if (p_data.size() >= 4) {
				s->half_extents = Vector4(p_data[0]*0.5f, p_data[1]*0.5f, p_data[2]*0.5f, p_data[3]*0.5f);
			}
			return s;
		}
		case 1: { // Sphere
			auto s = std::make_shared<SphereShape4DInternal>();
			if (p_data.size() >= 1) s->radius = p_data[0];
			return s;
		}
		case 2: { // Capsule
			auto s = std::make_shared<CapsuleShape4DInternal>();
			if (p_data.size() >= 2) {
				s->radius = p_data[0];
				s->height = p_data[1];
			}
			return s;
		}
		case 3: { // Cylinder
			auto s = std::make_shared<CylinderShape4DInternal>();
			if (p_data.size() >= 2) {
				s->radius = p_data[0];
				s->height = p_data[1];
			}
			return s;
		}
		case 4: { // Convex hull
			auto s = std::make_shared<ConvexHullShape4DInternal>();
			for (int i = 0; i + 3 < (int)p_data.size(); i += 4) {
				s->points.push_back(Vector4(p_data[i], p_data[i+1], p_data[i+2], p_data[i+3]));
			}
			return s;
		}
		case 5: { // Concave mesh
			auto s = std::make_shared<ConcaveMeshShape4DInternal>();
			// Data is groups of 16 floats (4 vertices * 4 components)
			for (int i = 0; i + 15 < (int)p_data.size(); i += 16) {
				for (int v = 0; v < 4; v++) {
					s->vertices.push_back(Vector4(p_data[i+v*4], p_data[i+v*4+1], p_data[i+v*4+2], p_data[i+v*4+3]));
				}
				ConcaveMeshShape4DInternal::Tetrahedron tet;
				int base = (int)s->vertices.size() - 4;
				tet.idx[0] = base; tet.idx[1] = base+1; tet.idx[2] = base+2; tet.idx[3] = base+3;
				s->tetrahedra.push_back(tet);
			}
			return s;
		}
		case 6: { // World boundary
			auto s = std::make_shared<WorldBoundaryShape4DInternal>();
			if (p_data.size() >= 5) {
				s->normal = Vector4(p_data[0], p_data[1], p_data[2], p_data[3]);
				s->distance = p_data[4];
			}
			return s;
		}
		case 7: { // Separation ray
			auto s = std::make_shared<SeparationRayShape4DInternal>();
			if (p_data.size() >= 1) s->length = p_data[0];
			return s;
		}
		default:
			return std::make_shared<SphereShape4DInternal>();
	}
}

// ============================================================================
// Bindings
// ============================================================================

void PhysicsServer4D::_bind_methods() {
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

	BIND_ENUM_CONSTANT(JOINT_TYPE_PIN);
	BIND_ENUM_CONSTANT(JOINT_TYPE_HINGE);
	BIND_ENUM_CONSTANT(JOINT_TYPE_SLIDER);
	BIND_ENUM_CONSTANT(JOINT_TYPE_CONE_TWIST);
	BIND_ENUM_CONSTANT(JOINT_TYPE_GENERIC_10DOF);

	// Default space
	ClassDB::bind_method(D_METHOD("get_default_space"), &PhysicsServer4D::get_default_space);

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
	ClassDB::bind_method(D_METHOD("body_apply_torque", "body", "torque"), &PhysicsServer4D::body_apply_torque);
	ClassDB::bind_method(D_METHOD("body_apply_torque_impulse", "body", "torque"), &PhysicsServer4D::body_apply_torque_impulse);
	ClassDB::bind_method(D_METHOD("body_set_axis_lock", "body", "axis", "lock"), &PhysicsServer4D::body_set_axis_lock);
	ClassDB::bind_method(D_METHOD("body_is_axis_locked", "body", "axis"), &PhysicsServer4D::body_is_axis_locked);

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

	// Joint API
	ClassDB::bind_method(D_METHOD("joint_create", "type", "body_a", "body_b"), &PhysicsServer4D::joint_create);
	ClassDB::bind_method(D_METHOD("joint_set_anchor_a", "joint", "anchor"), &PhysicsServer4D::joint_set_anchor_a);
	ClassDB::bind_method(D_METHOD("joint_set_anchor_b", "joint", "anchor"), &PhysicsServer4D::joint_set_anchor_b);

	// Step
	ClassDB::bind_method(D_METHOD("step", "delta"), &PhysicsServer4D::step);
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &PhysicsServer4D::free_rid);
}

// ============================================================================
// Space API
// ============================================================================

RID PhysicsServer4D::space_create() {
	RID rid = _make_rid();
	auto space = std::make_shared<Space4DInternal>();
	_spaces[rid] = space;

	Ref<PhysicsDirectSpaceState4D> ds;
	ds.instantiate();
	ds->_space_rid = rid;
	ds->_server = this;
	_space_direct_states[rid] = ds;

	return rid;
}

void PhysicsServer4D::space_set_active(RID p_space, bool p_active) {
	if (!_spaces.has(p_space)) return;
	_spaces[p_space]->active = p_active;
	if (p_active && !_active_spaces.has(p_space)) {
		_active_spaces.push_back(p_space);
	} else if (!p_active) {
		_active_spaces.erase(p_space);
	}
}

bool PhysicsServer4D::space_is_active(RID p_space) const {
	auto it = _spaces.find(p_space);
	if (it == _spaces.end()) return false;
	return it->value->active;
}

void PhysicsServer4D::space_set_param(RID p_space, SpaceParam p_param, real_t p_value) {
	auto *space = _get_space(p_space);
	if (!space) return;
	switch (p_param) {
		case SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD:
			space->body_linear_velocity_sleep_threshold = p_value; break;
		case SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD:
			space->body_angular_velocity_sleep_threshold = p_value; break;
		case SPACE_PARAM_BODY_TIME_TO_SLEEP:
			space->body_time_to_sleep = p_value; break;
		case SPACE_PARAM_SOLVER_ITERATIONS:
			space->solver_iterations = (int)p_value; break;
		default: break;
	}
}

real_t PhysicsServer4D::space_get_param(RID p_space, SpaceParam p_param) const {
	const auto *space = _get_space(p_space);
	if (!space) return 0.0f;
	switch (p_param) {
		case SPACE_PARAM_SOLVER_ITERATIONS: return space->solver_iterations;
		case SPACE_PARAM_BODY_TIME_TO_SLEEP: return space->body_time_to_sleep;
		default: return 0.0f;
	}
}

Ref<PhysicsDirectSpaceState4D> PhysicsServer4D::space_get_direct_state(RID p_space) {
	auto it = _space_direct_states.find(p_space);
	if (it == _space_direct_states.end()) return Ref<PhysicsDirectSpaceState4D>();
	return it->value;
}

// ============================================================================
// Body API
// ============================================================================

RID PhysicsServer4D::body_create() {
	RID rid = _make_rid();
	RIDData data;
	data.type = RIDData::BODY;
	data.internal_id = -1; // Not yet assigned to a space
	_rid_map[rid] = data;
	return rid;
}

void PhysicsServer4D::body_set_space(RID p_body, RID p_space) {
	auto it = _rid_map.find(p_body);
	if (it == _rid_map.end() || it->value.type != RIDData::BODY) return;

	// Remove from old space
	if (it->value.space_rid.is_valid() && it->value.internal_id >= 0) {
		// We don't actually remove from internal arrays to avoid index invalidation
		// Instead mark as inactive (TODO: proper removal with index compaction)
	}

	if (!p_space.is_valid() || !_spaces.has(p_space)) {
		it->value.space_rid = RID();
		it->value.internal_id = -1;
		return;
	}

	auto &space = _spaces[p_space];
	RigidBody4DInternal body;
	body.id = (int)space->bodies.size();
	space->body_index_map[body.id] = body.id;
	space->bodies.push_back(body);

	it->value.space_rid = p_space;
	it->value.internal_id = body.id;
}

RID PhysicsServer4D::body_get_space(RID p_body) const {
	auto it = _rid_map.find(p_body);
	if (it == _rid_map.end() || it->value.type != RIDData::BODY) return RID();
	return it->value.space_rid;
}

void PhysicsServer4D::body_set_mode(RID p_body, BodyMode p_mode) {
	auto *body = _get_body(p_body);
	if (!body) return;
	body->mode = (BodyMode4D)p_mode;
	body->recompute_inertia();
}

PhysicsServer4D::BodyMode PhysicsServer4D::body_get_mode(RID p_body) const {
	const auto *body = _get_body(p_body);
	if (!body) return BODY_MODE_STATIC;
	return (BodyMode)body->mode;
}

void PhysicsServer4D::body_add_shape(RID p_body, RID p_shape, const PackedFloat32Array &p_transform, bool p_disabled) {
	auto *body = _get_body(p_body);
	if (!body) return;

	auto shape_it = _shapes.find(p_shape);
	if (shape_it == _shapes.end()) return;

	ShapeInstance4D si;
	si.shape = shape_it->value;
	si.disabled = p_disabled;

	// Parse local transform if provided
	if (p_transform.size() >= 20) {
		si.local_transform = Xform4::from_packed(p_transform.ptr());
	}

	body->shapes.push_back(si);
	body->recompute_inertia();
}

void PhysicsServer4D::body_remove_shape(RID p_body, int p_shape_idx) {
	auto *body = _get_body(p_body);
	if (!body || p_shape_idx < 0 || p_shape_idx >= (int)body->shapes.size()) return;
	body->shapes.erase(body->shapes.begin() + p_shape_idx);
	body->recompute_inertia();
}

void PhysicsServer4D::body_clear_shapes(RID p_body) {
	auto *body = _get_body(p_body);
	if (!body) return;
	body->shapes.clear();
	body->recompute_inertia();
}

int PhysicsServer4D::body_get_shape_count(RID p_body) const {
	const auto *body = _get_body(p_body);
	if (!body) return 0;
	return (int)body->shapes.size();
}

void PhysicsServer4D::body_set_shape_disabled(RID p_body, int p_shape_idx, bool p_disabled) {
	auto *body = _get_body(p_body);
	if (!body || p_shape_idx < 0 || p_shape_idx >= (int)body->shapes.size()) return;
	body->shapes[p_shape_idx].disabled = p_disabled;
}

void PhysicsServer4D::body_set_param(RID p_body, BodyParam p_param, const Variant &p_value) {
	auto *body = _get_body(p_body);
	if (!body) return;
	switch (p_param) {
		case BODY_PARAM_MASS: body->set_mass(p_value); break;
		case BODY_PARAM_FRICTION: body->friction = p_value; break;
		case BODY_PARAM_BOUNCE: body->bounce = p_value; break;
		case BODY_PARAM_GRAVITY_SCALE: body->gravity_scale = p_value; break;
		case BODY_PARAM_LINEAR_DAMP: body->linear_damp = p_value; break;
		case BODY_PARAM_ANGULAR_DAMP: body->angular_damp = p_value; break;
		default: break;
	}
}

Variant PhysicsServer4D::body_get_param(RID p_body, BodyParam p_param) const {
	const auto *body = _get_body(p_body);
	if (!body) return Variant();
	switch (p_param) {
		case BODY_PARAM_MASS: return body->mass;
		case BODY_PARAM_FRICTION: return body->friction;
		case BODY_PARAM_BOUNCE: return body->bounce;
		case BODY_PARAM_GRAVITY_SCALE: return body->gravity_scale;
		case BODY_PARAM_LINEAR_DAMP: return body->linear_damp;
		case BODY_PARAM_ANGULAR_DAMP: return body->angular_damp;
		default: return Variant();
	}
}

void PhysicsServer4D::body_reset_mass_properties(RID p_body) {
	auto *body = _get_body(p_body);
	if (body) body->recompute_inertia();
}

void PhysicsServer4D::body_set_state(RID p_body, BodyState p_state, const Variant &p_value) {
	auto *body = _get_body(p_body);
	if (!body) return;
	switch (p_state) {
		case BODY_STATE_TRANSFORM: {
			PackedFloat32Array arr = p_value;
			if (arr.size() >= 20) {
				body->transform = Xform4::from_packed(arr.ptr());
				body->update_world_inertia();
			}
		} break;
		case BODY_STATE_LINEAR_VELOCITY:
			body->linear_velocity = p_value;
			break;
		case BODY_STATE_ANGULAR_VELOCITY: {
			PackedFloat32Array arr = p_value;
			if (arr.size() >= 6) {
				for (int i = 0; i < 6; i++) body->angular_velocity.v[i] = arr[i];
			}
		} break;
		case BODY_STATE_SLEEPING:
			body->sleeping = p_value;
			if (!body->sleeping) body->wake_up();
			break;
		case BODY_STATE_CAN_SLEEP:
			body->can_sleep = p_value;
			break;
	}
}

Variant PhysicsServer4D::body_get_state(RID p_body, BodyState p_state) const {
	const auto *body = _get_body(p_body);
	if (!body) return Variant();
	switch (p_state) {
		case BODY_STATE_TRANSFORM: {
			PackedFloat32Array arr;
			arr.resize(20);
			body->transform.to_packed(arr.ptrw());
			return arr;
		}
		case BODY_STATE_LINEAR_VELOCITY:
			return body->linear_velocity;
		case BODY_STATE_ANGULAR_VELOCITY: {
			PackedFloat32Array arr;
			arr.resize(6);
			for (int i = 0; i < 6; i++) arr.set(i, body->angular_velocity.v[i]);
			return arr;
		}
		case BODY_STATE_SLEEPING:
			return body->sleeping;
		case BODY_STATE_CAN_SLEEP:
			return body->can_sleep;
		default:
			return Variant();
	}
}

void PhysicsServer4D::body_set_object_instance_id(RID p_body, uint64_t p_id) {
	auto *body = _get_body(p_body);
	if (body) body->object_instance_id = p_id;
}

uint64_t PhysicsServer4D::body_get_object_instance_id(RID p_body) const {
	const auto *body = _get_body(p_body);
	return body ? body->object_instance_id : 0;
}

void PhysicsServer4D::body_set_state_sync_callback(RID p_body, const Callable &p_callable) {
	auto *body = _get_body(p_body);
	if (body) body->state_sync_callback = p_callable;
}

void PhysicsServer4D::body_apply_central_force(RID p_body, const Vector4 &p_force) {
	auto *body = _get_body(p_body);
	if (body) body->apply_central_force(p_force);
}

void PhysicsServer4D::body_apply_force(RID p_body, const Vector4 &p_force, const Vector4 &p_position) {
	auto *body = _get_body(p_body);
	if (body) body->apply_force(p_force, p_position);
}

void PhysicsServer4D::body_apply_central_impulse(RID p_body, const Vector4 &p_impulse) {
	auto *body = _get_body(p_body);
	if (body) body->apply_central_impulse(p_impulse);
}

void PhysicsServer4D::body_apply_impulse(RID p_body, const Vector4 &p_impulse, const Vector4 &p_position) {
	auto *body = _get_body(p_body);
	if (body) body->apply_impulse(p_impulse, p_position);
}

void PhysicsServer4D::body_apply_torque(RID p_body, const PackedFloat32Array &p_torque) {
	auto *body = _get_body(p_body);
	if (!body || p_torque.size() < 6) return;
	Bivec4 torque;
	for (int i = 0; i < 6; i++) torque.v[i] = p_torque[i];
	body->applied_torque += torque;
}

void PhysicsServer4D::body_apply_torque_impulse(RID p_body, const PackedFloat32Array &p_torque) {
	auto *body = _get_body(p_body);
	if (!body || p_torque.size() < 6) return;
	Bivec4 torque;
	for (int i = 0; i < 6; i++) torque.v[i] = p_torque[i];
	body->apply_torque_impulse(torque);
}

void PhysicsServer4D::body_set_axis_lock(RID p_body, int p_axis, bool p_lock) {
	auto *body = _get_body(p_body);
	if (!body) return;
	if (p_axis >= 0 && p_axis < 4) {
		body->axis_lock[p_axis] = p_lock;
	} else if (p_axis >= 4 && p_axis < 10) {
		body->plane_lock[p_axis - 4] = p_lock;
	}
}

bool PhysicsServer4D::body_is_axis_locked(RID p_body, int p_axis) const {
	const auto *body = _get_body(p_body);
	if (!body) return false;
	if (p_axis >= 0 && p_axis < 4) return body->axis_lock[p_axis];
	if (p_axis >= 4 && p_axis < 10) return body->plane_lock[p_axis - 4];
	return false;
}

void PhysicsServer4D::body_set_collision_layer(RID p_body, uint32_t p_layer) {
	auto *body = _get_body(p_body);
	if (body) body->collision_layer = p_layer;
}

void PhysicsServer4D::body_set_collision_mask(RID p_body, uint32_t p_mask) {
	auto *body = _get_body(p_body);
	if (body) body->collision_mask = p_mask;
}

// ============================================================================
// Area API
// ============================================================================

RID PhysicsServer4D::area_create() {
	RID rid = _make_rid();
	RIDData data;
	data.type = RIDData::AREA;
	data.internal_id = -1;
	_rid_map[rid] = data;
	return rid;
}

void PhysicsServer4D::area_set_space(RID p_area, RID p_space) {
	auto it = _rid_map.find(p_area);
	if (it == _rid_map.end() || it->value.type != RIDData::AREA) return;

	if (!p_space.is_valid() || !_spaces.has(p_space)) {
		it->value.space_rid = RID();
		it->value.internal_id = -1;
		return;
	}

	auto &space = _spaces[p_space];
	Area4DInternal area;
	area.id = (int)space->areas.size();
	space->area_index_map[area.id] = area.id;
	space->areas.push_back(area);

	it->value.space_rid = p_space;
	it->value.internal_id = area.id;
}

RID PhysicsServer4D::area_get_space(RID p_area) const {
	auto it = _rid_map.find(p_area);
	if (it == _rid_map.end() || it->value.type != RIDData::AREA) return RID();
	return it->value.space_rid;
}

void PhysicsServer4D::area_add_shape(RID p_area, RID p_shape, const PackedFloat32Array &p_transform, bool p_disabled) {
	auto *area = _get_area(p_area);
	if (!area) return;

	auto shape_it = _shapes.find(p_shape);
	if (shape_it == _shapes.end()) return;

	ShapeInstance4D si;
	si.shape = shape_it->value;
	si.disabled = p_disabled;
	if (p_transform.size() >= 20) {
		si.local_transform = Xform4::from_packed(p_transform.ptr());
	}
	area->shapes.push_back(si);
}

void PhysicsServer4D::area_remove_shape(RID p_area, int p_shape_idx) {
	auto *area = _get_area(p_area);
	if (!area || p_shape_idx < 0 || p_shape_idx >= (int)area->shapes.size()) return;
	area->shapes.erase(area->shapes.begin() + p_shape_idx);
}

void PhysicsServer4D::area_clear_shapes(RID p_area) {
	auto *area = _get_area(p_area);
	if (area) area->shapes.clear();
}

int PhysicsServer4D::area_get_shape_count(RID p_area) const {
	auto it = _rid_map.find(p_area);
	if (it == _rid_map.end() || it->value.type != RIDData::AREA) return 0;
	const auto *area = const_cast<PhysicsServer4D*>(this)->_get_area(p_area);
	return area ? (int)area->shapes.size() : 0;
}

void PhysicsServer4D::area_set_param(RID p_area, AreaParam p_param, const Variant &p_value) {
	auto *area = _get_area(p_area);
	if (!area) return;
	switch (p_param) {
		case AREA_PARAM_GRAVITY: area->gravity = p_value; break;
		case AREA_PARAM_GRAVITY_VECTOR: area->gravity_direction = p_value; break;
		case AREA_PARAM_GRAVITY_IS_POINT: area->gravity_is_point = p_value; break;
		case AREA_PARAM_GRAVITY_OVERRIDE_MODE: area->gravity_override_mode = p_value; break;
		case AREA_PARAM_PRIORITY: area->priority = p_value; break;
		case AREA_PARAM_LINEAR_DAMP: area->linear_damp = p_value; break;
		case AREA_PARAM_ANGULAR_DAMP: area->angular_damp = p_value; break;
		case AREA_PARAM_LINEAR_DAMP_OVERRIDE_MODE: area->linear_damp_override_mode = p_value; break;
		case AREA_PARAM_ANGULAR_DAMP_OVERRIDE_MODE: area->angular_damp_override_mode = p_value; break;
		default: break;
	}
}

Variant PhysicsServer4D::area_get_param(RID p_area, AreaParam p_param) const {
	const auto *area = const_cast<PhysicsServer4D*>(this)->_get_area(const_cast<RID&>(p_area));
	if (!area) return Variant();
	switch (p_param) {
		case AREA_PARAM_GRAVITY: return area->gravity;
		case AREA_PARAM_GRAVITY_VECTOR: return area->gravity_direction;
		case AREA_PARAM_GRAVITY_IS_POINT: return area->gravity_is_point;
		case AREA_PARAM_PRIORITY: return area->priority;
		default: return Variant();
	}
}

void PhysicsServer4D::area_set_transform(RID p_area, const PackedFloat32Array &p_transform) {
	auto *area = _get_area(p_area);
	if (!area || p_transform.size() < 20) return;
	area->transform = Xform4::from_packed(p_transform.ptr());
}

PackedFloat32Array PhysicsServer4D::area_get_transform(RID p_area) const {
	const auto *area = const_cast<PhysicsServer4D*>(this)->_get_area(const_cast<RID&>(p_area));
	if (!area) return PackedFloat32Array();
	PackedFloat32Array arr;
	arr.resize(20);
	area->transform.to_packed(arr.ptrw());
	return arr;
}

void PhysicsServer4D::area_set_monitor_callback(RID p_area, const Callable &p_callback) {
	auto *area = _get_area(p_area);
	if (area) area->body_monitor_callback = p_callback;
}

void PhysicsServer4D::area_set_area_monitor_callback(RID p_area, const Callable &p_callback) {
	auto *area = _get_area(p_area);
	if (area) area->area_monitor_callback = p_callback;
}

void PhysicsServer4D::area_set_monitorable(RID p_area, bool p_monitorable) {
	auto *area = _get_area(p_area);
	if (area) area->monitorable = p_monitorable;
}

void PhysicsServer4D::area_set_object_instance_id(RID p_area, uint64_t p_id) {
	auto *area = _get_area(p_area);
	if (area) area->object_instance_id = p_id;
}

// ============================================================================
// Shape API
// ============================================================================

RID PhysicsServer4D::shape_create(int p_type) {
	RID rid = _make_rid();
	_shapes[rid] = _create_internal_shape(p_type, PackedFloat32Array());

	RIDData data;
	data.type = RIDData::SHAPE;
	data.internal_id = p_type;
	_rid_map[rid] = data;

	return rid;
}

void PhysicsServer4D::shape_set_data(RID p_shape, const PackedFloat32Array &p_data) {
	auto it = _rid_map.find(p_shape);
	if (it == _rid_map.end() || it->value.type != RIDData::SHAPE) return;
	_shapes[p_shape] = _create_internal_shape(it->value.internal_id, p_data);
}

PackedFloat32Array PhysicsServer4D::shape_get_data(RID p_shape) const {
	// Return empty for now - shape data is stored internally
	return PackedFloat32Array();
}

// ============================================================================
// Joint API
// ============================================================================

RID PhysicsServer4D::joint_create(JointType p_type, RID p_body_a, RID p_body_b) {
	auto *body_a = _get_body(p_body_a);
	auto *body_b = _get_body(p_body_b);
	if (!body_a || !body_b) return RID();

	auto it_a = _rid_map.find(p_body_a);
	auto it_b = _rid_map.find(p_body_b);
	if (!it_a->value.space_rid.is_valid()) return RID();
	auto *space = _get_space(it_a->value.space_rid);
	if (!space) return RID();

	std::shared_ptr<Joint4DInternal> joint;
	switch (p_type) {
		case JOINT_TYPE_PIN: joint = std::make_shared<PinJoint4DInternal>(); break;
		case JOINT_TYPE_HINGE: joint = std::make_shared<HingeJoint4DInternal>(); break;
		case JOINT_TYPE_SLIDER: joint = std::make_shared<SliderJoint4DInternal>(); break;
		case JOINT_TYPE_CONE_TWIST: joint = std::make_shared<ConeTwistJoint4DInternal>(); break;
		case JOINT_TYPE_GENERIC_10DOF: joint = std::make_shared<Generic10DOFJoint4DInternal>(); break;
	}

	if (!joint) return RID();

	joint->body_a_index = it_a->value.internal_id;
	joint->body_b_index = it_b->value.internal_id;

	space->joints.push_back(joint);

	RID rid = _make_rid();
	RIDData data;
	data.type = RIDData::JOINT;
	data.internal_id = (int)space->joints.size() - 1;
	data.space_rid = it_a->value.space_rid;
	_rid_map[rid] = data;

	return rid;
}

void PhysicsServer4D::joint_set_param(RID p_joint, const String &p_param, const Variant &p_value) {
	// Generic parameter setter for joints - can be extended
}

void PhysicsServer4D::joint_set_anchor_a(RID p_joint, const Vector4 &p_anchor) {
	auto it = _rid_map.find(p_joint);
	if (it == _rid_map.end() || it->value.type != RIDData::JOINT) return;
	auto *space = _get_space(it->value.space_rid);
	if (!space) return;
	int idx = it->value.internal_id;
	if (idx < 0 || idx >= (int)space->joints.size()) return;
	space->joints[idx]->local_anchor_a = p_anchor;
}

void PhysicsServer4D::joint_set_anchor_b(RID p_joint, const Vector4 &p_anchor) {
	auto it = _rid_map.find(p_joint);
	if (it == _rid_map.end() || it->value.type != RIDData::JOINT) return;
	auto *space = _get_space(it->value.space_rid);
	if (!space) return;
	int idx = it->value.internal_id;
	if (idx < 0 || idx >= (int)space->joints.size()) return;
	space->joints[idx]->local_anchor_b = p_anchor;
}

// ============================================================================
// Physics step
// ============================================================================

void PhysicsServer4D::_fire_body_callbacks(Space4DInternal &space) {
	for (auto &body : space.bodies) {
		if (!body.is_dynamic()) continue;
		if (!body.state_sync_callback.is_valid()) continue;

		Ref<PhysicsDirectBodyState4D> state;
		state.instantiate();
		state->_server = this;

		// Find the body's RID
		for (auto &kv : _rid_map) {
			if (kv.value.type == RIDData::BODY &&
				kv.value.space_rid.is_valid() &&
				kv.value.internal_id == body.id) {
				state->_body_rid = kv.key;
				break;
			}
		}

		body.state_sync_callback.call(state);
	}
}

void PhysicsServer4D::_step_space(RID p_space_rid, real_t p_delta) {
	auto *space = _get_space(p_space_rid);
	if (!space) return;

	space->step(p_delta);
	_fire_body_callbacks(*space);
}

void PhysicsServer4D::step(real_t p_delta) {
	for (int i = 0; i < _active_spaces.size(); i++) {
		_step_space(_active_spaces[i], p_delta);
	}
}

// ============================================================================
// Free
// ============================================================================

void PhysicsServer4D::free_rid(RID p_rid) {
	auto it = _rid_map.find(p_rid);
	if (it != _rid_map.end()) {
		// Clean up based on type
		switch (it->value.type) {
			case RIDData::BODY:
			case RIDData::AREA:
				// Mark as removed in the space
				// (Full cleanup would require index compaction)
				break;
			case RIDData::SHAPE:
				_shapes.erase(p_rid);
				break;
			case RIDData::JOINT:
				break;
			case RIDData::SPACE:
				_spaces.erase(p_rid);
				_space_direct_states.erase(p_rid);
				_active_spaces.erase(p_rid);
				break;
		}
		_rid_map.erase(p_rid);
		return;
	}

	// Also check direct space map
	if (_spaces.has(p_rid)) {
		_spaces.erase(p_rid);
		_space_direct_states.erase(p_rid);
		_active_spaces.erase(p_rid);
	}
}
