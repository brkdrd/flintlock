#include "hyper_sphere_shape_4d.h"

// -- Constructor -------------------------------------------------------------

HyperSphereShape4D::HyperSphereShape4D(real_t p_radius) :
		radius(p_radius) {
}

// -- Accessors ---------------------------------------------------------------

void HyperSphereShape4D::set_radius(real_t p_radius) {
	radius = p_radius;
}

real_t HyperSphereShape4D::get_radius() const {
	return radius;
}

// -- Shape4D interface -------------------------------------------------------

Vector4 HyperSphereShape4D::get_support(const Vector4 &p_direction) const {
	// Handle zero or near-zero direction
	real_t len_sq = p_direction.length_squared();
	if (len_sq < CMP_EPSILON * CMP_EPSILON) {
		// Degenerate case: return any point on surface
		return Vector4(radius, 0, 0, 0);
	}

	// Return point on surface in direction of p_direction
	return p_direction.normalized() * radius;
}

AABB4D HyperSphereShape4D::get_aabb(const Transform4D &p_transform) const {
	// For a sphere, the AABB is axis-aligned regardless of rotation
	// Only translation affects position
	// Scale is encoded in the transform's basis

	// Get world-space center
	Vector4 center = p_transform.xform(Vector4(0, 0, 0, 0));

	// Compute world-space radius by transforming a point on the surface
	// We use the fact that any radius vector transforms the same way
	Vector4 radius_vec = p_transform.basis_xform(Vector4(radius, 0, 0, 0));
	real_t world_radius = radius_vec.length();

	// AABB extends radius in all directions from center
	Vector4 extent(world_radius, world_radius, world_radius, world_radius);
	return AABB4D(center - extent, extent * 2.0);
}

bool HyperSphereShape4D::contains_point(const Vector4 &p_point) const {
	// Point is in local space (transform already applied by caller)
	return p_point.length_squared() <= radius * radius;
}
