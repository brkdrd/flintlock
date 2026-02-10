#pragma once

#include "math/math_defs_4d.h"
#include <vector>

/// SliceResult — Container for 3D cross-section geometry.
///
/// Stores the triangulated mesh resulting from slicing a 4D shape
/// with a 3D hyperplane. Vertices are in the hyperplane's local 3D
/// coordinate system.
class SliceResult {
public:
	SliceResult();

	// -- Geometry data -------------------------------------------------------

	/// Add a vertex to the slice
	void add_vertex(const Vector3 &p_vertex);

	/// Add a normal vector
	void add_normal(const Vector3 &p_normal);

	/// Add a triangle (three vertex indices)
	void add_triangle(int p_i0, int p_i1, int p_i2);

	/// Get vertex count
	int vertex_count() const;

	/// Get vertex by index
	Vector3 get_vertex(int p_index) const;

	/// Get normal by index
	Vector3 get_normal(int p_index) const;

	/// Get triangle index count
	int index_count() const;

	/// Get index by position
	int get_index(int p_position) const;

	// -- Query methods -------------------------------------------------------

	/// Returns true if the slice is empty (no intersection)
	bool is_empty() const;

	/// Get approximate bounding radius (for sphere slices)
	real_t get_bounding_radius() const;

	/// Set bounding radius (calculated by slicer)
	void set_bounding_radius(real_t p_radius);

	// -- Direct access (for efficiency) --------------------------------------

	const std::vector<Vector3> &get_vertices() const { return vertices; }
	const std::vector<Vector3> &get_normals() const { return normals; }
	const std::vector<int> &get_indices() const { return indices; }

	// -- Public members for test compatibility -------------------------------

	real_t bounding_radius; // Approximate bounding radius

private:
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<int> indices;
};
