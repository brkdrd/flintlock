#include "convex_hull_4d_resource.h"
#include <godot_cpp/core/class_db.hpp>

ConvexHull4DResource::ConvexHull4DResource() {
}

ConvexHull4DResource::~ConvexHull4DResource() {
}

void ConvexHull4DResource::set_vertices(const PackedVector4Array &p_vertices) {
	vertices = p_vertices;
	emit_changed();  // Notify that the resource has changed
}

PackedVector4Array ConvexHull4DResource::get_vertices() const {
	return vertices;
}

void ConvexHull4DResource::add_vertex(const Vector4 &p_vertex) {
	vertices.push_back(p_vertex);
	emit_changed();
}

void ConvexHull4DResource::remove_vertex(int p_index) {
	if (p_index >= 0 && p_index < vertices.size()) {
		vertices.remove_at(p_index);
		emit_changed();
	}
}

void ConvexHull4DResource::clear_vertices() {
	vertices.clear();
	emit_changed();
}

int ConvexHull4DResource::get_vertex_count() const {
	return vertices.size();
}

void ConvexHull4DResource::_bind_methods() {
	// Vertices array
	ClassDB::bind_method(D_METHOD("set_vertices", "vertices"), &ConvexHull4DResource::set_vertices);
	ClassDB::bind_method(D_METHOD("get_vertices"), &ConvexHull4DResource::get_vertices);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR4_ARRAY, "vertices"), "set_vertices", "get_vertices");

	// Individual vertex operations
	ClassDB::bind_method(D_METHOD("add_vertex", "vertex"), &ConvexHull4DResource::add_vertex);
	ClassDB::bind_method(D_METHOD("remove_vertex", "index"), &ConvexHull4DResource::remove_vertex);
	ClassDB::bind_method(D_METHOD("clear_vertices"), &ConvexHull4DResource::clear_vertices);
	ClassDB::bind_method(D_METHOD("get_vertex_count"), &ConvexHull4DResource::get_vertex_count);
}
