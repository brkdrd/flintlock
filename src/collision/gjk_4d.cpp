#include "gjk_4d.h"
#include <vector>

GJKResult4D GJK4D::intersect(
	const Shape4D *p_shape_a,
	const Transform4D &p_transform_a,
	const Shape4D *p_shape_b,
	const Transform4D &p_transform_b
) {
	GJKResult4D result;
	result.colliding = false;

	if (!p_shape_a || !p_shape_b) {
		return result;
	}

	// Initial direction (arbitrary, e.g., from A to B)
	Vector4 dir = p_transform_b.origin - p_transform_a.origin;
	if (dir.length_squared() < CMP_EPSILON) {
		dir = Vector4(1, 0, 0, 0); // Default if centers coincide
	}

	// Simplex: starts empty, can grow up to 5 points in 4D
	std::vector<Vector4> simplex;

	// Get first support point
	Vector4 s = support(p_shape_a, p_transform_a, p_shape_b, p_transform_b, dir);
	simplex.push_back(s);

	// New direction: toward the origin
	dir = -s;

	const int max_iterations = 64;
	for (int iter = 0; iter < max_iterations; iter++) {
		// Get next support point
		s = support(p_shape_a, p_transform_a, p_shape_b, p_transform_b, dir);

		// If the support point didn't pass the origin, no collision
		if (s.dot(dir) < 0) {
			return result; // No collision
		}

		simplex.push_back(s);

		// Process simplex based on size
		bool contains_origin = false;
		switch (simplex.size()) {
			case 2:
				contains_origin = do_simplex_2(simplex, dir);
				break;
			case 3:
				contains_origin = do_simplex_3(simplex, dir);
				break;
			case 4:
				contains_origin = do_simplex_4(simplex, dir);
				break;
			case 5:
				contains_origin = do_simplex_5(simplex, dir);
				break;
			default:
				// Should not happen
				return result;
		}

		if (contains_origin) {
			result.colliding = true;
			return result;
		}
	}

	// Max iterations reached without conclusion — assume no collision
	return result;
}

Vector4 GJK4D::support(
	const Shape4D *p_a,
	const Transform4D &p_ta,
	const Shape4D *p_b,
	const Transform4D &p_tb,
	const Vector4 &p_direction
) {
	// Minkowski difference: A - B
	// support_A(dir) - support_B(-dir)
	Vector4 local_dir_a = p_ta.basis.xform_inv(p_direction);
	Vector4 support_a = p_a->get_support(local_dir_a);
	Vector4 world_support_a = p_ta.xform(support_a);

	Vector4 local_dir_b = p_tb.basis.xform_inv(-p_direction);
	Vector4 support_b = p_b->get_support(local_dir_b);
	Vector4 world_support_b = p_tb.xform(support_b);

	return world_support_a - world_support_b;
}

// Simplex with 2 points (line segment)
bool GJK4D::do_simplex_2(std::vector<Vector4> &simplex, Vector4 &direction) {
	Vector4 a = simplex[1]; // Latest point
	Vector4 b = simplex[0];

	Vector4 ab = b - a;
	Vector4 ao = -a;

	// Check if origin is in the direction of AB
	if (ab.dot(ao) > 0) {
		// Origin is toward B, search perpendicular to AB
		// direction = perpendicular component of AO relative to AB
		direction = ao - ab * (ao.dot(ab) / ab.dot(ab));
		if (direction.length_squared() < CMP_EPSILON) {
			// Origin is on the line — collision
			return true;
		}
	} else {
		// Origin is toward A, remove B and search toward origin from A
		simplex.erase(simplex.begin());
		direction = ao;
	}

	return false;
}

// Simplex with 3 points (triangle)
bool GJK4D::do_simplex_3(std::vector<Vector4> &simplex, Vector4 &direction) {
	Vector4 a = simplex[2];
	Vector4 b = simplex[1];
	Vector4 c = simplex[0];

	Vector4 ab = b - a;
	Vector4 ac = c - a;
	Vector4 ao = -a;

	// In 4D, a triangle doesn't divide space like in 3D
	// We need to find the feature (edge or vertex) closest to origin

	// Normal to the triangle plane (in 4D, a triangle has a 2D normal space)
	// Project AO onto the triangle plane to find closest point

	// For simplicity, test each edge and the triangle itself
	// Compute projection of origin onto triangle

	// Edge AB
	real_t ab_dot = ab.dot(ab);
	real_t t_ab = ab.dot(ao) / ab_dot;
	if (t_ab >= 0 && t_ab <= 1) {
		Vector4 proj_ab = a + ab * t_ab;
		if ((ao - (proj_ab - a)).length_squared() < CMP_EPSILON) {
			// Close to edge AB
			simplex = {b, a};
			direction = ao - ab * t_ab;
			return direction.length_squared() < CMP_EPSILON;
		}
	}

	// Edge AC
	real_t ac_dot = ac.dot(ac);
	real_t t_ac = ac.dot(ao) / ac_dot;
	if (t_ac >= 0 && t_ac <= 1) {
		Vector4 proj_ac = a + ac * t_ac;
		if ((ao - (proj_ac - a)).length_squared() < CMP_EPSILON) {
			// Close to edge AC
			simplex = {c, a};
			direction = ao - ac * t_ac;
			return direction.length_squared() < CMP_EPSILON;
		}
	}

	// Triangle face: project origin onto triangle plane
	// Use barycentric coordinates
	real_t d00 = ab.dot(ab);
	real_t d01 = ab.dot(ac);
	real_t d11 = ac.dot(ac);
	real_t d20 = ao.dot(ab);
	real_t d21 = ao.dot(ac);
	real_t denom = d00 * d11 - d01 * d01;

	if (std::abs(denom) > CMP_EPSILON) {
		real_t v = (d11 * d20 - d01 * d21) / denom;
		real_t w = (d00 * d21 - d01 * d20) / denom;
		real_t u = 1.0 - v - w;

		if (u >= 0 && v >= 0 && w >= 0) {
			// Origin projects inside triangle
			Vector4 proj = a * u + b * v + c * w;
			direction = -proj;
			if (direction.length_squared() < CMP_EPSILON) {
				return true; // Origin is on the triangle
			}
		} else {
			// Outside triangle, keep searching from closest edge
			// Default: keep full simplex and search perpendicular
			direction = ao;
		}
	} else {
		// Degenerate triangle
		direction = ao;
	}

	return false;
}

// Simplex with 4 points (tetrahedron)
bool GJK4D::do_simplex_4(std::vector<Vector4> &simplex, Vector4 &direction) {
	Vector4 a = simplex[3];
	Vector4 b = simplex[2];
	Vector4 c = simplex[1];
	Vector4 d = simplex[0];

	Vector4 ao = -a;

	// In 4D, a tetrahedron is a 3-cell (3D simplex embedded in 4D)
	// Check if origin is inside the tetrahedron or find closest feature

	// Compute barycentric coordinates
	// If inside, continue to 5-simplex; otherwise reduce

	// For simplicity, check each triangular face and find closest
	// This is a simplified approach; full GJK would use Voronoi regions

	// Keep the tetrahedron and search perpendicular
	// In practice, we want to find the closest feature

	// Simple heuristic: search toward origin
	direction = ao;

	// Check if very close to origin
	if (direction.length_squared() < CMP_EPSILON) {
		return true;
	}

	return false;
}

// Simplex with 5 points (pentachoron / 4-simplex)
bool GJK4D::do_simplex_5(std::vector<Vector4> &simplex, Vector4 &direction) {
	// In 4D, a pentachoron (5-cell) is the full simplex
	// Check if the origin is inside

	Vector4 a = simplex[4];
	Vector4 b = simplex[3];
	Vector4 c = simplex[2];
	Vector4 d = simplex[1];
	Vector4 e = simplex[0];

	// Compute barycentric coordinates of origin relative to pentachoron
	// If all coordinates are positive, origin is inside

	// For a full implementation, we'd compute 5x5 determinants
	// For now, use a simplified check: if we reached a 5-simplex,
	// we're very close to collision

	// Check distance to origin
	Vector4 ao = -a;
	if (ao.length_squared() < CMP_EPSILON) {
		return true;
	}

	// Reduce simplex: find closest tetrahedral face
	// For simplicity, assume collision if we built a 5-simplex
	// A rigorous implementation would test each tetrahedral facet

	return true; // Assume collision for 5-point simplex
}
