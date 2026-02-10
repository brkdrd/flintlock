#include "hyper_capsule_shape_4d.h"
#include <cmath>

// -- Constructor -------------------------------------------------------------

HyperCapsuleShape4D::HyperCapsuleShape4D(real_t p_radius, real_t p_height) :
		radius(p_radius), height(p_height) {
}

// -- Accessors ---------------------------------------------------------------

void HyperCapsuleShape4D::set_radius(real_t p_radius) {
	radius = p_radius;
}

real_t HyperCapsuleShape4D::get_radius() const {
	return radius;
}

void HyperCapsuleShape4D::set_height(real_t p_height) {
	height = p_height;
}

real_t HyperCapsuleShape4D::get_height() const {
	return height;
}

// -- Shape4D interface -------------------------------------------------------

Vector4 HyperCapsuleShape4D::get_support(const Vector4 &p_direction) const {
	// A capsule is a Minkowski sum: line_segment ⊕ sphere
	// Support(capsule, d) = Support(line_segment, d) + Support(sphere, d)
	//
	// The line segment runs along W-axis from (0,0,0,-height/2) to (0,0,0,+height/2)
	// Support of line segment: choose endpoint in direction of d.w

	// Handle zero or near-zero direction
	real_t dir_len_sq = p_direction.length_squared();
	if (dir_len_sq < CMP_EPSILON * CMP_EPSILON) {
		// Return top of capsule
		return Vector4(radius, 0, 0, height / 2.0);
	}

	// Choose endpoint based on W component of direction
	real_t half_height = height / 2.0;
	Vector4 endpoint;
	if (p_direction.w >= 0) {
		endpoint = Vector4(0, 0, 0, half_height);
	} else {
		endpoint = Vector4(0, 0, 0, -half_height);
	}

	// Add spherical offset in direction
	Vector4 sphere_offset = p_direction.normalized() * radius;

	return endpoint + sphere_offset;
}

AABB4D HyperCapsuleShape4D::get_aabb(const Transform4D &p_transform) const {
	// The capsule extends radius in XYZ directions and height/2 + radius in W direction
	// We need to transform these extents through the transform

	// Use support function approach
	Vector4 axes[4] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, 0, 1)
	};

	// Initialize with first axis
	Vector4 local_dir_0 = p_transform.basis.xform_inv(axes[0]);
	Vector4 support_pos_0 = get_support(local_dir_0);
	Vector4 support_neg_0 = get_support(-local_dir_0);
	Vector4 world_pos_0 = p_transform.xform(support_pos_0);
	Vector4 world_neg_0 = p_transform.xform(support_neg_0);

	Vector4 min_corner(world_neg_0.x, 0, 0, 0);
	Vector4 max_corner(world_pos_0.x, 0, 0, 0);

	// Process remaining axes
	for (int i = 1; i < 4; i++) {
		Vector4 local_dir_pos = p_transform.basis.xform_inv(axes[i]);
		Vector4 local_dir_neg = -local_dir_pos;

		Vector4 support_pos = get_support(local_dir_pos);
		Vector4 support_neg = get_support(local_dir_neg);

		Vector4 world_pos = p_transform.xform(support_pos);
		Vector4 world_neg = p_transform.xform(support_neg);

		min_corner.components[i] = world_neg.components[i];
		max_corner.components[i] = world_pos.components[i];
	}

	return AABB4D(min_corner, max_corner - min_corner);
}

bool HyperCapsuleShape4D::contains_point(const Vector4 &p_point) const {
	// A point is inside the capsule if its distance to the central axis segment
	// is <= radius

	real_t half_height = height / 2.0;

	// Clamp the W coordinate to the segment bounds
	real_t clamped_w = p_point.w;
	if (clamped_w < -half_height) {
		clamped_w = -half_height;
	} else if (clamped_w > half_height) {
		clamped_w = half_height;
	}

	// Closest point on axis segment
	Vector4 closest_on_axis(0, 0, 0, clamped_w);

	// Distance from point to closest point on axis
	real_t dist_sq = (p_point - closest_on_axis).length_squared();

	return dist_sq <= radius * radius;
}
