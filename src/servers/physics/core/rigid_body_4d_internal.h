#pragma once

#include "physics_types_4d.h"
#include "shape_4d_internal.h"
#include <vector>
#include <memory>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

// ============================================================================
// Internal rigid body representation for the 4D physics engine.
// This is the physics server's internal storage, not exposed to GDScript.
// ============================================================================

enum BodyMode4D {
	BODY_MODE_4D_STATIC = 0,
	BODY_MODE_4D_KINEMATIC = 1,
	BODY_MODE_4D_RIGID = 2,
	BODY_MODE_4D_RIGID_LINEAR = 3, // No angular motion
};

struct ShapeInstance4D {
	std::shared_ptr<Shape4DInternal> shape;
	Xform4 local_transform;    // Shape-local transform relative to body
	bool disabled = false;
	AABB4 cached_aabb;         // World-space AABB (cached)
};

struct RigidBody4DInternal {
	// Identity
	int id = -1;                    // Index in the space's body array
	uint64_t object_instance_id = 0;
	int space_index = -1;           // Which space this body belongs to

	// State
	BodyMode4D mode = BODY_MODE_4D_STATIC;
	Xform4 transform;               // World-space transform
	Vector4 linear_velocity;
	Bivec4 angular_velocity;

	// Properties
	float mass = 1.0f;
	float inv_mass = 1.0f;
	Inertia4 local_inertia;          // Local-space inertia tensor
	Inertia4 inv_local_inertia;      // Inverse of local inertia
	Inertia4 world_inv_inertia;      // World-space inverse inertia (recomputed each step)

	float friction = 0.6f;
	float bounce = 0.0f;
	float gravity_scale = 1.0f;
	float linear_damp = 0.0f;
	float angular_damp = 0.0f;

	// Sleep
	bool sleeping = false;
	bool can_sleep = true;
	float sleep_timer = 0.0f;

	// Accumulation
	Vector4 applied_force;
	Bivec4 applied_torque;
	Vector4 constant_force;          // Applied every frame (not cleared)

	// Shapes
	std::vector<ShapeInstance4D> shapes;

	// Collision filtering
	uint32_t collision_layer = 1;
	uint32_t collision_mask = 1;
	float collision_priority = 1.0f;

	// Axis locking
	bool axis_lock[4] = {false, false, false, false}; // X, Y, Z, W linear
	bool plane_lock[6] = {false, false, false, false, false, false}; // Rotation planes

	// Callback
	Callable state_sync_callback;

	// Broadphase proxy ID
	int broadphase_proxy = -1;

	// Island
	int island_id = -1;

	// Contact count (for sleeping)
	int contact_count = 0;

	// Methods
	bool is_static() const { return mode == BODY_MODE_4D_STATIC; }
	bool is_kinematic() const { return mode == BODY_MODE_4D_KINEMATIC; }
	bool is_dynamic() const { return mode == BODY_MODE_4D_RIGID || mode == BODY_MODE_4D_RIGID_LINEAR; }

	float get_inv_mass() const {
		return is_dynamic() ? inv_mass : 0.0f;
	}

	const Inertia4 &get_world_inv_inertia() const {
		return world_inv_inertia;
	}

	void set_mass(float m) {
		mass = m;
		inv_mass = m > PHYSICS_EPSILON ? 1.0f / m : 0.0f;
		recompute_inertia();
	}

	void recompute_inertia() {
		if (!is_dynamic() || shapes.empty()) {
			local_inertia = Inertia4::uniform(mass);
			inv_local_inertia = Inertia4::uniform(inv_mass);
			update_world_inertia();
			return;
		}

		// Sum inertia from all shapes
		local_inertia = Inertia4();
		for (const auto &si : shapes) {
			if (si.disabled || !si.shape) continue;
			Inertia4 shape_inertia = si.shape->compute_inertia(mass);
			for (int i = 0; i < 6; i++)
				for (int j = 0; j < 6; j++)
					local_inertia.m[i][j] += shape_inertia.m[i][j];
		}

		if (shapes.size() > 1) {
			// Normalize by shape count (approximate for compound shapes)
			float inv_count = 1.0f / (float)shapes.size();
			for (int i = 0; i < 6; i++)
				for (int j = 0; j < 6; j++)
					local_inertia.m[i][j] *= inv_count;
		}

		inv_local_inertia = local_inertia.inverse();

		if (mode == BODY_MODE_4D_RIGID_LINEAR) {
			// Zero out angular inertia
			inv_local_inertia = Inertia4();
		}

		update_world_inertia();
	}

	void update_world_inertia() {
		if (!is_dynamic()) {
			world_inv_inertia = Inertia4();
			return;
		}
		world_inv_inertia = inv_local_inertia.rotated(transform.basis);

		// Apply plane locks
		for (int i = 0; i < 6; i++) {
			if (plane_lock[i]) {
				for (int j = 0; j < 6; j++) {
					world_inv_inertia.m[i][j] = 0;
					world_inv_inertia.m[j][i] = 0;
				}
			}
		}
	}

	AABB4 get_world_aabb() const {
		AABB4 result;
		bool first = true;
		for (const auto &si : shapes) {
			if (si.disabled || !si.shape) continue;
			Xform4 world_shape_xf = transform * si.local_transform;
			AABB4 shape_aabb = compute_world_aabb(si.shape.get(), world_shape_xf);
			if (first) {
				result = shape_aabb;
				first = false;
			} else {
				result = result.merged(shape_aabb);
			}
		}
		if (first) {
			// No shapes - use a point AABB at the origin
			result = AABB4(transform.origin, transform.origin);
		}
		return result;
	}

	void apply_central_force(const Vector4 &force) {
		if (!is_dynamic()) return;
		applied_force += force;
	}

	void apply_force(const Vector4 &force, const Vector4 &position) {
		if (!is_dynamic()) return;
		applied_force += force;
		// Torque = position ^ force (wedge product)
		Vector4 r = position - transform.origin;
		applied_torque += Bivec4::wedge(r, force);
	}

	void apply_central_impulse(const Vector4 &impulse) {
		if (!is_dynamic()) return;
		linear_velocity += impulse * inv_mass;
	}

	void apply_impulse(const Vector4 &impulse, const Vector4 &position) {
		if (!is_dynamic()) return;
		linear_velocity += impulse * inv_mass;
		Vector4 r = position - transform.origin;
		Bivec4 angular_impulse = Bivec4::wedge(r, impulse);
		angular_velocity += world_inv_inertia.xform(angular_impulse);
	}

	void apply_torque_impulse(const Bivec4 &torque) {
		if (!is_dynamic()) return;
		angular_velocity += world_inv_inertia.xform(torque);
	}

	void integrate_forces(float dt, const Vector4 &gravity) {
		if (!is_dynamic() || sleeping) return;

		// Apply gravity
		linear_velocity += gravity * gravity_scale * dt;

		// Apply accumulated forces
		linear_velocity += (applied_force + constant_force) * (inv_mass * dt);
		angular_velocity += world_inv_inertia.xform(applied_torque * dt);

		// Apply damping
		if (linear_damp > 0.0f) {
			float damp = fmaxf(0.0f, 1.0f - linear_damp * dt);
			linear_velocity *= damp;
		}
		if (angular_damp > 0.0f) {
			float damp = fmaxf(0.0f, 1.0f - angular_damp * dt);
			angular_velocity *= damp;
		}

		// Apply axis locks
		float *lv = &linear_velocity.x;
		for (int i = 0; i < 4; i++) {
			if (axis_lock[i]) lv[i] = 0;
		}

		// Clear applied forces (constant_force is not cleared)
		applied_force = Vector4(0, 0, 0, 0);
		applied_torque = Bivec4();
	}

	void integrate_velocity(float dt) {
		if (!is_dynamic() || sleeping) return;

		// Update position
		transform.origin += linear_velocity * dt;

		// Update orientation
		if (mode != BODY_MODE_4D_RIGID_LINEAR && !angular_velocity.is_zero()) {
			transform.basis = transform.basis.rotated_by_bivector(angular_velocity, dt);
		}

		// Update world inertia
		update_world_inertia();
	}

	// Check if body should go to sleep
	void update_sleep(float dt) {
		if (!can_sleep || !is_dynamic()) return;

		float lin_energy = linear_velocity.length_squared();
		float ang_energy = angular_velocity.length_squared();
		float threshold = 0.01f;

		if (lin_energy < threshold && ang_energy < threshold) {
			sleep_timer += dt;
			if (sleep_timer > 0.5f) {
				sleeping = true;
				linear_velocity = Vector4(0, 0, 0, 0);
				angular_velocity = Bivec4();
			}
		} else {
			sleep_timer = 0.0f;
			sleeping = false;
		}
	}

	void wake_up() {
		sleeping = false;
		sleep_timer = 0.0f;
	}
};
