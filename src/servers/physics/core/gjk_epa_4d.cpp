#include "gjk_epa_4d.h"
#include <cmath>
#include <algorithm>
#include <vector>

// ============================================================================
// GJK Simplex operations in 4D
// The simplex progresses: point -> line -> triangle -> tetrahedron -> pentachoron
// At each step, we determine the closest feature to the origin and update the
// search direction accordingly.
// ============================================================================

namespace {

// Helper: compute barycentric coordinates for closest point on line segment
// Returns true if origin is in the Voronoi region of the edge AB.
bool closest_on_line(const Vector4 &A, const Vector4 &B, Vector4 &dir, int &count, Vector4 pts[5]) {
	Vector4 AB = B - A;
	Vector4 AO = -A;
	float t = AO.dot(AB) / AB.dot(AB);

	if (t <= 0.0f) {
		// Closest to A
		pts[0] = A;
		count = 1;
		dir = AO;
		return false;
	}
	if (t >= 1.0f) {
		// Closest to B
		pts[0] = B;
		count = 1;
		dir = -B;
		return false;
	}

	// Closest to edge interior
	Vector4 closest = A + AB * t;
	dir = -closest;
	return false; // Can't contain origin on a line
}

// Helper: closest point on triangle to origin
bool closest_on_triangle(Vector4 pts[5], int &count, Vector4 &dir) {
	Vector4 A = pts[0], B = pts[1], C = pts[2];
	Vector4 AB = B - A, AC = C - A;
	Vector4 AO = -A;

	float d_ab = AO.dot(AB);
	float d_ac = AO.dot(AC);
	float d_ab_ab = AB.dot(AB);
	float d_ac_ac = AC.dot(AC);
	float d_ab_ac = AB.dot(AC);

	// Solve for barycentric coords: AO = u*AB + v*AC
	float denom = d_ab_ab * d_ac_ac - d_ab_ac * d_ab_ac;
	if (fabsf(denom) < PHYSICS_EPSILON) {
		// Degenerate triangle - fall back to line
		count = 2;
		closest_on_line(A, B, dir, count, pts);
		return false;
	}

	float u = (d_ac_ac * d_ab - d_ab_ac * d_ac) / denom;
	float v = (d_ab_ab * d_ac - d_ab_ac * d_ab) / denom;

	if (u >= 0 && v >= 0 && u + v <= 1.0f) {
		// Origin projects inside triangle - compute perpendicular direction
		// Need a direction perpendicular to both AB and AC in 4D
		// In 4D, we need 2 more vectors to define a perpendicular
		// Use the rejection of AO from the triangle plane
		Vector4 proj = A + AB * u + AC * v;
		dir = -proj;
		if (dir.length_squared() < PHYSICS_EPSILON * PHYSICS_EPSILON) {
			// Origin is ON the triangle
			return true;
		}
		return false;
	}

	// Origin is outside triangle - find closest edge
	if (u < 0) {
		// Closest to edge AC
		pts[1] = C;
		count = 2;
		closest_on_line(A, C, dir, count, pts);
	} else if (v < 0) {
		// Closest to edge AB
		count = 2;
		closest_on_line(A, B, dir, count, pts);
	} else {
		// Closest to edge BC
		pts[0] = B;
		pts[1] = C;
		count = 2;
		closest_on_line(B, C, dir, count, pts);
	}
	return false;
}

// Helper: closest point on tetrahedron to origin
bool closest_on_tetrahedron(Vector4 pts[5], int &count, Vector4 &dir) {
	Vector4 A = pts[0], B = pts[1], C = pts[2], D = pts[3];
	Vector4 AB = B - A, AC = C - A, AD = D - A;
	Vector4 AO = -A;

	// Compute the normal to the tetrahedron's 3D subspace using triple cross
	Vector4 normal = triple_cross_4d(AB, AC, AD);
	float normal_len = normal.length();

	if (normal_len < PHYSICS_EPSILON) {
		// Degenerate tetrahedron - fall back to triangle
		count = 3;
		return closest_on_triangle(pts, count, dir);
	}

	// Check if origin is on the positive or negative side of each face
	// A tetrahedron has 4 triangular faces:
	// Face 0: BCD (opposite A) - normal points away from A
	// Face 1: ACD (opposite B) - normal points away from B
	// Face 2: ABD (opposite C) - normal points away from C
	// Face 3: ABC (opposite D) - normal points away from D

	// For each face, check if origin is outside
	// Face BCD: normal = triple_cross(CB, CD, ??) - but we're in 4D
	// Instead, use the approach: project AO onto the normal and the face normals

	// Solve for barycentric coords: AO = u*AB + v*AC + w*AD + t*N
	// First, check if origin is within the tetrahedron volume by checking
	// if it's on the inside of all 4 faces.

	// Compute the 4D "volume" orientation
	float vol = AO.dot(normal);
	float det_sign = vol;

	// Check each face
	// We use the approach of checking regions systematically
	// Solve: AO = u*AB + v*AC + w*AD
	// Using the gram matrix approach

	float g[3][3] = {
		{AB.dot(AB), AB.dot(AC), AB.dot(AD)},
		{AC.dot(AB), AC.dot(AC), AC.dot(AD)},
		{AD.dot(AB), AD.dot(AC), AD.dot(AD)}
	};
	float rhs[3] = {AO.dot(AB), AO.dot(AC), AO.dot(AD)};

	// Solve 3x3 system via Cramer's rule
	float det3 = g[0][0]*(g[1][1]*g[2][2] - g[1][2]*g[2][1])
				- g[0][1]*(g[1][0]*g[2][2] - g[1][2]*g[2][0])
				+ g[0][2]*(g[1][0]*g[2][1] - g[1][1]*g[2][0]);

	if (fabsf(det3) < PHYSICS_EPSILON) {
		count = 3;
		return closest_on_triangle(pts, count, dir);
	}

	float inv_det = 1.0f / det3;
	float u = inv_det * (rhs[0]*(g[1][1]*g[2][2] - g[1][2]*g[2][1])
						- g[0][1]*(rhs[1]*g[2][2] - g[1][2]*rhs[2])
						+ g[0][2]*(rhs[1]*g[2][1] - g[1][1]*rhs[2]));
	float v = inv_det * (g[0][0]*(rhs[1]*g[2][2] - g[1][2]*rhs[2])
						- rhs[0]*(g[1][0]*g[2][2] - g[1][2]*g[2][0])
						+ g[0][2]*(g[1][0]*rhs[2] - rhs[1]*g[2][0]));
	float w = inv_det * (g[0][0]*(g[1][1]*rhs[2] - rhs[1]*g[2][1])
						- g[0][1]*(g[1][0]*rhs[2] - rhs[1]*g[2][0])
						+ rhs[0]*(g[1][0]*g[2][1] - g[1][1]*g[2][0]));

	// Check if origin is inside the tetrahedron (in the 3D subspace)
	if (u >= 0 && v >= 0 && w >= 0 && u + v + w <= 1.0f) {
		// Origin projects inside the tetrahedron - direction is the component
		// of AO perpendicular to the tetrahedron's 3D subspace
		Vector4 proj = A + AB * u + AC * v + AD * w;
		dir = -proj;
		if (dir.length_squared() < PHYSICS_EPSILON * PHYSICS_EPSILON) {
			return true;
		}
		return false;
	}

	// Origin is outside the tetrahedron - find closest sub-feature
	// Check faces (triangles)
	struct FaceCheck {
		int idx[3]; // indices into pts array
		float bary[2]; // barycentric coords on the face
		bool outside;
	};

	// Face BCD (indices 1,2,3 - opposite vertex 0/A)
	// Face ACD (indices 0,2,3 - opposite vertex 1/B)
	// Face ABD (indices 0,1,3 - opposite vertex 2/C)
	// Face ABC (indices 0,1,2 - opposite vertex 3/D)

	float min_dist = 1e30f;
	int best_face = -1;

	int face_indices[4][3] = {{1,2,3}, {0,2,3}, {0,1,3}, {0,1,2}};

	for (int f = 0; f < 4; f++) {
		Vector4 face_pts[5];
		face_pts[0] = pts[face_indices[f][0]];
		face_pts[1] = pts[face_indices[f][1]];
		face_pts[2] = pts[face_indices[f][2]];
		int fc = 3;
		Vector4 fd;
		closest_on_triangle(face_pts, fc, fd);
		float dist = fd.length_squared();
		if (dist < min_dist) {
			min_dist = dist;
			best_face = f;
		}
	}

	if (best_face >= 0) {
		pts[0] = pts[face_indices[best_face][0]];
		pts[1] = pts[face_indices[best_face][1]];
		pts[2] = pts[face_indices[best_face][2]];
		count = 3;
		return closest_on_triangle(pts, count, dir);
	}

	// Fallback
	dir = -A;
	count = 1;
	return false;
}

// Helper: closest point on pentachoron (5-cell) to origin
bool closest_on_pentachoron(Vector4 pts[5], int &count, Vector4 &dir) {
	Vector4 A = pts[0], B = pts[1], C = pts[2], D = pts[3], E = pts[4];
	Vector4 AB = B - A, AC = C - A, AD = D - A, AE = E - A;
	Vector4 AO = -A;

	// Solve for barycentric coords: AO = u*AB + v*AC + w*AD + t*AE
	// Using the 4x4 gram matrix
	float g[4][4] = {
		{AB.dot(AB), AB.dot(AC), AB.dot(AD), AB.dot(AE)},
		{AC.dot(AB), AC.dot(AC), AC.dot(AD), AC.dot(AE)},
		{AD.dot(AB), AD.dot(AC), AD.dot(AD), AD.dot(AE)},
		{AE.dot(AB), AE.dot(AC), AE.dot(AD), AE.dot(AE)}
	};
	float rhs[4] = {AO.dot(AB), AO.dot(AC), AO.dot(AD), AO.dot(AE)};

	// Solve 4x4 system using Gaussian elimination
	float aug[4][5];
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) aug[i][j] = g[i][j];
		aug[i][4] = rhs[i];
	}

	for (int col = 0; col < 4; col++) {
		// Partial pivoting
		int max_row = col;
		float max_val = fabsf(aug[col][col]);
		for (int row = col + 1; row < 4; row++) {
			if (fabsf(aug[row][col]) > max_val) {
				max_val = fabsf(aug[row][col]);
				max_row = row;
			}
		}
		if (max_val < PHYSICS_EPSILON) {
			// Degenerate - fall back to tetrahedron
			count = 4;
			return closest_on_tetrahedron(pts, count, dir);
		}
		if (max_row != col) {
			for (int j = 0; j < 5; j++) std::swap(aug[col][j], aug[max_row][j]);
		}
		for (int row = col + 1; row < 4; row++) {
			float factor = aug[row][col] / aug[col][col];
			for (int j = col; j < 5; j++) aug[row][j] -= factor * aug[col][j];
		}
	}

	// Back substitution
	float bary[4];
	for (int i = 3; i >= 0; i--) {
		bary[i] = aug[i][4];
		for (int j = i + 1; j < 4; j++) bary[i] -= aug[i][j] * bary[j];
		bary[i] /= aug[i][i];
	}

	float u = bary[0], v = bary[1], w = bary[2], t = bary[3];

	// Check if origin is inside the pentachoron
	if (u >= 0 && v >= 0 && w >= 0 && t >= 0 && u + v + w + t <= 1.0f) {
		// Origin is inside - collision detected
		return true;
	}

	// Origin is outside - find closest tetrahedral face
	// A pentachoron has 5 tetrahedral cells:
	// Cell 0: BCDE (opposite A, indices 1,2,3,4)
	// Cell 1: ACDE (opposite B, indices 0,2,3,4)
	// Cell 2: ABDE (opposite C, indices 0,1,3,4)
	// Cell 3: ABCE (opposite D, indices 0,1,2,4)
	// Cell 4: ABCD (opposite E, indices 0,1,2,3)

	int cell_indices[5][4] = {
		{1,2,3,4}, {0,2,3,4}, {0,1,3,4}, {0,1,2,4}, {0,1,2,3}
	};

	float min_dist = 1e30f;
	int best_cell = -1;

	for (int c = 0; c < 5; c++) {
		Vector4 cell_pts[5];
		cell_pts[0] = pts[cell_indices[c][0]];
		cell_pts[1] = pts[cell_indices[c][1]];
		cell_pts[2] = pts[cell_indices[c][2]];
		cell_pts[3] = pts[cell_indices[c][3]];
		int cc = 4;
		Vector4 cd;
		bool inside = closest_on_tetrahedron(cell_pts, cc, cd);
		if (inside) {
			pts[0] = cell_pts[0]; pts[1] = cell_pts[1];
			pts[2] = cell_pts[2]; pts[3] = cell_pts[3];
			count = 4;
			return closest_on_tetrahedron(pts, count, dir);
		}
		float dist = cd.length_squared();
		if (dist < min_dist) {
			min_dist = dist;
			best_cell = c;
		}
	}

	if (best_cell >= 0) {
		pts[0] = pts[cell_indices[best_cell][0]];
		pts[1] = pts[cell_indices[best_cell][1]];
		pts[2] = pts[cell_indices[best_cell][2]];
		pts[3] = pts[cell_indices[best_cell][3]];
		count = 4;
		return closest_on_tetrahedron(pts, count, dir);
	}

	dir = -pts[0];
	count = 1;
	return false;
}

// GJK do_simplex dispatcher
bool do_simplex(Vector4 pts[5], int &count, Vector4 &dir) {
	switch (count) {
		case 2:
			return closest_on_line(pts[0], pts[1], dir, count, pts);
		case 3:
			return closest_on_triangle(pts, count, dir);
		case 4:
			return closest_on_tetrahedron(pts, count, dir);
		case 5:
			return closest_on_pentachoron(pts, count, dir);
		default:
			dir = -pts[0];
			return false;
	}
}

} // anonymous namespace

// ============================================================================
// GJK Implementation
// ============================================================================
GJKResult gjk_4d(const MinkowskiPair &pair) {
	GJKResult result;

	// Initial direction: from center of B to center of A
	Vector4 dir = pair.xform_a.origin - pair.xform_b.origin;
	if (dir.length_squared() < PHYSICS_EPSILON) {
		dir = Vector4(1, 0, 0, 0);
	}

	// Get first support point
	Vector4 sa, sb;
	Vector4 S = pair.support(dir, sa, sb);

	Vector4 simplex[5];
	Vector4 simplex_a[5], simplex_b[5];
	simplex[0] = S;
	simplex_a[0] = sa;
	simplex_b[0] = sb;
	int simplex_count = 1;

	dir = -S;

	for (int iter = 0; iter < GJK_MAX_ITERATIONS; iter++) {
		if (dir.length_squared() < GJK_TOLERANCE * GJK_TOLERANCE) {
			// Direction is zero - origin is on the simplex boundary
			result.colliding = true;
			break;
		}

		S = pair.support(dir, sa, sb);

		// Check if the new point passes the origin
		float dot = S.dot(dir);
		if (dot < 0) {
			// No intersection - shapes are separated
			result.colliding = false;
			// Compute closest points
			// The closest point on the Minkowski difference is approximated by
			// the projection of origin onto the current simplex
			if (simplex_count == 1) {
				result.closest_on_a = simplex_a[0];
				result.closest_on_b = simplex_b[0];
			} else {
				// Use barycentric coordinates from the simplex
				// For simplicity, use the closest point from the last direction
				result.closest_on_a = sa;
				result.closest_on_b = sb;
			}
			result.distance = dir.length();
			return result;
		}

		// Add point to simplex
		// Shift existing points to make room for the new point at index 0
		for (int i = simplex_count; i > 0; i--) {
			simplex[i] = simplex[i-1];
			simplex_a[i] = simplex_a[i-1];
			simplex_b[i] = simplex_b[i-1];
		}
		simplex[0] = S;
		simplex_a[0] = sa;
		simplex_b[0] = sb;
		simplex_count++;

		// Check if simplex contains origin
		if (do_simplex(simplex, simplex_count, dir)) {
			result.colliding = true;
			break;
		}
	}

	// Store simplex for EPA
	result.simplex_count = simplex_count;
	for (int i = 0; i < simplex_count; i++) {
		result.simplex[i] = simplex[i];
		result.simplex_a[i] = simplex_a[i];
		result.simplex_b[i] = simplex_b[i];
	}

	return result;
}

// ============================================================================
// EPA Implementation in 4D
// The polytope boundary consists of tetrahedral cells (3-simplices).
// ============================================================================

namespace {

struct EPAFace {
	int v[4];      // Vertex indices
	Vector4 normal;
	float dist;    // Distance from origin to face
	bool obsolete = false;
};

// Compute the outward-facing normal of a tetrahedral face and its distance from origin.
// The normal points away from 'reference_point' (a vertex not on this face).
bool compute_face_normal(const std::vector<Vector4> &verts, const EPAFace &face,
						 const Vector4 &reference_point, Vector4 &normal, float &dist) {
	Vector4 A = verts[face.v[0]];
	Vector4 B = verts[face.v[1]];
	Vector4 C = verts[face.v[2]];
	Vector4 D = verts[face.v[3]];

	Vector4 AB = B - A, AC = C - A, AD = D - A;
	normal = triple_cross_4d(AB, AC, AD);

	float len = normal.length();
	if (len < PHYSICS_EPSILON) return false;
	normal /= len;

	// Ensure normal points away from the reference point
	if (normal.dot(A - reference_point) < 0) {
		normal = -normal;
	}

	dist = normal.dot(A);
	return true;
}

// Find the horizon (boundary triangles between visible and non-visible faces)
struct HorizonEdge {
	int v[3]; // Triangle vertices on the horizon
	int face_idx; // The non-visible face that shares this triangle
};

} // anonymous namespace

EPAResult epa_4d(const MinkowskiPair &pair, const GJKResult &gjk) {
	EPAResult result;

	if (gjk.simplex_count < 4) {
		// Need at least a tetrahedron to start EPA
		// Try to build one by adding support points
		return result;
	}

	std::vector<Vector4> vertices;
	vertices.reserve(64);

	// Initialize with the GJK simplex
	for (int i = 0; i < gjk.simplex_count; i++) {
		vertices.push_back(gjk.simplex[i]);
	}

	// If we only have a tetrahedron (4 points), we need to extend to a pentachoron
	// by adding a support point in the direction perpendicular to the tetrahedron
	if (gjk.simplex_count == 4) {
		Vector4 AB = vertices[1] - vertices[0];
		Vector4 AC = vertices[2] - vertices[0];
		Vector4 AD = vertices[3] - vertices[0];
		Vector4 perp = triple_cross_4d(AB, AC, AD);
		float plen = perp.length();
		if (plen < PHYSICS_EPSILON) return result;
		perp /= plen;

		// Try both directions
		Vector4 sa, sb;
		Vector4 p1 = pair.support(perp, sa, sb);
		Vector4 p2 = pair.support(-perp, sa, sb);

		// Pick the one further from the tetrahedron plane
		float d1 = fabsf(perp.dot(p1 - vertices[0]));
		float d2 = fabsf(perp.dot(p2 - vertices[0]));

		if (d1 > PHYSICS_EPSILON || d2 > PHYSICS_EPSILON) {
			vertices.push_back(d1 >= d2 ? p1 : p2);
		} else {
			return result; // Degenerate
		}
	}

	// Build initial polytope faces (5 tetrahedral cells of a pentachoron)
	std::vector<EPAFace> faces;
	faces.reserve(EPA_MAX_FACES);

	// A pentachoron with vertices 0,1,2,3,4 has 5 tetrahedral cells:
	// Each cell is formed by omitting one vertex
	int cell_indices[5][4] = {
		{1,2,3,4}, // opposite vertex 0
		{0,2,3,4}, // opposite vertex 1
		{0,1,3,4}, // opposite vertex 2
		{0,1,2,4}, // opposite vertex 3
		{0,1,2,3}  // opposite vertex 4
	};

	Vector4 centroid = (vertices[0] + vertices[1] + vertices[2] + vertices[3] + vertices[4]) * 0.2f;

	for (int i = 0; i < 5; i++) {
		EPAFace face;
		face.v[0] = cell_indices[i][0];
		face.v[1] = cell_indices[i][1];
		face.v[2] = cell_indices[i][2];
		face.v[3] = cell_indices[i][3];

		// Use centroid as reference to orient normal outward
		Vector4 ref = vertices[i]; // The opposite vertex (interior)
		if (!compute_face_normal(vertices, face, ref, face.normal, face.dist)) {
			// Degenerate face - ensure it points away from centroid
			face.normal = Vector4(0, 1, 0, 0);
			face.dist = 0;
		}
		// Normal should point away from the centroid
		if (face.normal.dot(vertices[face.v[0]] - centroid) < 0) {
			face.normal = -face.normal;
			face.dist = -face.dist;
		}
		faces.push_back(face);
	}

	// EPA main loop
	for (int iter = 0; iter < EPA_MAX_ITERATIONS; iter++) {
		// Find the closest face to the origin
		int closest_face = -1;
		float closest_dist = 1e30f;
		for (int i = 0; i < (int)faces.size(); i++) {
			if (faces[i].obsolete) continue;
			if (faces[i].dist < closest_dist) {
				closest_dist = faces[i].dist;
				closest_face = i;
			}
		}

		if (closest_face < 0) break;

		Vector4 search_dir = faces[closest_face].normal;

		// Get support point in the direction of the closest face's normal
		Vector4 sa, sb;
		Vector4 support = pair.support(search_dir, sa, sb);

		float support_dist = support.dot(search_dir);

		// Check for convergence
		if (support_dist - closest_dist < EPA_TOLERANCE) {
			// Converged - compute contact info
			result.valid = true;
			result.normal = faces[closest_face].normal;
			result.depth = closest_dist;

			// Compute contact point using barycentric coordinates on the closest face
			const EPAFace &cf = faces[closest_face];
			Vector4 A = vertices[cf.v[0]], B = vertices[cf.v[1]];
			Vector4 C = vertices[cf.v[2]], D = vertices[cf.v[3]];

			// Project origin onto the face to get barycentric coords
			Vector4 proj = result.normal * closest_dist;
			Vector4 AP = proj - A;
			Vector4 AB = B - A, AC = C - A, AD = D - A;

			// Solve AP = u*AB + v*AC + w*AD via gram matrix
			float g[3][3] = {
				{AB.dot(AB), AB.dot(AC), AB.dot(AD)},
				{AC.dot(AB), AC.dot(AC), AC.dot(AD)},
				{AD.dot(AB), AD.dot(AC), AD.dot(AD)}
			};
			float r[3] = {AP.dot(AB), AP.dot(AC), AP.dot(AD)};

			float det3 = g[0][0]*(g[1][1]*g[2][2] - g[1][2]*g[2][1])
						- g[0][1]*(g[1][0]*g[2][2] - g[1][2]*g[2][0])
						+ g[0][2]*(g[1][0]*g[2][1] - g[1][1]*g[2][0]);

			if (fabsf(det3) > PHYSICS_EPSILON) {
				float inv = 1.0f / det3;
				float u = inv * (r[0]*(g[1][1]*g[2][2] - g[1][2]*g[2][1])
								- g[0][1]*(r[1]*g[2][2] - g[1][2]*r[2])
								+ g[0][2]*(r[1]*g[2][1] - g[1][1]*r[2]));
				float v = inv * (g[0][0]*(r[1]*g[2][2] - g[1][2]*r[2])
								- r[0]*(g[1][0]*g[2][2] - g[1][2]*g[2][0])
								+ g[0][2]*(g[1][0]*r[2] - r[1]*g[2][0]));
				float w = inv * (g[0][0]*(g[1][1]*r[2] - r[1]*g[2][1])
								- g[0][1]*(g[1][0]*r[2] - r[1]*g[2][0])
								+ r[0]*(g[1][0]*g[2][1] - g[1][1]*g[2][0]));
				float s = 1.0f - u - v - w;

				// Clamp barycentric coords
				s = fmaxf(0, s); u = fmaxf(0, u); v = fmaxf(0, v); w = fmaxf(0, w);
				float total = s + u + v + w;
				if (total > PHYSICS_EPSILON) {
					s /= total; u /= total; v /= total; w /= total;
				} else {
					s = 0.25f; u = 0.25f; v = 0.25f; w = 0.25f;
				}

				// Contact point is the centroid of the Minkowski face projected back
				// Approximate: use the face normal and depth
				result.point_on_a = result.normal * (result.depth * 0.5f);
				result.point_on_b = -result.normal * (result.depth * 0.5f);
			} else {
				result.point_on_a = result.normal * (result.depth * 0.5f);
				result.point_on_b = -result.normal * (result.depth * 0.5f);
			}

			return result;
		}

		// Add support point as new vertex
		int new_vert = (int)vertices.size();
		vertices.push_back(support);

		// Mark all faces visible from the new point as obsolete
		std::vector<bool> visible(faces.size(), false);
		for (int i = 0; i < (int)faces.size(); i++) {
			if (faces[i].obsolete) continue;
			float d = faces[i].normal.dot(support - vertices[faces[i].v[0]]);
			if (d > PHYSICS_EPSILON) {
				visible[i] = true;
				faces[i].obsolete = true;
			}
		}

		// Find horizon: triangles shared between visible and non-visible faces
		// A triangle (3 vertices) is a ridge between two tetrahedral faces.
		// If one face is visible and the other is not, the triangle is on the horizon.

		// For each visible face, check each of its 4 sub-triangles
		// A sub-triangle of face {a,b,c,d} is obtained by dropping one vertex
		struct Triangle {
			int v[3];
			bool operator==(const Triangle &o) const {
				int sv[3] = {v[0], v[1], v[2]};
				int so[3] = {o.v[0], o.v[1], o.v[2]};
				std::sort(sv, sv+3);
				std::sort(so, so+3);
				return sv[0]==so[0] && sv[1]==so[1] && sv[2]==so[2];
			}
		};

		std::vector<Triangle> horizon_triangles;

		for (int i = 0; i < (int)faces.size(); i++) {
			if (!visible[i]) continue;
			// This face's 4 sub-triangles
			for (int skip = 0; skip < 4; skip++) {
				Triangle tri;
				int ti = 0;
				for (int j = 0; j < 4; j++) {
					if (j == skip) continue;
					tri.v[ti++] = faces[i].v[j];
				}

				// Check if this triangle is shared with a non-visible face
				bool is_horizon = false;
				for (int k = 0; k < (int)faces.size(); k++) {
					if (k == i || faces[k].obsolete || visible[k]) continue;
					// Check if face k contains this triangle
					for (int s = 0; s < 4; s++) {
						Triangle other_tri;
						int oi = 0;
						for (int j = 0; j < 4; j++) {
							if (j == s) continue;
							other_tri.v[oi++] = faces[k].v[j];
						}
						if (tri == other_tri) {
							is_horizon = true;
							break;
						}
					}
					if (is_horizon) break;
				}

				// Also check if no other non-obsolete face shares this triangle
				// (edge of the polytope)
				if (!is_horizon) {
					bool shared = false;
					for (int k = 0; k < (int)faces.size(); k++) {
						if (k == i || faces[k].obsolete) continue;
						if (visible[k]) continue;
						// Already checked above
					}
					if (!shared) {
						is_horizon = true;
					}
				}

				if (is_horizon) {
					// Check if we already have this triangle
					bool dup = false;
					for (const auto &ht : horizon_triangles) {
						if (ht == tri) { dup = true; break; }
					}
					if (!dup) {
						horizon_triangles.push_back(tri);
					}
				}
			}
		}

		// Create new faces connecting each horizon triangle to the new vertex
		for (const auto &tri : horizon_triangles) {
			if ((int)faces.size() >= EPA_MAX_FACES) break;

			EPAFace new_face;
			new_face.v[0] = tri.v[0];
			new_face.v[1] = tri.v[1];
			new_face.v[2] = tri.v[2];
			new_face.v[3] = new_vert;

			// Compute normal pointing away from origin (centroid approximation)
			if (!compute_face_normal(vertices, new_face, centroid, new_face.normal, new_face.dist)) {
				continue;
			}
			// Ensure outward facing (away from origin, so dist should be positive)
			if (new_face.dist < 0) {
				new_face.normal = -new_face.normal;
				new_face.dist = -new_face.dist;
				// Flip winding
				std::swap(new_face.v[0], new_face.v[1]);
			}

			faces.push_back(new_face);
		}
	}

	// If we get here, EPA didn't converge - return best result
	// Find closest non-obsolete face
	int closest_face = -1;
	float closest_dist = 1e30f;
	for (int i = 0; i < (int)faces.size(); i++) {
		if (faces[i].obsolete) continue;
		if (faces[i].dist < closest_dist) {
			closest_dist = faces[i].dist;
			closest_face = i;
		}
	}

	if (closest_face >= 0) {
		result.valid = true;
		result.normal = faces[closest_face].normal;
		result.depth = closest_dist;
		result.point_on_a = result.normal * (result.depth * 0.5f);
		result.point_on_b = -result.normal * (result.depth * 0.5f);
	}

	return result;
}

// ============================================================================
// Combined collision detection
// ============================================================================
CollisionResult collide_shapes_4d(
	const Shape4DInternal *shape_a, const Xform4 &xform_a,
	const Shape4DInternal *shape_b, const Xform4 &xform_b
) {
	CollisionResult result;

	// Special case: world boundary vs anything
	if (shape_a->type == SHAPE_4D_WORLD_BOUNDARY || shape_b->type == SHAPE_4D_WORLD_BOUNDARY) {
		const Shape4DInternal *boundary_shape;
		const Shape4DInternal *other_shape;
		const Xform4 *boundary_xf;
		const Xform4 *other_xf;
		bool swapped = false;

		if (shape_a->type == SHAPE_4D_WORLD_BOUNDARY) {
			boundary_shape = shape_a; boundary_xf = &xform_a;
			other_shape = shape_b; other_xf = &xform_b;
		} else {
			boundary_shape = shape_b; boundary_xf = &xform_b;
			other_shape = shape_a; other_xf = &xform_a;
			swapped = true;
		}

		const auto *wb = static_cast<const WorldBoundaryShape4DInternal *>(boundary_shape);
		// Transform normal to world space
		Vector4 world_normal = boundary_xf->xform_direction(wb->normal).normalized();
		Vector4 plane_point = boundary_xf->xform(wb->normal * wb->distance);
		float plane_d = world_normal.dot(plane_point);

		// Find deepest point on other shape in the -normal direction
		Vector4 local_dir = other_xf->xform_inv_direction(-world_normal);
		Vector4 deepest_local = other_shape->support(local_dir);
		Vector4 deepest_world = other_xf->xform(deepest_local);

		float depth = plane_d - world_normal.dot(deepest_world);
		if (depth > 0) {
			result.colliding = true;
			result.normal = swapped ? -world_normal : world_normal;
			result.depth = depth;
			result.point_on_b = swapped ? (deepest_world + world_normal * depth) : deepest_world;
			result.point_on_a = swapped ? deepest_world : (deepest_world + world_normal * depth);
		}
		return result;
	}

	// Special case: sphere vs sphere (exact)
	if (shape_a->type == SHAPE_4D_SPHERE && shape_b->type == SHAPE_4D_SPHERE) {
		const auto *sa = static_cast<const SphereShape4DInternal *>(shape_a);
		const auto *sb = static_cast<const SphereShape4DInternal *>(shape_b);
		Vector4 diff = xform_a.origin - xform_b.origin;
		float dist = diff.length();
		float radii = sa->radius + sb->radius;
		if (dist < radii) {
			result.colliding = true;
			if (dist > PHYSICS_EPSILON) {
				result.normal = diff / dist;
			} else {
				result.normal = Vector4(0, 1, 0, 0);
			}
			result.depth = radii - dist;
			result.point_on_a = xform_a.origin - result.normal * sa->radius;
			result.point_on_b = xform_b.origin + result.normal * sb->radius;
		}
		return result;
	}

	// General case: GJK + EPA
	MinkowskiPair mink;
	mink.shape_a = shape_a;
	mink.shape_b = shape_b;
	mink.xform_a = xform_a;
	mink.xform_b = xform_b;

	GJKResult gjk = gjk_4d(mink);

	if (!gjk.colliding) {
		result.colliding = false;
		return result;
	}

	// Run EPA
	EPAResult epa = epa_4d(mink, gjk);

	if (epa.valid) {
		result.colliding = true;
		result.normal = epa.normal;
		result.depth = epa.depth;

		// Compute contact points from the EPA normal and depth
		// support_A(-normal) and support_B(normal) give the contact witnesses
		Vector4 dir_a = xform_a.xform_inv_direction(-epa.normal);
		Vector4 dir_b = xform_b.xform_inv_direction(epa.normal);
		result.point_on_a = xform_a.xform(shape_a->support(dir_a));
		result.point_on_b = xform_b.xform(shape_b->support(dir_b));
	} else {
		// EPA failed - use a fallback
		result.colliding = true;
		Vector4 diff = xform_a.origin - xform_b.origin;
		float dist = diff.length();
		result.normal = dist > PHYSICS_EPSILON ? diff / dist : Vector4(0, 1, 0, 0);
		result.depth = 0.01f;
		result.point_on_a = xform_a.origin;
		result.point_on_b = xform_b.origin;
	}

	return result;
}

// ============================================================================
// Ray casting against a single shape
// Uses GJK ray cast algorithm: binary search with GJK distance queries
// ============================================================================
RayCastResult4D ray_cast_shape_4d(
	const Vector4 &ray_origin, const Vector4 &ray_dir, float ray_length,
	const Shape4DInternal *shape, const Xform4 &xform
) {
	RayCastResult4D result;

	// Special case: sphere
	if (shape->type == SHAPE_4D_SPHERE) {
		const auto *sphere = static_cast<const SphereShape4DInternal *>(shape);
		Vector4 oc = ray_origin - xform.origin;
		float a = ray_dir.dot(ray_dir);
		float b = 2.0f * oc.dot(ray_dir);
		float c = oc.dot(oc) - sphere->radius * sphere->radius;
		float disc = b*b - 4*a*c;
		if (disc < 0) return result;
		float sqrt_disc = sqrtf(disc);
		float t = (-b - sqrt_disc) / (2*a);
		if (t < 0) t = (-b + sqrt_disc) / (2*a);
		if (t >= 0 && t <= ray_length) {
			result.hit = true;
			result.t = t / ray_length;
			result.point = ray_origin + ray_dir * t;
			result.normal = (result.point - xform.origin).normalized();
		}
		return result;
	}

	// Special case: world boundary (infinite plane)
	if (shape->type == SHAPE_4D_WORLD_BOUNDARY) {
		const auto *wb = static_cast<const WorldBoundaryShape4DInternal *>(shape);
		Vector4 world_normal = xform.xform_direction(wb->normal).normalized();
		Vector4 plane_point = xform.xform(wb->normal * wb->distance);
		float denom = ray_dir.dot(world_normal);
		if (fabsf(denom) < PHYSICS_EPSILON) return result;
		float t = (plane_point - ray_origin).dot(world_normal) / denom;
		if (t >= 0 && t <= ray_length) {
			result.hit = true;
			result.t = t / ray_length;
			result.point = ray_origin + ray_dir * t;
			result.normal = world_normal;
			if (denom > 0) result.normal = -result.normal;
		}
		return result;
	}

	// Special case: box (exact slab test)
	if (shape->type == SHAPE_4D_BOX) {
		const auto *box = static_cast<const BoxShape4DInternal *>(shape);
		// Transform ray to local space
		Vector4 local_origin = xform.xform_inv(ray_origin);
		Vector4 local_dir = xform.xform_inv_direction(ray_dir);

		AABB4 local_aabb = box->get_aabb();
		float t_min, t_max;
		if (local_aabb.intersect_ray(local_origin, local_dir, t_min, t_max)) {
			if (t_min >= 0 && t_min <= ray_length) {
				result.hit = true;
				result.t = t_min / ray_length;
				result.point = ray_origin + ray_dir * t_min;
				// Compute normal from the face that was hit
				Vector4 local_hit = local_origin + local_dir * t_min;
				Vector4 normal(0, 0, 0, 0);
				float min_dist = 1e30f;
				const float *he = &box->half_extents.x;
				const float *lh = &local_hit.x;
				float *n = &normal.x;
				for (int i = 0; i < 4; i++) {
					float d_pos = fabsf(lh[i] - he[i]);
					float d_neg = fabsf(lh[i] + he[i]);
					if (d_pos < min_dist) { min_dist = d_pos; memset(n, 0, 16); n[i] = 1.0f; }
					if (d_neg < min_dist) { min_dist = d_neg; memset(n, 0, 16); n[i] = -1.0f; }
				}
				result.normal = xform.xform_direction(normal).normalized();
			} else if (t_min < 0 && t_max >= 0) {
				// Ray starts inside box
				result.hit = true;
				result.t = 0;
				result.point = ray_origin;
				result.normal = (ray_origin - xform.origin).normalized();
			}
		}
		return result;
	}

	// General case: GJK-based ray cast
	// March along the ray, using GJK distance queries to find the intersection point
	// Use the approach from "Ray Casting against General Convex Objects with Application to Continuous Collision Detection"

	SphereShape4DInternal point_shape;
	point_shape.radius = 0.001f; // Tiny sphere as point approximation

	float t_lo = 0.0f, t_hi = ray_length;
	Vector4 closest_normal;

	// Binary search along the ray
	for (int iter = 0; iter < 32; iter++) {
		float t_mid = (t_lo + t_hi) * 0.5f;
		Vector4 point = ray_origin + ray_dir * t_mid;

		Xform4 point_xf;
		point_xf.origin = point;

		MinkowskiPair mink;
		mink.shape_a = &point_shape;
		mink.shape_b = shape;
		mink.xform_a = point_xf;
		mink.xform_b = xform;

		GJKResult gjk = gjk_4d(mink);

		if (gjk.colliding) {
			t_hi = t_mid;
			// Get approximate normal
			closest_normal = (point - xform.origin).normalized();
		} else {
			t_lo = t_mid;
		}

		if (t_hi - t_lo < PHYSICS_EPSILON) break;
	}

	// Check if we found an intersection
	if (t_hi < ray_length - PHYSICS_EPSILON) {
		result.hit = true;
		result.t = t_hi / ray_length;
		result.point = ray_origin + ray_dir * t_hi;
		// Compute a better normal using support function
		Vector4 local_dir = xform.xform_inv_direction(-(result.point - xform.origin));
		Vector4 support_pt = xform.xform(shape->support(local_dir));
		result.normal = (result.point - support_pt).normalized();
		if (result.normal.length_squared() < PHYSICS_EPSILON) {
			result.normal = closest_normal;
		}
	}

	return result;
}

// ============================================================================
// Point containment test
// ============================================================================
bool point_in_shape_4d(
	const Vector4 &point,
	const Shape4DInternal *shape, const Xform4 &xform
) {
	// Transform point to local space
	Vector4 local = xform.xform_inv(point);

	switch (shape->type) {
		case SHAPE_4D_SPHERE: {
			const auto *s = static_cast<const SphereShape4DInternal *>(shape);
			return local.length_squared() <= s->radius * s->radius;
		}
		case SHAPE_4D_BOX: {
			const auto *b = static_cast<const BoxShape4DInternal *>(shape);
			return fabsf(local.x) <= b->half_extents.x &&
				   fabsf(local.y) <= b->half_extents.y &&
				   fabsf(local.z) <= b->half_extents.z &&
				   fabsf(local.w) <= b->half_extents.w;
		}
		case SHAPE_4D_CAPSULE: {
			const auto *c = static_cast<const CapsuleShape4DInternal *>(shape);
			float half_h = (c->height - 2.0f * c->radius) * 0.5f;
			if (half_h < 0) half_h = 0;
			// Closest point on capsule axis
			float y_clamped = fmaxf(-half_h, fminf(half_h, local.y));
			Vector4 axis_point(0, y_clamped, 0, 0);
			return (local - axis_point).length_squared() <= c->radius * c->radius;
		}
		case SHAPE_4D_CYLINDER: {
			const auto *cyl = static_cast<const CylinderShape4DInternal *>(shape);
			float half_h = cyl->height * 0.5f;
			if (fabsf(local.y) > half_h) return false;
			Vector4 lateral(local.x, 0, local.z, local.w);
			return lateral.length_squared() <= cyl->radius * cyl->radius;
		}
		case SHAPE_4D_WORLD_BOUNDARY: {
			const auto *wb = static_cast<const WorldBoundaryShape4DInternal *>(shape);
			return local.dot(wb->normal) <= wb->distance;
		}
		default: {
			// General case: use GJK with a point
			SphereShape4DInternal point_shape;
			point_shape.radius = 0.0f;
			Xform4 point_xf;
			point_xf.origin = point;

			MinkowskiPair mink;
			mink.shape_a = &point_shape;
			mink.shape_b = shape;
			mink.xform_a = point_xf;
			mink.xform_b = xform;
			GJKResult gjk = gjk_4d(mink);
			return gjk.colliding;
		}
	}
}
