#include "slicer4d.h"
#include "sphere_slicer.h"
#include "polytope_slicer.h"

SliceResult Slicer4D::slice_shape(
	const Shape4D *p_shape,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	if (!p_shape) {
		return SliceResult();
	}

	// Try to cast to specific shape types and dispatch
	if (const HyperSphereShape4D *sphere = dynamic_cast<const HyperSphereShape4D*>(p_shape)) {
		return slice_sphere(sphere, p_transform, p_hyperplane);
	}

	if (const HyperBoxShape4D *box = dynamic_cast<const HyperBoxShape4D*>(p_shape)) {
		return slice_box(box, p_transform, p_hyperplane);
	}

	if (const HyperCapsuleShape4D *capsule = dynamic_cast<const HyperCapsuleShape4D*>(p_shape)) {
		return slice_capsule(capsule, p_transform, p_hyperplane);
	}

	if (const HyperEllipsoidShape4D *ellipsoid = dynamic_cast<const HyperEllipsoidShape4D*>(p_shape)) {
		return slice_ellipsoid(ellipsoid, p_transform, p_hyperplane);
	}

	if (const ConvexHull4D *hull = dynamic_cast<const ConvexHull4D*>(p_shape)) {
		return slice_hull(hull, p_transform, p_hyperplane);
	}

	// Unknown shape type - return empty
	return SliceResult();
}

SliceResult Slicer4D::slice_sphere(
	const HyperSphereShape4D *p_sphere,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	return SphereSlicer::slice(p_sphere, p_transform, p_hyperplane);
}

SliceResult Slicer4D::slice_box(
	const HyperBoxShape4D *p_box,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	return PolytopeSlicer::slice(p_box, p_transform, p_hyperplane);
}

SliceResult Slicer4D::slice_capsule(
	const HyperCapsuleShape4D *p_capsule,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	// For now, use a simplified approach: approximate with vertices
	// A proper implementation would combine cylindrical and spherical sections
	// TODO: Implement proper capsule slicing

	// Return empty for now
	return SliceResult();
}

SliceResult Slicer4D::slice_ellipsoid(
	const HyperEllipsoidShape4D *p_ellipsoid,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	// For now, approximate by sampling vertices on the ellipsoid surface
	// A proper implementation would use analytic ellipsoid-plane intersection
	// TODO: Implement proper ellipsoid slicing

	// Return empty for now
	return SliceResult();
}

SliceResult Slicer4D::slice_hull(
	const ConvexHull4D *p_hull,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	return PolytopeSlicer::slice(p_hull, p_transform, p_hyperplane);
}
