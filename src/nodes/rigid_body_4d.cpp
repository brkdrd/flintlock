#include "rigid_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

RigidBody4D::RigidBody4D() {}
RigidBody4D::~RigidBody4D() {}

// ---------------------------------------------------------------------------
// Create physics body
// ---------------------------------------------------------------------------

void RigidBody4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps) return;

	// Only create the RID here. Properties are pushed in _configure_physics_body()
	// which runs after body_set_space() so _get_body() can find the internal body.
	_rid = ps->body_create();
}

void RigidBody4D::_configure_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_rid.is_valid()) return;

	ps->body_set_mode(_rid, _freeze ? PhysicsServer4D::BODY_MODE_STATIC : PhysicsServer4D::BODY_MODE_RIGID);

	ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_MASS, _mass);
	ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_GRAVITY_SCALE, _gravity_scale);
	ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, _linear_damp);
	ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_ANGULAR_DAMP, _angular_damp);
	ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_SLEEPING, _sleeping);
	ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_CAN_SLEEP, _can_sleep);
	ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, _linear_velocity);

	// Register state sync callback so the server can push updates back each step.
	Callable cb(this, "_state_sync_callback");
	ps->body_set_state_sync_callback(_rid, cb);
}

// ---------------------------------------------------------------------------
// Notifications
// ---------------------------------------------------------------------------

void RigidBody4D::_notification(int p_what) {
	// CollisionObject4D handles ENTER_TREE / EXIT_TREE / TRANSFORM_4D_CHANGED.
	CollisionObject4D::_notification(p_what);
}

// ---------------------------------------------------------------------------
// State sync callback (called by PhysicsServer4D each step)
// ---------------------------------------------------------------------------

void RigidBody4D::_state_sync_callback(PhysicsDirectBodyState4D *p_state) {
	if (!p_state) return;

	// Read transform from state and push it to the node.
	PackedFloat32Array xf_arr = p_state->get_transform();
	if (xf_arr.size() == 20) {
		Ref<Basis4D> b;
		b.instantiate();
		for (int col = 0; col < 4; col++) {
			b->data[col][0] = xf_arr[col * 4 + 0];
			b->data[col][1] = xf_arr[col * 4 + 1];
			b->data[col][2] = xf_arr[col * 4 + 2];
			b->data[col][3] = xf_arr[col * 4 + 3];
		}
		Ref<Vector4D> o;
		o.instantiate();
		o->x = xf_arr[16];
		o->y = xf_arr[17];
		o->z = xf_arr[18];
		o->w = xf_arr[19];

		Ref<Transform4D> xf = Transform4D::create(b, o);
		// Use set_global_transform_4d which won't re-notify the server
		// because we set_notify_transform_4d(true) only when in tree
		// and the TRANSFORM_4D_CHANGED path checks _rid.is_valid().
		set_global_transform_4d(xf);
	}

	// Sync linear velocity back.
	_linear_velocity = p_state->get_linear_velocity();

	// Sync sleeping state.
	_sleeping = p_state->is_sleeping();
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void RigidBody4D::set_mass(float p_mass) {
	_mass = p_mass;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_MASS, p_mass);
	}
}
float RigidBody4D::get_mass() const { return _mass; }

void RigidBody4D::set_gravity_scale(float p_scale) {
	_gravity_scale = p_scale;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_GRAVITY_SCALE, p_scale);
	}
}
float RigidBody4D::get_gravity_scale() const { return _gravity_scale; }

void RigidBody4D::set_linear_damp(float p_damp) {
	_linear_damp = p_damp;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_LINEAR_DAMP, p_damp);
	}
}
float RigidBody4D::get_linear_damp() const { return _linear_damp; }

void RigidBody4D::set_angular_damp(float p_damp) {
	_angular_damp = p_damp;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_param(_rid, PhysicsServer4D::BODY_PARAM_ANGULAR_DAMP, p_damp);
	}
}
float RigidBody4D::get_angular_damp() const { return _angular_damp; }

void RigidBody4D::set_sleeping(bool p_sleeping) {
	_sleeping = p_sleeping;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_SLEEPING, p_sleeping);
	}
}
bool RigidBody4D::get_sleeping() const { return _sleeping; }

void RigidBody4D::set_can_sleep(bool p_can_sleep) {
	_can_sleep = p_can_sleep;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_CAN_SLEEP, p_can_sleep);
	}
}
bool RigidBody4D::get_can_sleep() const { return _can_sleep; }

void RigidBody4D::set_freeze(bool p_freeze) {
	_freeze = p_freeze;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) {
			ps->body_set_mode(_rid, p_freeze
				? PhysicsServer4D::BODY_MODE_STATIC
				: PhysicsServer4D::BODY_MODE_RIGID);
		}
	}
}
bool RigidBody4D::get_freeze() const { return _freeze; }

void RigidBody4D::set_linear_velocity(const Vector4 &p_velocity) {
	_linear_velocity = p_velocity;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY, p_velocity);
	}
}
Vector4 RigidBody4D::get_linear_velocity() const { return _linear_velocity; }

void RigidBody4D::set_constant_force(const Vector4 &p_force) {
	_constant_force = p_force;
	// Store for now; full integration would pass this to the server each step.
}
Vector4 RigidBody4D::get_constant_force() const { return _constant_force; }

// ---------------------------------------------------------------------------
// Force / impulse methods
// ---------------------------------------------------------------------------

void RigidBody4D::apply_central_force(const Vector4 &p_force) {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) ps->body_apply_central_force(_rid, p_force);
}

void RigidBody4D::apply_force(const Vector4 &p_force, const Vector4 &p_position) {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) ps->body_apply_force(_rid, p_force, p_position);
}

void RigidBody4D::apply_central_impulse(const Vector4 &p_impulse) {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) ps->body_apply_central_impulse(_rid, p_impulse);
}

void RigidBody4D::apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position) {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) ps->body_apply_impulse(_rid, p_impulse, p_position);
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void RigidBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_mass", "mass"), &RigidBody4D::set_mass);
	ClassDB::bind_method(D_METHOD("get_mass"), &RigidBody4D::get_mass);
	ClassDB::bind_method(D_METHOD("set_gravity_scale", "scale"), &RigidBody4D::set_gravity_scale);
	ClassDB::bind_method(D_METHOD("get_gravity_scale"), &RigidBody4D::get_gravity_scale);
	ClassDB::bind_method(D_METHOD("set_linear_damp", "damp"), &RigidBody4D::set_linear_damp);
	ClassDB::bind_method(D_METHOD("get_linear_damp"), &RigidBody4D::get_linear_damp);
	ClassDB::bind_method(D_METHOD("set_angular_damp", "damp"), &RigidBody4D::set_angular_damp);
	ClassDB::bind_method(D_METHOD("get_angular_damp"), &RigidBody4D::get_angular_damp);
	ClassDB::bind_method(D_METHOD("set_sleeping", "sleeping"), &RigidBody4D::set_sleeping);
	ClassDB::bind_method(D_METHOD("get_sleeping"), &RigidBody4D::get_sleeping);
	ClassDB::bind_method(D_METHOD("set_can_sleep", "can_sleep"), &RigidBody4D::set_can_sleep);
	ClassDB::bind_method(D_METHOD("get_can_sleep"), &RigidBody4D::get_can_sleep);
	ClassDB::bind_method(D_METHOD("set_freeze", "freeze"), &RigidBody4D::set_freeze);
	ClassDB::bind_method(D_METHOD("get_freeze"), &RigidBody4D::get_freeze);
	ClassDB::bind_method(D_METHOD("set_linear_velocity", "velocity"), &RigidBody4D::set_linear_velocity);
	ClassDB::bind_method(D_METHOD("get_linear_velocity"), &RigidBody4D::get_linear_velocity);
	ClassDB::bind_method(D_METHOD("set_constant_force", "force"), &RigidBody4D::set_constant_force);
	ClassDB::bind_method(D_METHOD("get_constant_force"), &RigidBody4D::get_constant_force);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "mass"), "set_mass", "get_mass");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity_scale"), "set_gravity_scale", "get_gravity_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_damp"), "set_linear_damp", "get_linear_damp");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_damp"), "set_angular_damp", "get_angular_damp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sleeping"), "set_sleeping", "get_sleeping");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_sleep"), "set_can_sleep", "get_can_sleep");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "freeze"), "set_freeze", "get_freeze");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "linear_velocity"), "set_linear_velocity", "get_linear_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "constant_force"), "set_constant_force", "get_constant_force");

	// Force / impulse
	ClassDB::bind_method(D_METHOD("apply_central_force", "force"), &RigidBody4D::apply_central_force);
	ClassDB::bind_method(D_METHOD("apply_force", "force", "position"), &RigidBody4D::apply_force);
	ClassDB::bind_method(D_METHOD("apply_central_impulse", "impulse"), &RigidBody4D::apply_central_impulse);
	ClassDB::bind_method(D_METHOD("apply_impulse", "impulse", "position"), &RigidBody4D::apply_impulse);

	// Internal callback (must be bound so Callable works)
	ClassDB::bind_method(D_METHOD("_state_sync_callback", "state"), &RigidBody4D::_state_sync_callback);
}
