#pragma once

#include "physics_types_4d.h"
#include <vector>

// ============================================================================
// Internal 4D collision shape representations
// Each shape provides:
//   - support(dir): furthest point in direction (for GJK)
//   - get_aabb(): local-space AABB (for broadphase)
//   - compute_inertia(mass): inertia tensor (for dynamics)
// ============================================================================

enum ShapeType4D {
	SHAPE_4D_SPHERE = 0,
	SHAPE_4D_BOX = 1,
	SHAPE_4D_CAPSULE = 2,
	SHAPE_4D_CYLINDER = 3,
	SHAPE_4D_CONVEX_HULL = 4,
	SHAPE_4D_CONCAVE_MESH = 5,
	SHAPE_4D_WORLD_BOUNDARY = 6,
	SHAPE_4D_SEPARATION_RAY = 7,
};

// ============================================================================
// Base shape interface
// ============================================================================
struct Shape4DInternal {
	ShapeType4D type;
	float margin = 0.04f;

	virtual ~Shape4DInternal() = default;
	virtual Vector4 support(const Vector4 &dir) const = 0;
	virtual AABB4 get_aabb() const = 0;
	virtual Inertia4 compute_inertia(float mass) const = 0;
};

// ============================================================================
// SphereShape - 4D hypersphere
// ============================================================================
struct SphereShape4DInternal : public Shape4DInternal {
	float radius = 0.5f;

	SphereShape4DInternal() { type = SHAPE_4D_SPHERE; }
	SphereShape4DInternal(float r) : radius(r) { type = SHAPE_4D_SPHERE; }

	Vector4 support(const Vector4 &dir) const override {
		float len = dir.length();
		if (len < PHYSICS_EPSILON) return Vector4(radius, 0, 0, 0);
		return dir * (radius / len);
	}

	AABB4 get_aabb() const override {
		Vector4 r(radius, radius, radius, radius);
		return AABB4(-r, r);
	}

	Inertia4 compute_inertia(float mass) const override {
		// I = (1/3) * m * r^2 for all rotation planes (4D uniform ball)
		float i = mass * radius * radius / 3.0f;
		return Inertia4::uniform(i);
	}
};

// ============================================================================
// BoxShape - 4D hyperbox (tesseract)
// ============================================================================
struct BoxShape4DInternal : public Shape4DInternal {
	Vector4 half_extents = Vector4(0.5f, 0.5f, 0.5f, 0.5f);

	BoxShape4DInternal() { type = SHAPE_4D_BOX; }
	BoxShape4DInternal(const Vector4 &he) : half_extents(he) { type = SHAPE_4D_BOX; }

	Vector4 support(const Vector4 &dir) const override {
		return Vector4(
			dir.x >= 0 ? half_extents.x : -half_extents.x,
			dir.y >= 0 ? half_extents.y : -half_extents.y,
			dir.z >= 0 ? half_extents.z : -half_extents.z,
			dir.w >= 0 ? half_extents.w : -half_extents.w
		);
	}

	AABB4 get_aabb() const override {
		return AABB4(-half_extents, half_extents);
	}

	Inertia4 compute_inertia(float mass) const override {
		// I_{AB} = (m/3) * (h_A^2 + h_B^2)
		float hx2 = half_extents.x * half_extents.x;
		float hy2 = half_extents.y * half_extents.y;
		float hz2 = half_extents.z * half_extents.z;
		float hw2 = half_extents.w * half_extents.w;
		float f = mass / 3.0f;
		return Inertia4::diagonal(
			f * (hx2 + hy2), // XY
			f * (hx2 + hz2), // XZ
			f * (hx2 + hw2), // XW
			f * (hy2 + hz2), // YZ
			f * (hy2 + hw2), // YW
			f * (hz2 + hw2)  // ZW
		);
	}
};

// ============================================================================
// CapsuleShape - sphere swept along Y axis
// ============================================================================
struct CapsuleShape4DInternal : public Shape4DInternal {
	float radius = 0.5f;
	float height = 2.0f; // total height including caps

	CapsuleShape4DInternal() { type = SHAPE_4D_CAPSULE; }
	CapsuleShape4DInternal(float r, float h) : radius(r), height(h) { type = SHAPE_4D_CAPSULE; }

	Vector4 support(const Vector4 &dir) const override {
		float half_height = (height - 2.0f * radius) * 0.5f;
		if (half_height < 0) half_height = 0;
		Vector4 center(0, dir.y >= 0 ? half_height : -half_height, 0, 0);
		float len = dir.length();
		if (len < PHYSICS_EPSILON) return center;
		return center + dir * (radius / len);
	}

	AABB4 get_aabb() const override {
		float half_h = height * 0.5f;
		return AABB4(
			Vector4(-radius, -half_h, -radius, -radius),
			Vector4( radius,  half_h,  radius,  radius)
		);
	}

	Inertia4 compute_inertia(float mass) const override {
		// Approximate as cylinder + two hemispheres
		float i_radial = mass * radius * radius / 3.0f;
		float half_h = height * 0.5f;
		float i_axial = mass * (radius * radius / 3.0f + half_h * half_h / 3.0f);
		// Planes involving Y axis get the axial component
		return Inertia4::diagonal(
			i_axial,  // XY (involves Y)
			i_radial, // XZ
			i_radial, // XW
			i_axial,  // YZ (involves Y)
			i_axial,  // YW (involves Y)
			i_radial  // ZW
		);
	}
};

// ============================================================================
// CylinderShape - disk (3-sphere cross-section) extruded along Y axis
// ============================================================================
struct CylinderShape4DInternal : public Shape4DInternal {
	float radius = 0.5f;
	float height = 2.0f;

	CylinderShape4DInternal() { type = SHAPE_4D_CYLINDER; }
	CylinderShape4DInternal(float r, float h) : radius(r), height(h) { type = SHAPE_4D_CYLINDER; }

	Vector4 support(const Vector4 &dir) const override {
		float half_h = height * 0.5f;
		// Y component: clamp to half-height
		float sy = dir.y >= 0 ? half_h : -half_h;
		// XZW component: project onto 3-sphere of given radius
		Vector4 lateral(dir.x, 0, dir.z, dir.w);
		float lat_len = lateral.length();
		float sx, sz, sw;
		if (lat_len > PHYSICS_EPSILON) {
			float scale = radius / lat_len;
			sx = dir.x * scale;
			sz = dir.z * scale;
			sw = dir.w * scale;
		} else {
			sx = radius; sz = 0; sw = 0;
		}
		return Vector4(sx, sy, sz, sw);
	}

	AABB4 get_aabb() const override {
		float half_h = height * 0.5f;
		return AABB4(
			Vector4(-radius, -half_h, -radius, -radius),
			Vector4( radius,  half_h,  radius,  radius)
		);
	}

	Inertia4 compute_inertia(float mass) const override {
		float r2 = radius * radius;
		float h2 = (height * 0.5f) * (height * 0.5f);
		float f = mass / 3.0f;
		return Inertia4::diagonal(
			f * (r2 + h2),  // XY
			f * (r2 + r2),  // XZ
			f * (r2 + r2),  // XW
			f * (r2 + h2),  // YZ
			f * (r2 + h2),  // YW
			f * (r2 + r2)   // ZW
		);
	}
};

// ============================================================================
// ConvexHullShape - convex polytope defined by a set of vertices
// ============================================================================
struct ConvexHullShape4DInternal : public Shape4DInternal {
	std::vector<Vector4> points;

	ConvexHullShape4DInternal() { type = SHAPE_4D_CONVEX_HULL; }

	Vector4 support(const Vector4 &dir) const override {
		if (points.empty()) return Vector4(0, 0, 0, 0);
		float max_dot = points[0].dot(dir);
		int max_idx = 0;
		for (int i = 1; i < (int)points.size(); i++) {
			float d = points[i].dot(dir);
			if (d > max_dot) {
				max_dot = d;
				max_idx = i;
			}
		}
		return points[max_idx];
	}

	AABB4 get_aabb() const override {
		if (points.empty()) return AABB4();
		AABB4 aabb(points[0], points[0]);
		for (size_t i = 1; i < points.size(); i++) {
			aabb = aabb.expanded(points[i]);
		}
		return aabb;
	}

	Inertia4 compute_inertia(float mass) const override {
		if (points.empty()) return Inertia4::uniform(mass);
		// Approximate using AABB
		AABB4 aabb = get_aabb();
		Vector4 he = aabb.get_half_extents();
		float hx2 = he.x * he.x, hy2 = he.y * he.y;
		float hz2 = he.z * he.z, hw2 = he.w * he.w;
		float f = mass / 3.0f;
		return Inertia4::diagonal(
			f*(hx2+hy2), f*(hx2+hz2), f*(hx2+hw2),
			f*(hy2+hz2), f*(hy2+hw2), f*(hz2+hw2)
		);
	}
};

// ============================================================================
// ConcaveMeshShape - tetrahedral mesh (static bodies only)
// Stores triangles (3-simplices) of the boundary for ray casting and
// decomposes into convex pieces for collision detection.
// ============================================================================
struct ConcaveMeshShape4DInternal : public Shape4DInternal {
	// Vertices stored as groups of 4 floats (x,y,z,w)
	// Tetrahedra (cells) stored as groups of 4 vertex indices
	std::vector<Vector4> vertices;
	struct Tetrahedron {
		int idx[4];
	};
	std::vector<Tetrahedron> tetrahedra;

	ConcaveMeshShape4DInternal() { type = SHAPE_4D_CONCAVE_MESH; }

	// Support function: not meaningful for concave shapes, return furthest vertex
	Vector4 support(const Vector4 &dir) const override {
		if (vertices.empty()) return Vector4(0, 0, 0, 0);
		float max_dot = vertices[0].dot(dir);
		int max_idx = 0;
		for (int i = 1; i < (int)vertices.size(); i++) {
			float d = vertices[i].dot(dir);
			if (d > max_dot) {
				max_dot = d;
				max_idx = i;
			}
		}
		return vertices[max_idx];
	}

	AABB4 get_aabb() const override {
		if (vertices.empty()) return AABB4();
		AABB4 aabb(vertices[0], vertices[0]);
		for (size_t i = 1; i < vertices.size(); i++) {
			aabb = aabb.expanded(vertices[i]);
		}
		return aabb;
	}

	Inertia4 compute_inertia(float mass) const override {
		return Inertia4::uniform(mass); // Static bodies don't need inertia
	}
};

// ============================================================================
// WorldBoundaryShape - infinite hyperplane defined by normal and distance
// ============================================================================
struct WorldBoundaryShape4DInternal : public Shape4DInternal {
	Vector4 normal = Vector4(0, 1, 0, 0);
	float distance = 0.0f;

	WorldBoundaryShape4DInternal() { type = SHAPE_4D_WORLD_BOUNDARY; }
	WorldBoundaryShape4DInternal(const Vector4 &n, float d) : normal(n), distance(d) {
		type = SHAPE_4D_WORLD_BOUNDARY;
	}

	Vector4 support(const Vector4 &dir) const override {
		// For an infinite plane, support in the normal direction returns a point on the plane,
		// support in any other direction returns infinity.
		// For GJK, we approximate with a large but finite extent.
		const float BIG = 1e4f;
		// Project dir onto the plane tangent space and add normal*distance
		float dot = dir.dot(normal);
		Vector4 tangent_dir = dir - normal * dot;
		float tlen = tangent_dir.length();
		Vector4 result = normal * distance;
		if (tlen > PHYSICS_EPSILON) {
			result += tangent_dir * (BIG / tlen);
		}
		return result;
	}

	AABB4 get_aabb() const override {
		// Infinite - return a huge AABB
		const float BIG = 1e5f;
		return AABB4(Vector4(-BIG, -BIG, -BIG, -BIG), Vector4(BIG, BIG, BIG, BIG));
	}

	Inertia4 compute_inertia(float mass) const override {
		return Inertia4::uniform(0); // Static, infinite mass
	}
};

// ============================================================================
// SeparationRayShape - ray used for character controller ground detection
// Extends from origin along -Y by 'length'
// ============================================================================
struct SeparationRayShape4DInternal : public Shape4DInternal {
	float length = 1.0f;
	bool slide_on_slope = false;

	SeparationRayShape4DInternal() { type = SHAPE_4D_SEPARATION_RAY; }

	Vector4 support(const Vector4 &dir) const override {
		// The shape is a line segment from (0,0,0,0) to (0,-length,0,0)
		if (dir.y < 0) {
			return Vector4(0, -length, 0, 0);
		}
		return Vector4(0, 0, 0, 0);
	}

	AABB4 get_aabb() const override {
		return AABB4(Vector4(0, -length, 0, 0), Vector4(0, 0, 0, 0));
	}

	Inertia4 compute_inertia(float mass) const override {
		return Inertia4::uniform(mass * length * length / 3.0f);
	}
};

// ============================================================================
// Utility: Compute world-space AABB from a shape and transform
// ============================================================================
inline AABB4 compute_world_aabb(const Shape4DInternal *shape, const Xform4 &xf) {
	AABB4 local = shape->get_aabb();

	// Transform the 16 corners of the 4D AABB and find the bounding box
	// Optimization: use the abs of each basis column times the half-extents
	Vector4 center = xf.xform(local.get_center());
	Vector4 he = local.get_half_extents();

	// Each axis of the world AABB is the sum of |basis_col_i| * half_extent_i
	Vector4 world_he(0, 0, 0, 0);
	for (int col = 0; col < 4; col++) {
		Vector4 abs_col = vec4_abs(xf.basis.get_column(col));
		float extent;
		switch (col) {
			case 0: extent = he.x; break;
			case 1: extent = he.y; break;
			case 2: extent = he.z; break;
			default: extent = he.w; break;
		}
		world_he += abs_col * extent;
	}

	return AABB4::from_center_extents(center, world_he);
}

// ============================================================================
// Minkowski support function for GJK: support_A(dir) - support_B(-dir)
// Works with transformed shapes.
// ============================================================================
struct MinkowskiPair {
	const Shape4DInternal *shape_a;
	const Shape4DInternal *shape_b;
	Xform4 xform_a;
	Xform4 xform_b;

	// Returns the support point on the Minkowski difference A - B
	// Also outputs the individual support points on A and B
	Vector4 support(const Vector4 &dir, Vector4 &support_a_out, Vector4 &support_b_out) const {
		// Transform direction to local space of each shape
		Vector4 dir_a = xform_a.xform_inv_direction(dir);
		Vector4 dir_b = xform_b.xform_inv_direction(-dir);

		// Get local support points
		Vector4 local_a = shape_a->support(dir_a);
		Vector4 local_b = shape_b->support(dir_b);

		// Transform back to world space
		support_a_out = xform_a.xform(local_a);
		support_b_out = xform_b.xform(local_b);

		return support_a_out - support_b_out;
	}
};
