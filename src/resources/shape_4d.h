#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>

using namespace godot;

// ============================================================
// Shape4D - abstract base class for all 4D collision shapes
// ============================================================
class Shape4D : public Resource {
	GDCLASS(Shape4D, Resource);
protected:
	real_t _margin = 0.04f;
	static void _bind_methods();
public:
	real_t get_margin() const { return _margin; }
	void set_margin(real_t p_margin) { _margin = p_margin; }
};

// ============================================================
// BoxShape4D - axis-aligned hyperrectangle (tesseract)
// ============================================================
class BoxShape4D : public Shape4D {
	GDCLASS(BoxShape4D, Shape4D);
	Vector4 _size = Vector4(1, 1, 1, 1); // Full size (not half-extents)
protected:
	static void _bind_methods();
public:
	Vector4 get_size() const { return _size; }
	void set_size(const Vector4 &p_size);
};

// ============================================================
// SphereShape4D - 4D sphere (3-sphere boundary)
// ============================================================
class SphereShape4D : public Shape4D {
	GDCLASS(SphereShape4D, Shape4D);
	real_t _radius = 0.5f;
protected:
	static void _bind_methods();
public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
};

// ============================================================
// CapsuleShape4D - sphere extruded along one axis
// ============================================================
class CapsuleShape4D : public Shape4D {
	GDCLASS(CapsuleShape4D, Shape4D);
	real_t _radius = 0.5f;
	real_t _height = 2.0f;
protected:
	static void _bind_methods();
public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
	real_t get_height() const { return _height; }
	void set_height(real_t p_height);
};

// ============================================================
// CylinderShape4D - 3-sphere extruded along W axis
// ============================================================
class CylinderShape4D : public Shape4D {
	GDCLASS(CylinderShape4D, Shape4D);
	real_t _radius = 0.5f;
	real_t _height = 2.0f;
protected:
	static void _bind_methods();
public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
	real_t get_height() const { return _height; }
	void set_height(real_t p_height);
};

// ============================================================
// ConvexPolygonShape4D - convex polytope in 4D
// ============================================================
class ConvexPolygonShape4D : public Shape4D {
	GDCLASS(ConvexPolygonShape4D, Shape4D);
	PackedFloat32Array _points; // groups of 4 floats (x,y,z,w)
protected:
	static void _bind_methods();
public:
	PackedFloat32Array get_points() const { return _points; }
	void set_points(const PackedFloat32Array &p_points);
};

// ============================================================
// ConcavePolygonShape4D - tetrahedral mesh (for static bodies only)
// ============================================================
class ConcavePolygonShape4D : public Shape4D {
	GDCLASS(ConcavePolygonShape4D, Shape4D);
	PackedFloat32Array _faces; // groups of 16 floats (4 vertices x 4 components)
protected:
	static void _bind_methods();
public:
	PackedFloat32Array get_faces() const { return _faces; }
	void set_faces(const PackedFloat32Array &p_faces);
};

// ============================================================
// WorldBoundaryShape4D - infinite hyperplane
// ============================================================
class WorldBoundaryShape4D : public Shape4D {
	GDCLASS(WorldBoundaryShape4D, Shape4D);
	Vector4 _normal = Vector4(0, 1, 0, 0);
	real_t _distance = 0.0f;
protected:
	static void _bind_methods();
public:
	Vector4 get_normal() const { return _normal; }
	void set_normal(const Vector4 &p_normal);
	real_t get_distance() const { return _distance; }
	void set_distance(real_t p_distance);
};

// ============================================================
// SeparationRayShape4D - ray-based separation shape
// ============================================================
class SeparationRayShape4D : public Shape4D {
	GDCLASS(SeparationRayShape4D, Shape4D);
	real_t _length = 1.0f;
	bool _slide_on_slope = false;
protected:
	static void _bind_methods();
public:
	real_t get_length() const { return _length; }
	void set_length(real_t p_length);
	bool get_slide_on_slope() const { return _slide_on_slope; }
	void set_slide_on_slope(bool p_slide);
};
