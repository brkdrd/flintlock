#pragma once

#include "rigid_body_4d_internal.h"
#include "area_4d_internal.h"
#include "contact_4d.h"
#include "broadphase_4d.h"
#include "joint_4d_internal.h"
#include <vector>
#include <memory>
#include <unordered_map>

// ============================================================================
// Space4DInternal - The physics world/space.
// Contains all bodies, areas, and joints. Runs the full physics pipeline:
//   1. Integrate forces (gravity, applied forces)
//   2. Broadphase collision detection (BVH)
//   3. Narrowphase collision detection (GJK/EPA)
//   4. Contact constraint solving (sequential impulse)
//   5. Joint constraint solving
//   6. Integrate velocities (update positions/rotations)
//   7. Update sleep states
//   8. Fire area callbacks
//   9. Fire body state sync callbacks
// ============================================================================

struct Space4DInternal {
	int id = -1;
	bool active = false;

	// Default gravity
	Vector4 gravity_direction = Vector4(0, -1, 0, 0);
	float gravity_magnitude = 9.8f;

	// Space parameters
	float contact_recycle_radius = 0.01f;
	float contact_max_separation = 0.05f;
	float contact_max_penetration = 0.02f;
	float body_linear_velocity_sleep_threshold = 0.01f;
	float body_angular_velocity_sleep_threshold = 0.01f;
	float body_time_to_sleep = 0.5f;
	int solver_iterations = SOLVER_ITERATIONS;

	// Bodies and areas stored by their server-side IDs
	std::vector<RigidBody4DInternal> bodies;
	std::vector<Area4DInternal> areas;
	std::vector<std::shared_ptr<Joint4DInternal>> joints;

	// Contact manifolds (regenerated each step)
	std::vector<ContactManifold4D> contact_manifolds;

	// Broadphase
	BroadPhase4D broadphase;

	// Index maps: body/area server index -> internal array index
	std::unordered_map<int, int> body_index_map;  // server body ID -> bodies[] index
	std::unordered_map<int, int> area_index_map;   // server area ID -> areas[] index

	// Time step
	float last_dt = 1.0f / 60.0f;

	// Run one physics step
	void step(float dt);

	// Sub-steps of the pipeline
	void update_broadphase();
	void detect_collisions();
	void integrate_forces(float dt);
	void solve_constraints(float dt);
	void integrate_velocities(float dt);
	void update_sleep_states(float dt);
	void detect_area_overlaps();
	void fire_callbacks();

	// Compute gravity for a body (accounting for area overrides)
	Vector4 compute_gravity_for_body(const RigidBody4DInternal &body) const;

	// Ray cast through the space
	struct RayResult {
		bool hit = false;
		Vector4 point;
		Vector4 normal;
		int body_index = -1;
		int shape_index = -1;
		float t = 0.0f;
	};
	RayResult cast_ray(const Vector4 &from, const Vector4 &to,
					   uint32_t collision_mask = 0xFFFFFFFF,
					   bool collide_bodies = true, bool collide_areas = false,
					   const std::vector<int> &exclude_bodies = {}) const;

	// Point query
	struct PointResult {
		int body_index = -1;
		int shape_index = -1;
		uint64_t object_instance_id = 0;
	};
	std::vector<PointResult> query_point(const Vector4 &point, int max_results = 32,
										  uint32_t collision_mask = 0xFFFFFFFF,
										  bool collide_bodies = true, bool collide_areas = false) const;

	// Shape cast (motion query for character body)
	struct ShapeCastResult {
		bool colliding = false;
		float safe_fraction = 1.0f;
		float unsafe_fraction = 1.0f;
		Vector4 collision_point;
		Vector4 collision_normal;
		int collider_index = -1;
	};
	ShapeCastResult cast_shape(const Shape4DInternal *shape, const Xform4 &from,
							   const Vector4 &motion, float margin = 0.04f,
							   uint32_t collision_mask = 0xFFFFFFFF,
							   const std::vector<int> &exclude = {}) const;

	// Body motion test (for move_and_slide)
	struct MotionResult {
		bool colliding = false;
		Vector4 travel;
		Vector4 remainder;
		Vector4 collision_point;
		Vector4 collision_normal;
		float collision_depth = 0.0f;
		int collider_index = -1;
		float safe_fraction = 1.0f;
	};
	MotionResult test_body_motion(int body_index, const Vector4 &motion,
								  float margin = 0.08f) const;
};
