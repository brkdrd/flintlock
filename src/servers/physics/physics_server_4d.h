#pragma once
#include <cstring>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include "physics_direct_body_state_4d.h"
#include "physics_direct_space_state_4d.h"

using namespace godot;

// ============================================================
// PhysicsServer4D
//
// Independent 4D physics server. NOT a replacement for PhysicsServer3D.
// Registered as a singleton named "PhysicsServer4D".
//
// This is a functional stub with the correct Godot-facing API.
// A full implementation would fork Jolt Physics to 4D (Stage 5 of spec).
// ============================================================
class PhysicsServer4D : public Object {
	GDCLASS(PhysicsServer4D, Object);

	static PhysicsServer4D *singleton;

public:
	// Body types (analog of PhysicsServer3D::BodyMode)
	enum BodyMode {
		BODY_MODE_STATIC,
		BODY_MODE_KINEMATIC,
		BODY_MODE_RIGID,
		BODY_MODE_RIGID_LINEAR,
	};

	// Body parameters (analog of PhysicsServer3D::BodyParameter)
	enum BodyParam {
		BODY_PARAM_BOUNCE,
		BODY_PARAM_FRICTION,
		BODY_PARAM_MASS,
		BODY_PARAM_INERTIA,
		BODY_PARAM_CENTER_OF_MASS,
		BODY_PARAM_GRAVITY_SCALE,
		BODY_PARAM_LINEAR_DAMP_MODE,
		BODY_PARAM_ANGULAR_DAMP_MODE,
		BODY_PARAM_LINEAR_DAMP,
		BODY_PARAM_ANGULAR_DAMP,
		BODY_PARAM_MAX,
	};

	// Body state
	enum BodyState {
		BODY_STATE_TRANSFORM,
		BODY_STATE_LINEAR_VELOCITY,
		BODY_STATE_ANGULAR_VELOCITY,
		BODY_STATE_SLEEPING,
		BODY_STATE_CAN_SLEEP,
	};

	enum AreaParam {
		AREA_PARAM_GRAVITY_OVERRIDE_MODE,
		AREA_PARAM_GRAVITY,
		AREA_PARAM_GRAVITY_VECTOR,
		AREA_PARAM_GRAVITY_IS_POINT,
		AREA_PARAM_GRAVITY_POINT_UNIT_DISTANCE,
		AREA_PARAM_LINEAR_DAMP_OVERRIDE_MODE,
		AREA_PARAM_LINEAR_DAMP,
		AREA_PARAM_ANGULAR_DAMP_OVERRIDE_MODE,
		AREA_PARAM_ANGULAR_DAMP,
		AREA_PARAM_PRIORITY,
	};

	enum SpaceParam {
		SPACE_PARAM_CONTACT_RECYCLE_RADIUS,
		SPACE_PARAM_CONTACT_MAX_SEPARATION,
		SPACE_PARAM_CONTACT_MAX_ALLOWED_PENETRATION,
		SPACE_PARAM_CONTACT_DEFAULT_BIAS,
		SPACE_PARAM_BODY_LINEAR_VELOCITY_SLEEP_THRESHOLD,
		SPACE_PARAM_BODY_ANGULAR_VELOCITY_SLEEP_THRESHOLD,
		SPACE_PARAM_BODY_TIME_TO_SLEEP,
		SPACE_PARAM_SOLVER_ITERATIONS,
	};

protected:
	static void _bind_methods();

	// Internal body data
	struct BodyData {
		BodyMode mode = BODY_MODE_STATIC;
		PackedFloat32Array transform; // 20 floats: 4x4 basis + 4 origin
		Vector4 linear_velocity;
		PackedFloat32Array angular_velocity; // 6 floats bivector
		real_t mass = 1.0f;
		real_t friction = 0.6f;
		real_t bounce = 0.0f;
		real_t gravity_scale = 1.0f;
		real_t linear_damp = 0.0f;
		real_t angular_damp = 0.0f;
		bool sleeping = false;
		bool can_sleep = true;
		RID space;
		uint64_t object_instance_id;
		Callable state_sync_callback;
		Vector<RID> shapes;
	};

	struct AreaData {
		PackedFloat32Array transform;
		real_t gravity = 9.8f;
		Vector4 gravity_vector = Vector4(0, -1, 0, 0);
		bool gravity_is_point = false;
		int gravity_override_mode = 0;
		real_t linear_damp = 0.0f;
		real_t angular_damp = 0.0f;
		int priority = 0;
		bool monitoring = true;
		bool monitorable = true;
		RID space;
		Callable body_monitor_callback;
		Callable area_monitor_callback;
		Vector<RID> shapes;
	};

	struct SpaceData {
		Vector4 gravity_direction = Vector4(0, -1, 0, 0);
		real_t gravity_magnitude = 9.8f;
		Vector<RID> bodies;
		Vector<RID> areas;
		Ref<PhysicsDirectSpaceState4D> direct_state;
	};

	struct ShapeData {
		int type = 0; // 0=box,1=sphere,2=capsule,3=cylinder,4=convex,5=concave,6=plane,7=ray
		PackedFloat32Array data;
	};

	HashMap<RID, BodyData> _bodies;
	HashMap<RID, AreaData> _areas;
	HashMap<RID, SpaceData> _spaces;
	HashMap<RID, ShapeData> _shapes;
	Vector<RID> _active_spaces;

	uint64_t _rid_counter = 1;
	RID _make_rid() {
		uint64_t id = _rid_counter++;
		RID rid;
		// RID is 8 bytes of opaque data; write our counter into it directly.
		// get_id() will then return this value, making HashMap hashing work correctly.
		memcpy(rid._native_ptr(), &id, sizeof(uint64_t));
		return rid;
	}

	void _step_space(RID p_space, real_t p_delta);
	Vector4 _compute_gravity(RID p_space, RID p_body);

public:
	static PhysicsServer4D *get_singleton();

	PhysicsServer4D();
	~PhysicsServer4D();

	// ---- Space API ----
	RID space_create();
	void space_set_active(RID p_space, bool p_active);
	bool space_is_active(RID p_space) const;
	void space_set_param(RID p_space, SpaceParam p_param, real_t p_value);
	real_t space_get_param(RID p_space, SpaceParam p_param) const;
	Ref<PhysicsDirectSpaceState4D> space_get_direct_state(RID p_space);

	// ---- Body API ----
	RID body_create();
	void body_set_space(RID p_body, RID p_space);
	RID body_get_space(RID p_body) const;
	void body_set_mode(RID p_body, BodyMode p_mode);
	BodyMode body_get_mode(RID p_body) const;

	void body_add_shape(RID p_body, RID p_shape, const PackedFloat32Array &p_transform = PackedFloat32Array(), bool p_disabled = false);
	void body_remove_shape(RID p_body, int p_shape_idx);
	void body_clear_shapes(RID p_body);
	int body_get_shape_count(RID p_body) const;
	void body_set_shape_disabled(RID p_body, int p_shape_idx, bool p_disabled);

	void body_set_param(RID p_body, BodyParam p_param, const Variant &p_value);
	Variant body_get_param(RID p_body, BodyParam p_param) const;
	void body_reset_mass_properties(RID p_body);

	void body_set_state(RID p_body, BodyState p_state, const Variant &p_value);
	Variant body_get_state(RID p_body, BodyState p_state) const;

	void body_set_object_instance_id(RID p_body, uint64_t p_id);
	uint64_t body_get_object_instance_id(RID p_body) const;

	void body_set_state_sync_callback(RID p_body, const Callable &p_callable);

	void body_apply_central_force(RID p_body, const Vector4 &p_force);
	void body_apply_force(RID p_body, const Vector4 &p_force, const Vector4 &p_position);
	void body_apply_central_impulse(RID p_body, const Vector4 &p_impulse);
	void body_apply_impulse(RID p_body, const Vector4 &p_impulse, const Vector4 &p_position);
	void body_apply_torque(RID p_body, const PackedFloat32Array &p_torque);
	void body_apply_torque_impulse(RID p_body, const PackedFloat32Array &p_torque);

	void body_set_axis_lock(RID p_body, int p_axis, bool p_lock);
	bool body_is_axis_locked(RID p_body, int p_axis) const;

	// ---- Area API ----
	RID area_create();
	void area_set_space(RID p_area, RID p_space);
	RID area_get_space(RID p_area) const;
	void area_add_shape(RID p_area, RID p_shape, const PackedFloat32Array &p_transform = PackedFloat32Array(), bool p_disabled = false);
	void area_remove_shape(RID p_area, int p_shape_idx);
	void area_clear_shapes(RID p_area);
	int area_get_shape_count(RID p_area) const;
	void area_set_param(RID p_area, AreaParam p_param, const Variant &p_value);
	Variant area_get_param(RID p_area, AreaParam p_param) const;
	void area_set_transform(RID p_area, const PackedFloat32Array &p_transform);
	PackedFloat32Array area_get_transform(RID p_area) const;
	void area_set_monitor_callback(RID p_area, const Callable &p_callback);
	void area_set_area_monitor_callback(RID p_area, const Callable &p_callback);
	void area_set_monitorable(RID p_area, bool p_monitorable);
	void area_set_object_instance_id(RID p_area, uint64_t p_id);

	// ---- Shape API ----
	RID shape_create(int p_type);
	void shape_set_data(RID p_shape, const PackedFloat32Array &p_data);
	PackedFloat32Array shape_get_data(RID p_shape) const;

	// ---- Step ----
	void step(real_t p_delta);

	// ---- Free ----
	void free_rid(RID p_rid);
};

VARIANT_ENUM_CAST(PhysicsServer4D::BodyMode);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyParam);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyState);
VARIANT_ENUM_CAST(PhysicsServer4D::AreaParam);
VARIANT_ENUM_CAST(PhysicsServer4D::SpaceParam);
