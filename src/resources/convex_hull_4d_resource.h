#pragma once

#include "shape_4d_resource_base.h"
#include "shapes/convex_hull_4d.h"
#include <godot_cpp/variant/packed_vector4_array.hpp>

using namespace godot;

/// ConvexHull4DResource — A convex hull collision shape in 4D.
///
/// This is a Godot Resource that can be saved and reused.
/// Represents an arbitrary convex polytope defined by its vertices.
class ConvexHull4DResource : public Shape4DResourceBase {
	GDCLASS(ConvexHull4DResource, Shape4DResourceBase)

private:
	PackedVector4Array vertices;
	mutable ConvexHull4D shape_data;

public:
	ConvexHull4DResource();
	~ConvexHull4DResource();

	void set_vertices(const PackedVector4Array &p_vertices);
	PackedVector4Array get_vertices() const;

	void add_vertex(const Vector4 &p_vertex);
	void remove_vertex(int p_index);
	void clear_vertices();
	int get_vertex_count() const;

	Shape4D *get_shape() const override;

protected:
	static void _bind_methods();
};
