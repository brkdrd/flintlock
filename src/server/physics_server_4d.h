#pragma once

#include "math/transform4d.h"
#include "math/vector4d.h"
#include <map>
#include <vector>

// Internal RID type for physics resources
// Note: This is separate from godot::RID which is Godot's resource system
// Our RID is just a simple uint64 handle for internal physics objects
namespace flintlock {
	class PhysicsRID {
	public:
		PhysicsRID() : id(0) {}
		explicit PhysicsRID(uint64_t p_id) : id(p_id) {}

		bool is_valid() const { return id != 0; }
		bool operator==(const PhysicsRID &p_other) const { return id == p_other.id; }
		bool operator!=(const PhysicsRID &p_other) const { return id != p_other.id; }
		bool operator<(const PhysicsRID &p_other) const { return id < p_other.id; }

		uint64_t get_id() const { return id; }

	private:
		uint64_t id;
	};
}

// Note: We use flintlock::PhysicsRID explicitly to avoid conflicts with godot::RID

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
class PhysicsServer4D {
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

	/// Get the singleton instance
	static PhysicsServer4D *get_singleton();

	/// Initialize the singleton (for testing)
	static void initialize();

	/// Cleanup the singleton (for testing)
	static void finalize();

	// Space management
	flintlock::PhysicsRID space_create();
	void space_set_active(flintlock::PhysicsRID p_space, bool p_active);

	// Body management
	flintlock::PhysicsRID body_create();
	void body_set_space(flintlock::PhysicsRID p_body, flintlock::PhysicsRID p_space);
	void body_set_mode(flintlock::PhysicsRID p_body, BodyMode p_mode);

	// Body state
	void body_set_state(flintlock::PhysicsRID p_body, BodyState p_state, const Vector4 &p_value);
	void body_set_state(flintlock::PhysicsRID p_body, BodyState p_state, const Transform4D &p_value);
	BodyStateValue body_get_state(flintlock::PhysicsRID p_body, BodyState p_state);

	// Body parameters
	void body_set_param(flintlock::PhysicsRID p_body, BodyParam p_param, real_t p_value);
	real_t body_get_param(flintlock::PhysicsRID p_body, BodyParam p_param);

	// Body forces
	void body_apply_impulse(flintlock::PhysicsRID p_body, const Vector4 &p_impulse, const Vector4 &p_position = Vector4());
	void body_apply_force(flintlock::PhysicsRID p_body, const Vector4 &p_force, const Vector4 &p_position = Vector4());

	// Shape management
	flintlock::PhysicsRID shape_create(ShapeType p_type);
	void shape_set_data(flintlock::PhysicsRID p_shape, real_t p_data); // For sphere radius
	void shape_set_data(flintlock::PhysicsRID p_shape, const Vector4 &p_data); // For box half-extents

	// Body-shape attachment
	void body_add_shape(flintlock::PhysicsRID p_body, flintlock::PhysicsRID p_shape, const Transform4D &p_transform);
	void body_remove_shape(flintlock::PhysicsRID p_body, int p_index);

	// Simulation
	void step(real_t p_delta);

	// Cleanup
	void free_rid(flintlock::PhysicsRID p_rid);

private:
	PhysicsServer4D();
	~PhysicsServer4D();

	static PhysicsServer4D *singleton;

	// RID allocation
	uint64_t next_rid_id = 1;
	flintlock::PhysicsRID allocate_rid() { return flintlock::PhysicsRID(next_rid_id++); }

	// Resource storage
	std::map<flintlock::PhysicsRID, Space4D*> spaces;
	std::map<flintlock::PhysicsRID, Body4D*> bodies;
	std::map<flintlock::PhysicsRID, Shape4DResource*> shapes;

	// Lookup helpers
	Space4D *get_space(flintlock::PhysicsRID p_rid);
	Body4D *get_body(flintlock::PhysicsRID p_rid);
	Shape4DResource *get_shape(flintlock::PhysicsRID p_rid);
};
