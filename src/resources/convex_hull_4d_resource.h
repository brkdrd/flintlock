#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_vector4_array.hpp>

using namespace godot;

/// ConvexHull4DResource — A convex hull collision shape in 4D.
///
/// This is a Godot Resource that can be saved and reused.
/// Represents an arbitrary convex polytope defined by its vertices.
class ConvexHull4DResource : public Resource {
	GDCLASS(ConvexHull4DResource, Resource)

private:
	PackedVector4Array vertices;

public:
	ConvexHull4DResource();
	~ConvexHull4DResource();

	void set_vertices(const PackedVector4Array &p_vertices);
	PackedVector4Array get_vertices() const;

	// Individual vertex access
	void add_vertex(const Vector4 &p_vertex);
	void remove_vertex(int p_index);
	void clear_vertices();
	int get_vertex_count() const;

protected:
	static void _bind_methods();
};
