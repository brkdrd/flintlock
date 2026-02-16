#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "math/transform4d.h"
#include <map>
#include <vector>

using namespace godot;

// Forward declarations
class Space4D;
class Body4D;
class Shape4DResource;

/// Simple variant-like wrapper for body state values
struct BodyStateValue {
	enum Type {
		TYPE_VECTOR4,
		TYPE_TRANSFORM4D
	};

	Type type;
	Vector4 vector_value;
	Transform4D transform_value;

	BodyStateValue() : type(TYPE_VECTOR4) {}
	BodyStateValue(const Vector4 &v) : type(TYPE_VECTOR4), vector_value(v) {}
	BodyStateValue(const Transform4D &t) : type(TYPE_TRANSFORM4D), transform_value(t) {}

	operator Vector4() const { return vector_value; }
	operator Transform4D() const { return transform_value; }
};

/// PhysicsServer4D — Singleton server for 4D physics simulation.
///
/// Manages spaces, bodies, and shapes. Provides RID-based API
/// matching Godot's PhysicsServer3D pattern.
/// Inherits from Object to be a proper Godot singleton.
class PhysicsServer4D : public Object {
	GDCLASS(PhysicsServer4D, Object)

public:
	// Enums matching Godot conventions
	enum BodyMode {
		BODY_MODE_STATIC,
		BODY_MODE_KINEMATIC,
		BODY_MODE_RIGID
	};

	enum BodyState {
		BODY_STATE_TRANSFORM,
		BODY_STATE_LINEAR_VELOCITY,
		BODY_STATE_ANGULAR_VELOCITY,
		BODY_STATE_SLEEPING,
		BODY_STATE_CAN_SLEEP
	};

	enum BodyParam {
		BODY_PARAM_MASS,
		BODY_PARAM_GRAVITY_SCALE,
		BODY_PARAM_LINEAR_DAMP,
		BODY_PARAM_ANGULAR_DAMP
	};

	enum ShapeType {
		SHAPE_HYPER_SPHERE,
		SHAPE_HYPER_BOX,
		SHAPE_HYPER_CAPSULE,
		SHAPE_HYPER_ELLIPSOID,
		SHAPE_CONVEX_HULL
	};

	PhysicsServer4D();
	~PhysicsServer4D();

	/// Get the singleton instance
	static PhysicsServer4D *get_singleton();

	// Space management
	RID space_create();
	void space_set_active(const RID &p_space, bool p_active);

	// Body management
	RID body_create();
	void body_set_space(const RID &p_body, const RID &p_space);
	void body_set_mode(const RID &p_body, BodyMode p_mode);

	// Body state - Vector4 variant
	void body_set_state_vector(const RID &p_body, BodyState p_state, const Vector4 &p_value);
	Vector4 body_get_state_vector(const RID &p_body, BodyState p_state);

	// Body state - Transform4D (internal use, not bound to GDScript)
	void body_set_state(const RID &p_body, BodyState p_state, const Transform4D &p_value);
	BodyStateValue body_get_state(const RID &p_body, BodyState p_state);

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
	static PhysicsServer4D *singleton;

	// RID allocation
	uint64_t next_rid_id = 1;
	RID allocate_rid();

	// RID tracking
	std::map<uint64_t, RID> rid_storage; // Store RID objects by their ID

	// Resource storage (using RID's ID as key)
	std::map<uint64_t, Space4D*> spaces;
	std::map<uint64_t, Body4D*> bodies;
	std::map<uint64_t, Shape4DResource*> shapes;

	// Lookup helpers
	Space4D *get_space(const RID &p_rid);
	Body4D *get_body(const RID &p_rid);
	Shape4DResource *get_shape(const RID &p_rid);
};

// Enum binding for GDScript
VARIANT_ENUM_CAST(PhysicsServer4D::BodyMode);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyState);
VARIANT_ENUM_CAST(PhysicsServer4D::BodyParam);
VARIANT_ENUM_CAST(PhysicsServer4D::ShapeType);
