#include "convex_hull_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>
#include <vector>

ConvexHull4DResource::ConvexHull4DResource() {}
ConvexHull4DResource::~ConvexHull4DResource() {}

void ConvexHull4DResource::set_vertices(const PackedVector4Array &p_vertices) {
	vertices = p_vertices;
	// Rebuild the shape's vertex list.
	std::vector<Vector4> verts;
	verts.reserve(vertices.size());
	for (int i = 0; i < vertices.size(); i++) {
		verts.push_back(vertices[i]);
	}
	shape_data.set_vertices(verts);
	emit_changed();
}

PackedVector4Array ConvexHull4DResource::get_vertices() const {
	return vertices;
}

void ConvexHull4DResource::add_vertex(const Vector4 &p_vertex) {
	vertices.push_back(p_vertex);
	// Rebuild shape with the new vertex included.
	std::vector<Vector4> verts;
	verts.reserve(vertices.size());
	for (int i = 0; i < vertices.size(); i++) {
		verts.push_back(vertices[i]);
	}
	shape_data.set_vertices(verts);
	emit_changed();
}

void ConvexHull4DResource::remove_vertex(int p_index) {
	if (p_index >= 0 && p_index < vertices.size()) {
		vertices.remove_at(p_index);
		// Rebuild shape after removal.
		std::vector<Vector4> verts;
		verts.reserve(vertices.size());
		for (int i = 0; i < vertices.size(); i++) {
			verts.push_back(vertices[i]);
		}
		shape_data.set_vertices(verts);
		emit_changed();
	}
}

void ConvexHull4DResource::clear_vertices() {
	vertices.clear();
	shape_data = ConvexHull4D();
	emit_changed();
}

int ConvexHull4DResource::get_vertex_count() const {
	return vertices.size();
}

Shape4D *ConvexHull4DResource::get_shape() const {
	return &shape_data;
}

void ConvexHull4DResource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ConvexHull4DResource::set_vertices);
	ClassDB::bind_method(D_METHOD("get_vertices"), &ConvexHull4DResource::get_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");

	ClassDB::bind_method(D_METHOD("add_vertex", "vertex"), &ConvexHull4DResource::add_vertex);
	ClassDB::bind_method(D_METHOD("remove_vertex", "index"), &ConvexHull4DResource::remove_vertex);
	ClassDB::bind_method(D_METHOD("clear_vertices"), &ConvexHull4DResource::clear_vertices);
	ClassDB::bind_method(D_METHOD("get_vertex_count"), &ConvexHull4DResource::get_vertex_count);
}
