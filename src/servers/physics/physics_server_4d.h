#pragma once
#include <cstring>
#include <memory>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#include "physics_direct_body_state_4d.h"
#include "physics_direct_space_state_4d.h"
#include "core/space_4d_internal.h"

using namespace godot;

// ============================================================================
// PhysicsServer4D
//
// Full 4D physics server with GJK/EPA collision detection, sequential impulse
// constraint solver, dynamic BVH broadphase, and joint constraints.
// All algorithms are native 4D generalizations of Jolt Physics patterns.
// ============================================================================
class PhysicsServer4D : public Object {
	GDCLASS(PhysicsServer4D, Object);

	static PhysicsServer4D *singleton;

public:
	enum BodyMode {
		BODY_MODE_STATIC,
		BODY_MODE_KINEMATIC,
		BODY_MODE_RIGID,
		BODY_MODE_RIGID_LINEAR,
	};

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

	// Joint types exposed to GDScript
	enum JointType {
		JOINT_TYPE_PIN,
		JOINT_TYPE_HINGE,
		JOINT_TYPE_SLIDER,
		JOINT_TYPE_CONE_TWIST,
		JOINT_TYPE_GENERIC_10DOF,
	};

protected:
	static void _bind_methods();

	// ---- Internal storage ----
	// Maps RID -> internal ID for each resource type
	struct RIDData {
		enum Type { BODY, AREA, SPACE, SHAPE, JOINT };
		Type type;
		int internal_id;     // Index into the internal arrays
		RID space_rid;       // Which space this belongs to (for bodies/areas)
	};

	HashMap<RID, RIDData> _rid_map;
	HashMap<RID, std::shared_ptr<Shape4DInternal>> _shapes;
	HashMap<RID, std::shared_ptr<Space4DInternal>> _spaces;
	Vector<RID> _active_spaces;

	// Direct state objects (one per space)
	HashMap<RID, Ref<PhysicsDirectSpaceState4D>> _space_direct_states;

	uint64_t _rid_counter = 1;
	RID _make_rid() {
		uint64_t id = _rid_counter++;
		RID rid;
		memcpy(rid._native_ptr(), &id, sizeof(uint64_t));
		return rid;
	}

	// Default space (lazily created when first body/area needs one)
	RID _default_space;

	// Helpers
	RigidBody4DInternal *_get_body(RID p_rid);
	const RigidBody4DInternal *_get_body(RID p_rid) const;
	Area4DInternal *_get_area(RID p_rid);
	Space4DInternal *_get_space(RID p_rid);
	const Space4DInternal *_get_space(RID p_rid) const;

	void _step_space(RID p_space_rid, real_t p_delta);
	void _fire_body_callbacks(Space4DInternal &space);

	// Create internal shape from server shape data
	std::shared_ptr<Shape4DInternal> _create_internal_shape(int p_type, const PackedFloat32Array &p_data);

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

	void body_set_collision_layer(RID p_body, uint32_t p_layer);
	void body_set_collision_mask(RID p_body, uint32_t p_mask);

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

	// ---- Joint API ----
	RID joint_create(JointType p_type, RID p_body_a, RID p_body_b);
	void joint_set_param(RID p_joint, const String &p_param, const Variant &p_value);
	void joint_set_anchor_a(RID p_joint, const Vector4 &p_anchor);
	void joint_set_anchor_b(RID p_joint, const Vector4 &p_anchor);

	// ---- Step ----
	void step(real_t p_delta);

	// ---- Free ----
	void free_rid(RID p_rid);

	// ---- Default space ----
	RID get_default_space();

	// ---- Internal access (for direct state objects) ----
	Space4DInternal *get_space_internal(RID p_space);
	RigidBody4DInternal *get_body_internal(RID p_body);
};

VARIANT_ENUM_CAST(PhysicsServer4D::BodyMode);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyParam);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyState);
VARIANT_ENUM_CAST(PhysicsServer4D::AreaParam);
VARIANT_ENUM_CAST(PhysicsServer4D::SpaceParam);
VARIANT_ENUM_CAST(PhysicsServer4D::JointType);
