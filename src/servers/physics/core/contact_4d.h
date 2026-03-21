#pragma once

#include "physics_types_4d.h"

// ============================================================================
// Contact manifold structures for the 4D physics engine
// ============================================================================

static constexpr int MAX_CONTACT_POINTS = 4; // Max contact points per manifold

struct ContactPoint4D {
	Vector4 position_on_a; // World-space contact point on body A
	Vector4 position_on_b; // World-space contact point on body B
	Vector4 local_a;       // Local-space contact point on body A
	Vector4 local_b;       // Local-space contact point on body B
	Vector4 normal;        // Contact normal (from B to A)
	float depth = 0.0f;   // Penetration depth (positive when penetrating)

	// Solver data (accumulated impulses for warm starting)
	float normal_impulse = 0.0f;
	float tangent_impulse[3] = {0, 0, 0}; // Up to 3 tangent directions in 4D
	float bias = 0.0f;
	float normal_mass = 0.0f;
	float tangent_mass[3] = {0, 0, 0};
};

struct ContactManifold4D {
	int body_a_index = -1;   // Index into the space's body array
	int body_b_index = -1;
	int shape_a_index = -1;  // Shape index on body A
	int shape_b_index = -1;  // Shape index on body B

	ContactPoint4D points[MAX_CONTACT_POINTS];
	int point_count = 0;

	// Add a contact point, potentially replacing the worst one if full
	void add_point(const ContactPoint4D &point) {
		if (point_count < MAX_CONTACT_POINTS) {
			points[point_count++] = point;
			return;
		}

		// Replace the point with the smallest penetration depth
		int worst = 0;
		float worst_depth = points[0].depth;
		for (int i = 1; i < point_count; i++) {
			if (points[i].depth < worst_depth) {
				worst_depth = points[i].depth;
				worst = i;
			}
		}
		if (point.depth > worst_depth) {
			points[worst] = point;
		}
	}

	// Update contact points for a new frame (keep warm-started impulses)
	void update(const Xform4 &xform_a, const Xform4 &xform_b) {
		for (int i = 0; i < point_count; ) {
			// Recompute world positions from local positions
			points[i].position_on_a = xform_a.xform(points[i].local_a);
			points[i].position_on_b = xform_b.xform(points[i].local_b);

			// Recompute penetration depth
			Vector4 diff = points[i].position_on_a - points[i].position_on_b;
			points[i].depth = diff.dot(points[i].normal);

			// Remove contacts that have separated too much
			Vector4 tangent_diff = diff - points[i].normal * points[i].depth;
			float tangent_dist_sq = tangent_diff.length_squared();

			if (points[i].depth < -0.1f || tangent_dist_sq > 0.04f) {
				// Remove this contact
				points[i] = points[point_count - 1];
				point_count--;
			} else {
				i++;
			}
		}
	}
};
