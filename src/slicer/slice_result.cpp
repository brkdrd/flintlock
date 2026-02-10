#include "slice_result.h"

SliceResult::SliceResult() :
		bounding_radius(0.0) {
}

// -- Geometry data -----------------------------------------------------------

void SliceResult::add_vertex(const Vector3 &p_vertex) {
	vertices.push_back(p_vertex);
}

void SliceResult::add_normal(const Vector3 &p_normal) {
	normals.push_back(p_normal);
}

void SliceResult::add_triangle(int p_i0, int p_i1, int p_i2) {
	indices.push_back(p_i0);
	indices.push_back(p_i1);
	indices.push_back(p_i2);
}

int SliceResult::vertex_count() const {
	return vertices.size();
}

Vector3 SliceResult::get_vertex(int p_index) const {
	if (p_index >= 0 && p_index < (int)vertices.size()) {
		return vertices[p_index];
	}
	return Vector3(0, 0, 0);
}

Vector3 SliceResult::get_normal(int p_index) const {
	if (p_index >= 0 && p_index < (int)normals.size()) {
		return normals[p_index];
	}
	return Vector3(0, 0, 0);
}

int SliceResult::index_count() const {
	return indices.size();
}

int SliceResult::get_index(int p_position) const {
	if (p_position >= 0 && p_position < (int)indices.size()) {
		return indices[p_position];
	}
	return 0;
}

// -- Query methods -----------------------------------------------------------

bool SliceResult::is_empty() const {
	return vertices.empty();
}

real_t SliceResult::get_bounding_radius() const {
	return bounding_radius;
}

void SliceResult::set_bounding_radius(real_t p_radius) {
	bounding_radius = p_radius;
}
