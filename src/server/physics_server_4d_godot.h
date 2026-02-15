#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "physics_server_4d.h"
#include "math/transform4d.h"

using namespace godot;

/// PhysicsServer4DGodot — GDScript-facing wrapper for PhysicsServer4D
///
/// This class exposes the internal PhysicsServer4D to Godot's scripting system.
/// It converts between Godot RIDs and internal PhysicsRIDs, and registers as
/// an Engine singleton accessible from GDScript.
class PhysicsServer4DGodot : public Object {
	GDCLASS(PhysicsServer4DGodot, Object)

public:
	// Body mode enum (exposed to GDScript)
	enum BodyMode {
		BODY_MODE_STATIC = PhysicsServer4D::BODY_MODE_STATIC,
		BODY_MODE_KINEMATIC = PhysicsServer4D::BODY_MODE_KINEMATIC,
		BODY_MODE_RIGID = PhysicsServer4D::BODY_MODE_RIGID
	};

	// Body state enum (exposed to GDScript)
	enum BodyState {
		BODY_STATE_TRANSFORM = PhysicsServer4D::BODY_STATE_TRANSFORM,
		BODY_STATE_LINEAR_VELOCITY = PhysicsServer4D::BODY_STATE_LINEAR_VELOCITY,
		BODY_STATE_ANGULAR_VELOCITY = PhysicsServer4D::BODY_STATE_ANGULAR_VELOCITY,
		BODY_STATE_SLEEPING = PhysicsServer4D::BODY_STATE_SLEEPING,
		BODY_STATE_CAN_SLEEP = PhysicsServer4D::BODY_STATE_CAN_SLEEP
	};

	// Body parameter enum (exposed to GDScript)
	enum BodyParam {
		BODY_PARAM_MASS = PhysicsServer4D::BODY_PARAM_MASS,
		BODY_PARAM_GRAVITY_SCALE = PhysicsServer4D::BODY_PARAM_GRAVITY_SCALE,
		BODY_PARAM_LINEAR_DAMP = PhysicsServer4D::BODY_PARAM_LINEAR_DAMP,
		BODY_PARAM_ANGULAR_DAMP = PhysicsServer4D::BODY_PARAM_ANGULAR_DAMP
	};

	// Shape type enum (exposed to GDScript)
	enum ShapeType {
		SHAPE_HYPER_SPHERE = PhysicsServer4D::SHAPE_HYPER_SPHERE,
		SHAPE_HYPER_BOX = PhysicsServer4D::SHAPE_HYPER_BOX,
		SHAPE_HYPER_CAPSULE = PhysicsServer4D::SHAPE_HYPER_CAPSULE,
		SHAPE_HYPER_ELLIPSOID = PhysicsServer4D::SHAPE_HYPER_ELLIPSOID,
		SHAPE_CONVEX_HULL = PhysicsServer4D::SHAPE_CONVEX_HULL
	};

	PhysicsServer4DGodot();
	~PhysicsServer4DGodot();

	// Space management
	RID space_create();
	void space_set_active(const RID &p_space, bool p_active);

	// Body management
	RID body_create();
	void body_set_space(const RID &p_body, const RID &p_space);
	void body_set_mode(const RID &p_body, BodyMode p_mode);

	// Body state (Vector4 variant)
	void body_set_state_vector(const RID &p_body, BodyState p_state, const Vector4 &p_value);
	Vector4 body_get_state_vector(const RID &p_body, BodyState p_state);

	// Body parameters
	void body_set_param(const RID &p_body, BodyParam p_param, real_t p_value);
	real_t body_get_param(const RID &p_body, BodyParam p_param);

	// Body forces
	void body_apply_impulse(const RID &p_body, const Vector4 &p_impulse, const Vector4 &p_position = Vector4());
	void body_apply_force(const RID &p_body, const Vector4 &p_force, const Vector4 &p_position = Vector4());

	// Shape management
	RID shape_create(ShapeType p_type);
	void shape_set_data_real(const RID &p_shape, real_t p_data); // For sphere radius
	void shape_set_data_vector4(const RID &p_shape, const Vector4 &p_data); // For box half-extents

	// Body-shape attachment
	void body_add_shape(const RID &p_body, const RID &p_shape);
	void body_remove_shape(const RID &p_body, int p_index);

	// Simulation
	void step(real_t p_delta);

	// Cleanup
	void free_rid(const RID &p_rid);

protected:
	static void _bind_methods();

private:
	// RID conversion helpers
	flintlock::PhysicsRID to_internal_rid(const RID &p_godot_rid);
	RID to_godot_rid(const flintlock::PhysicsRID &p_internal_rid);

	// RID mapping (godot::RID <-> flintlock::PhysicsRID)
	std::map<uint64_t, flintlock::PhysicsRID> rid_map;
	std::map<uint64_t, uint64_t> reverse_rid_map;
	uint64_t next_godot_rid_id = 1;
};

VARIANT_ENUM_CAST(PhysicsServer4DGodot::BodyMode);
VARIANT_ENUM_CAST(PhysicsServer4DGodot::BodyState);
VARIANT_ENUM_CAST(PhysicsServer4DGodot::BodyParam);
VARIANT_ENUM_CAST(PhysicsServer4DGodot::ShapeType);
