#include "sphere_slicer.h"
#include <cmath>

SliceResult SphereSlicer::slice(
	const HyperSphereShape4D *p_sphere,
	const Transform4D &p_transform,
	const Hyperplane4D &p_hyperplane
) {
	SliceResult result;

	if (!p_sphere) {
		return result;
	}

	// Get the 4D center of the sphere in world space
	Vector4 center_4d = p_transform.xform(Vector4(0, 0, 0, 0));

	// Compute signed distance from sphere center to hyperplane
	real_t distance = p_hyperplane.distance_to(center_4d);

	real_t radius = p_sphere->get_radius();

	// Check if hyperplane intersects the sphere
	// If |distance| > radius, no intersection
	if (std::abs(distance) > radius + CMP_EPSILON) {
		return result; // Empty
	}

	// Calculate the radius of the 3D cross-section sphere
	// Using Pythagorean theorem: r_3d² + d² = r_4d²
	real_t distance_sq = distance * distance;
	real_t radius_sq = radius * radius;
	real_t slice_radius_sq = radius_sq - distance_sq;

	if (slice_radius_sq < 0.0) {
		slice_radius_sq = 0.0; // Tangent case
	}

	real_t slice_radius = std::sqrt(slice_radius_sq);

	// Set the bounding radius for tests
	result.set_bounding_radius(slice_radius);

	// Generate a 3D sphere mesh
	// For now, we generate a simple UV sphere
	if (slice_radius > CMP_EPSILON) {
		generate_sphere_mesh(result, slice_radius, 16);
	}

	return result;
}

void SphereSlicer::generate_sphere_mesh(
	SliceResult &r_result,
	real_t p_radius,
	int p_segments
) {
	// Generate a UV sphere with p_segments latitude and longitude divisions
	// This creates a simple tessellated 3D sphere

	int latitude_segments = p_segments;
	int longitude_segments = p_segments;

	// Generate vertices
	for (int lat = 0; lat <= latitude_segments; lat++) {
		real_t theta = lat * Math_PI / latitude_segments; // 0 to PI
		real_t sin_theta = std::sin(theta);
		real_t cos_theta = std::cos(theta);

		for (int lon = 0; lon <= longitude_segments; lon++) {
			real_t phi = lon * 2.0 * Math_PI / longitude_segments; // 0 to 2PI
			real_t sin_phi = std::sin(phi);
			real_t cos_phi = std::cos(phi);

			// Spherical coordinates to Cartesian
			real_t x = p_radius * sin_theta * cos_phi;
			real_t y = p_radius * sin_theta * sin_phi;
			real_t z = p_radius * cos_theta;

			r_result.add_vertex(Vector3(x, y, z));

			// Normal is the normalized position for a unit sphere
			Vector3 normal = Vector3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
			r_result.add_normal(normal);
		}
	}

	// Generate triangles
	for (int lat = 0; lat < latitude_segments; lat++) {
		for (int lon = 0; lon < longitude_segments; lon++) {
			int first = lat * (longitude_segments + 1) + lon;
			int second = first + longitude_segments + 1;

			// Two triangles per quad
			r_result.add_triangle(first, second, first + 1);
			r_result.add_triangle(second, second + 1, first + 1);
		}
	}
}
