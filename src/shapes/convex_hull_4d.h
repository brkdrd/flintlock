#pragma once

#include "shape4d.h"
#include <vector>

/// ConvexHull4D — A convex polytope defined by its vertices.
///
/// Represents an arbitrary convex shape in 4D space by storing a set of vertices.
/// The actual convex hull is the minimal convex set containing all vertices.
///
/// Support function uses brute-force search over all vertices.
/// For large hulls, this can be optimized with spatial partitioning or hill-climbing.
class ConvexHull4D : public Shape4D {
public:
	// -- Constructor ---------------------------------------------------------

	ConvexHull4D();
	explicit ConvexHull4D(const std::vector<Vector4> &p_vertices);

	// -- Accessors -----------------------------------------------------------

	void set_vertices(const std::vector<Vector4> &p_vertices);
	const std::vector<Vector4> &get_vertices() const;

	int get_vertex_count() const;
	Vector4 get_vertex(int p_index) const;

	// -- Shape4D interface ---------------------------------------------------

	virtual Vector4 get_support(const Vector4 &p_direction) const override;
	virtual AABB4D get_aabb(const Transform4D &p_transform) const override;
	virtual bool contains_point(const Vector4 &p_point) const override;

private:
	std::vector<Vector4> vertices;

	// Helper for containment testing using support function
	bool is_point_inside_convex_hull(const Vector4 &p_point) const;
};
