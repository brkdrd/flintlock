#include "constraint_solver_4d.h"
#include <cmath>
#include <algorithm>

// Compute tangent vectors orthogonal to the contact normal in 4D.
// In 4D, there are 3 tangent directions perpendicular to a normal.
static void compute_tangent_basis(const Vector4 &normal, Vector4 tangents[3]) {
	// Find an axis not parallel to normal
	Vector4 candidates[4] = {
		Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1)
	};

	// Pick the axis least parallel to the normal
	int best = 0;
	float min_dot = fabsf(normal.dot(candidates[0]));
	for (int i = 1; i < 4; i++) {
		float d = fabsf(normal.dot(candidates[i]));
		if (d < min_dot) {
			min_dot = d;
			best = i;
		}
	}

	// First tangent: Gram-Schmidt
	tangents[0] = candidates[best] - normal * normal.dot(candidates[best]);
	float len = tangents[0].length();
	if (len > PHYSICS_EPSILON) tangents[0] /= len;

	// Second tangent: find another non-parallel axis
	int second_best = (best + 1) % 4;
	tangents[1] = candidates[second_best]
				  - normal * normal.dot(candidates[second_best])
				  - tangents[0] * tangents[0].dot(candidates[second_best]);
	len = tangents[1].length();
	if (len > PHYSICS_EPSILON) tangents[1] /= len;

	// Third tangent: perpendicular to normal, tangent[0], tangent[1]
	// Use the triple cross product in 4D
	tangents[2] = triple_cross_4d(normal, tangents[0], tangents[1]);
	len = tangents[2].length();
	if (len > PHYSICS_EPSILON) tangents[2] /= len;
}

void prepare_contacts_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies,
	float dt
) {
	float inv_dt = dt > 0.0f ? 1.0f / dt : 0.0f;

	for (auto &manifold : manifolds) {
		RigidBody4DInternal &body_a = bodies[manifold.body_a_index];
		RigidBody4DInternal &body_b = bodies[manifold.body_b_index];

		float inv_mass_a = body_a.get_inv_mass();
		float inv_mass_b = body_b.get_inv_mass();
		const Inertia4 &inv_I_a = body_a.get_world_inv_inertia();
		const Inertia4 &inv_I_b = body_b.get_world_inv_inertia();

		// Compute mixed restitution and friction
		float restitution = fmaxf(body_a.bounce, body_b.bounce);
		float friction = sqrtf(body_a.friction * body_b.friction);

		Vector4 tangents[3];
		if (manifold.point_count > 0) {
			compute_tangent_basis(manifold.points[0].normal, tangents);
		}

		for (int i = 0; i < manifold.point_count; i++) {
			ContactPoint4D &cp = manifold.points[i];

			Vector4 r_a = cp.position_on_a - body_a.transform.origin;
			Vector4 r_b = cp.position_on_b - body_b.transform.origin;

			// Normal effective mass
			// K_n = 1/m_a + 1/m_b + (I_a^-1 * (r_a ^ n)) . (r_a ^ n) + (I_b^-1 * (r_b ^ n)) . (r_b ^ n)
			Bivec4 rn_a = Bivec4::wedge(r_a, cp.normal);
			Bivec4 rn_b = Bivec4::wedge(r_b, cp.normal);
			Bivec4 inv_I_rn_a = inv_I_a.xform(rn_a);
			Bivec4 inv_I_rn_b = inv_I_b.xform(rn_b);

			float k_normal = inv_mass_a + inv_mass_b + inv_I_rn_a.dot(rn_a) + inv_I_rn_b.dot(rn_b);
			cp.normal_mass = k_normal > PHYSICS_EPSILON ? 1.0f / k_normal : 0.0f;

			// Tangent effective masses
			for (int t = 0; t < 3; t++) {
				Bivec4 rt_a = Bivec4::wedge(r_a, tangents[t]);
				Bivec4 rt_b = Bivec4::wedge(r_b, tangents[t]);
				Bivec4 inv_I_rt_a = inv_I_a.xform(rt_a);
				Bivec4 inv_I_rt_b = inv_I_b.xform(rt_b);

				float k_tangent = inv_mass_a + inv_mass_b + inv_I_rt_a.dot(rt_a) + inv_I_rt_b.dot(rt_b);
				cp.tangent_mass[t] = k_tangent > PHYSICS_EPSILON ? 1.0f / k_tangent : 0.0f;
			}

			// Baumgarte stabilization bias
			cp.bias = 0.0f;
			if (cp.depth > SLOP) {
				cp.bias = BAUMGARTE_FACTOR * inv_dt * (cp.depth - SLOP);
			}

			// Restitution bias
			Vector4 vel_a = body_a.linear_velocity + body_a.angular_velocity.cross(r_a);
			Vector4 vel_b = body_b.linear_velocity + body_b.angular_velocity.cross(r_b);
			Vector4 rel_vel = vel_a - vel_b;
			float rel_vel_normal = rel_vel.dot(cp.normal);

			if (rel_vel_normal < -RESTITUTION_THRESHOLD) {
				cp.bias += restitution * rel_vel_normal;
			}

			// Warm start - apply accumulated impulses from last frame
			Vector4 P = cp.normal * cp.normal_impulse;
			for (int t = 0; t < 3; t++) {
				P += tangents[t] * cp.tangent_impulse[t];
			}

			body_a.linear_velocity -= P * inv_mass_a;
			body_a.angular_velocity -= inv_I_a.xform(Bivec4::wedge(r_a, P));
			body_b.linear_velocity += P * inv_mass_b;
			body_b.angular_velocity += inv_I_b.xform(Bivec4::wedge(r_b, P));
		}
	}
}

void solve_iteration_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies
) {
	for (auto &manifold : manifolds) {
		RigidBody4DInternal &body_a = bodies[manifold.body_a_index];
		RigidBody4DInternal &body_b = bodies[manifold.body_b_index];

		float inv_mass_a = body_a.get_inv_mass();
		float inv_mass_b = body_b.get_inv_mass();
		const Inertia4 &inv_I_a = body_a.get_world_inv_inertia();
		const Inertia4 &inv_I_b = body_b.get_world_inv_inertia();

		float friction = sqrtf(body_a.friction * body_b.friction);

		Vector4 tangents[3];
		if (manifold.point_count > 0) {
			compute_tangent_basis(manifold.points[0].normal, tangents);
		}

		for (int i = 0; i < manifold.point_count; i++) {
			ContactPoint4D &cp = manifold.points[i];

			Vector4 r_a = cp.position_on_a - body_a.transform.origin;
			Vector4 r_b = cp.position_on_b - body_b.transform.origin;

			// Compute relative velocity at contact point
			Vector4 vel_a = body_a.linear_velocity + body_a.angular_velocity.cross(r_a);
			Vector4 vel_b = body_b.linear_velocity + body_b.angular_velocity.cross(r_b);
			Vector4 rel_vel = vel_a - vel_b;

			// Normal impulse
			float vn = rel_vel.dot(cp.normal);
			float jn = cp.normal_mass * (-vn + cp.bias);

			// Clamp accumulated impulse (must be non-negative)
			float old_impulse = cp.normal_impulse;
			cp.normal_impulse = fmaxf(old_impulse + jn, 0.0f);
			jn = cp.normal_impulse - old_impulse;

			// Apply normal impulse
			Vector4 Pn = cp.normal * jn;
			body_a.linear_velocity -= Pn * inv_mass_a;
			body_a.angular_velocity -= inv_I_a.xform(Bivec4::wedge(r_a, Pn));
			body_b.linear_velocity += Pn * inv_mass_b;
			body_b.angular_velocity += inv_I_b.xform(Bivec4::wedge(r_b, Pn));

			// Friction impulses (3 tangent directions)
			// Recompute relative velocity after normal impulse
			vel_a = body_a.linear_velocity + body_a.angular_velocity.cross(r_a);
			vel_b = body_b.linear_velocity + body_b.angular_velocity.cross(r_b);
			rel_vel = vel_a - vel_b;

			float friction_limit = friction * cp.normal_impulse;

			for (int t = 0; t < 3; t++) {
				float vt = rel_vel.dot(tangents[t]);
				float jt = cp.tangent_mass[t] * (-vt);

				// Clamp to friction cone
				float old_t_impulse = cp.tangent_impulse[t];
				cp.tangent_impulse[t] = fmaxf(-friction_limit,
					fminf(old_t_impulse + jt, friction_limit));
				jt = cp.tangent_impulse[t] - old_t_impulse;

				Vector4 Pt = tangents[t] * jt;
				body_a.linear_velocity -= Pt * inv_mass_a;
				body_a.angular_velocity -= inv_I_a.xform(Bivec4::wedge(r_a, Pt));
				body_b.linear_velocity += Pt * inv_mass_b;
				body_b.angular_velocity += inv_I_b.xform(Bivec4::wedge(r_b, Pt));
			}
		}
	}
}

void solve_contacts_4d(
	std::vector<ContactManifold4D> &manifolds,
	std::vector<RigidBody4DInternal> &bodies,
	float dt
) {
	prepare_contacts_4d(manifolds, bodies, dt);
	for (int iter = 0; iter < SOLVER_ITERATIONS; iter++) {
		solve_iteration_4d(manifolds, bodies);
	}
}
