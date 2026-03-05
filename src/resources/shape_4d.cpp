#include "shape_4d.h"
#include <godot_cpp/core/class_db.hpp>

// ---- Shape4D ----
void Shape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_margin"), &Shape4D::get_margin);
	ClassDB::bind_method(D_METHOD("set_margin", "margin"), &Shape4D::set_margin);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "margin", PROPERTY_HINT_RANGE, "0.001,256,0.001"), "set_margin", "get_margin");
}

// ---- BoxShape4D ----
void BoxShape4D::set_size(const Vector4 &p_size) {
	_size = p_size;
	emit_changed();
}
void BoxShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_size"), &BoxShape4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &BoxShape4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size"), "set_size", "get_size");
}

// ---- SphereShape4D ----
void SphereShape4D::set_radius(real_t p_radius) {
	_radius = p_radius;
	emit_changed();
}
void SphereShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &SphereShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &SphereShape4D::set_radius);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_radius", "get_radius");
}

// ---- CapsuleShape4D ----
void CapsuleShape4D::set_radius(real_t p_radius) { _radius = p_radius; emit_changed(); }
void CapsuleShape4D::set_height(real_t p_height) { _height = p_height; emit_changed(); }
void CapsuleShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &CapsuleShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CapsuleShape4D::set_radius);
	ClassDB::bind_method(D_METHOD("get_height"), &CapsuleShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CapsuleShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_height", "get_height");
}

// ---- CylinderShape4D ----
void CylinderShape4D::set_radius(real_t p_radius) { _radius = p_radius; emit_changed(); }
void CylinderShape4D::set_height(real_t p_height) { _height = p_height; emit_changed(); }
void CylinderShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &CylinderShape4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &CylinderShape4D::set_radius);
	ClassDB::bind_method(D_METHOD("get_height"), &CylinderShape4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &CylinderShape4D::set_height);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_height", "get_height");
}

// ---- ConvexPolygonShape4D ----
void ConvexPolygonShape4D::set_points(const PackedFloat32Array &p_points) {
	_points = p_points;
	emit_changed();
}
void ConvexPolygonShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_points"), &ConvexPolygonShape4D::get_points);
	ClassDB::bind_method(D_METHOD("set_points", "points"), &ConvexPolygonShape4D::set_points);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "points"), "set_points", "get_points");
}

// ---- ConcavePolygonShape4D ----
void ConcavePolygonShape4D::set_faces(const PackedFloat32Array &p_faces) {
	_faces = p_faces;
	emit_changed();
}
void ConcavePolygonShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_faces"), &ConcavePolygonShape4D::get_faces);
	ClassDB::bind_method(D_METHOD("set_faces", "faces"), &ConcavePolygonShape4D::set_faces);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT32_ARRAY, "faces"), "set_faces", "get_faces");
}

// ---- WorldBoundaryShape4D ----
void WorldBoundaryShape4D::set_normal(const Vector4 &p_normal) { _normal = p_normal; emit_changed(); }
void WorldBoundaryShape4D::set_distance(real_t p_distance) { _distance = p_distance; emit_changed(); }
void WorldBoundaryShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_normal"), &WorldBoundaryShape4D::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "normal"), &WorldBoundaryShape4D::set_normal);
	ClassDB::bind_method(D_METHOD("get_distance"), &WorldBoundaryShape4D::get_distance);
	ClassDB::bind_method(D_METHOD("set_distance", "distance"), &WorldBoundaryShape4D::set_distance);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "normal"), "set_normal", "get_normal");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "distance"), "set_distance", "get_distance");
}

// ---- SeparationRayShape4D ----
void SeparationRayShape4D::set_length(real_t p_length) { _length = p_length; emit_changed(); }
void SeparationRayShape4D::set_slide_on_slope(bool p_slide) { _slide_on_slope = p_slide; emit_changed(); }
void SeparationRayShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_length"), &SeparationRayShape4D::get_length);
	ClassDB::bind_method(D_METHOD("set_length", "length"), &SeparationRayShape4D::set_length);
	ClassDB::bind_method(D_METHOD("get_slide_on_slope"), &SeparationRayShape4D::get_slide_on_slope);
	ClassDB::bind_method(D_METHOD("set_slide_on_slope", "slide"), &SeparationRayShape4D::set_slide_on_slope);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "length", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_length", "get_length");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "slide_on_slope"), "set_slide_on_slope", "get_slide_on_slope");
}
