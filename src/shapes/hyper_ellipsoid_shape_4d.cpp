#include "hyper_ellipsoid_shape_4d.h"
#include <cmath>

// -- Constructor -------------------------------------------------------------

HyperEllipsoidShape4D::HyperEllipsoidShape4D(const Vector4 &p_radii) :
		radii(p_radii) {
}

// -- Accessors ---------------------------------------------------------------

void HyperEllipsoidShape4D::set_radii(const Vector4 &p_radii) {
	radii = p_radii;
}

Vector4 HyperEllipsoidShape4D::get_radii() const {
	return radii;
}

// -- Shape4D interface -------------------------------------------------------

Vector4 HyperEllipsoidShape4D::get_support(const Vector4 &p_direction) const {
	// For an ellipsoid, the support function in direction d is:
	// s = (radii² * d) / |radii * d|
	// where * denotes component-wise multiplication
	//
	// This is derived from finding the point on the ellipsoid surface
	// that maximizes dot(point, direction)

	// Handle zero or near-zero direction
	real_t dir_len_sq = p_direction.length_squared();
	if (dir_len_sq < CMP_EPSILON * CMP_EPSILON) {
		// Return any point on surface (choose largest radius direction)
		if (radii.x >= radii.y && radii.x >= radii.z && radii.x >= radii.w) {
			return Vector4(radii.x, 0, 0, 0);
		} else if (radii.y >= radii.z && radii.y >= radii.w) {
			return Vector4(0, radii.y, 0, 0);
		} else if (radii.z >= radii.w) {
			return Vector4(0, 0, radii.z, 0);
		} else {
			return Vector4(0, 0, 0, radii.w);
		}
	}

	// Component-wise multiply: radii * direction
	Vector4 scaled_dir(
		radii.x * p_direction.x,
		radii.y * p_direction.y,
		radii.z * p_direction.z,
		radii.w * p_direction.w
	);

	// Normalize and scale by radii again
	real_t scaled_len = scaled_dir.length();
	if (scaled_len < CMP_EPSILON) {
		return Vector4(radii.x, 0, 0, 0);
	}

	Vector4 normalized = scaled_dir / scaled_len;

	// Component-wise multiply by radii
	return Vector4(
		normalized.x * radii.x,
		normalized.y * radii.y,
		normalized.z * radii.z,
		normalized.w * radii.w
	);
}

AABB4D HyperEllipsoidShape4D::get_aabb(const Transform4D &p_transform) const {
	// For an axis-aligned ellipsoid, the AABB is simply ±radii
	// For a transformed ellipsoid, we need to find the extent in each axis direction

	// Transform the ellipsoid by transforming its axes and finding extrema
	// We use the support function approach: find support in ±axis directions

	Vector4 axes[4] = {
		Vector4(1, 0, 0, 0),
		Vector4(0, 1, 0, 0),
		Vector4(0, 0, 1, 0),
		Vector4(0, 0, 0, 1)
	};

	// Transform first axis to initialize
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

bool HyperEllipsoidShape4D::contains_point(const Vector4 &p_point) const {
	// A point is inside the ellipsoid if:
	// (x/a)² + (y/b)² + (z/c)² + (w/d)² <= 1

	real_t sum =
		(p_point.x * p_point.x) / (radii.x * radii.x) +
		(p_point.y * p_point.y) / (radii.y * radii.y) +
		(p_point.z * p_point.z) / (radii.z * radii.z) +
		(p_point.w * p_point.w) / (radii.w * radii.w);

	return sum <= 1.0 + CMP_EPSILON;
}
