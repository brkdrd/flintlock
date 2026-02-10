#include "hyper_box_shape_4d.h"
#include <cmath>
#include <algorithm>

// -- Constructor -------------------------------------------------------------

HyperBoxShape4D::HyperBoxShape4D(const Vector4 &p_half_extents) :
		half_extents(p_half_extents) {
}

// -- Accessors ---------------------------------------------------------------

void HyperBoxShape4D::set_half_extents(const Vector4 &p_half_extents) {
	half_extents = p_half_extents;
}

Vector4 HyperBoxShape4D::get_half_extents() const {
	return half_extents;
}

// -- Shape4D interface -------------------------------------------------------

Vector4 HyperBoxShape4D::get_support(const Vector4 &p_direction) const {
	// Select the corner that maximizes dot(corner, direction)
	// For a box, this is simply the corner where each coordinate
	// has the same sign as the corresponding direction component
	// Note: For zero components, we use the negative extent (sign(0) = -1)

	Vector4 corner;
	corner.x = (p_direction.x > 0) ? half_extents.x : -half_extents.x;
	corner.y = (p_direction.y > 0) ? half_extents.y : -half_extents.y;
	corner.z = (p_direction.z > 0) ? half_extents.z : -half_extents.z;
	corner.w = (p_direction.w > 0) ? half_extents.w : -half_extents.w;

	return corner;
}

AABB4D HyperBoxShape4D::get_aabb(const Transform4D &p_transform) const {
	// Strategy: Transform all 16 vertices of the tesseract, then compute
	// min/max to get the enclosing axis-aligned bounding box.

	// Generate all 16 vertices (2^4 combinations of ±half_extents)
	Vector4 vertices[16];
	for (int i = 0; i < 16; i++) {
		vertices[i] = Vector4(
				(i & 1) ? half_extents.x : -half_extents.x,
				(i & 2) ? half_extents.y : -half_extents.y,
				(i & 4) ? half_extents.z : -half_extents.z,
				(i & 8) ? half_extents.w : -half_extents.w);
	}

	// Transform first vertex to initialize min/max
	Vector4 first = p_transform.xform(vertices[0]);
	Vector4 min_corner = first;
	Vector4 max_corner = first;

	// Transform remaining vertices and update min/max
	for (int i = 1; i < 16; i++) {
		Vector4 transformed = p_transform.xform(vertices[i]);
		min_corner.x = std::min(min_corner.x, transformed.x);
		min_corner.y = std::min(min_corner.y, transformed.y);
		min_corner.z = std::min(min_corner.z, transformed.z);
		min_corner.w = std::min(min_corner.w, transformed.w);

		max_corner.x = std::max(max_corner.x, transformed.x);
		max_corner.y = std::max(max_corner.y, transformed.y);
		max_corner.z = std::max(max_corner.z, transformed.z);
		max_corner.w = std::max(max_corner.w, transformed.w);
	}

	return AABB4D(min_corner, max_corner - min_corner);
}

bool HyperBoxShape4D::contains_point(const Vector4 &p_point) const {
	// Point is inside if all coordinates are within [-extent, +extent]
	return std::abs(p_point.x) <= half_extents.x &&
		   std::abs(p_point.y) <= half_extents.y &&
		   std::abs(p_point.z) <= half_extents.z &&
		   std::abs(p_point.w) <= half_extents.w;
}
