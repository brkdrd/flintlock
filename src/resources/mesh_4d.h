#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <godot_cpp/variant/packed_color_array.hpp>

using namespace godot;

// ============================================================
// Mesh4D - abstract base class for all 4D mesh data
// ============================================================
class Mesh4D : public Resource {
	GDCLASS(Mesh4D, Resource);
public:
	// Array indices (analog of Mesh::ARRAY_*)
	enum ArrayType {
		ARRAY_VERTEX = 0,  // PackedFloat32Array, 4 floats per vertex (x,y,z,w)
		ARRAY_NORMAL = 1,  // PackedFloat32Array, 4 floats per vertex
		ARRAY_TANGENT = 2, // PackedFloat32Array, tangent space in 4D
		ARRAY_TEX_UV = 3,  // PackedFloat32Array, 2 or 3 floats per vertex
		ARRAY_COLOR = 4,   // PackedColorArray
		ARRAY_INDEX = 5,   // PackedInt32Array, groups of 4 (tetrahedra)
		ARRAY_MAX = 6
	};

	// Surface data structure
	struct Surface {
		Array arrays; // ARRAY_MAX entries
		Ref<Material> material;
	};

	virtual int get_surface_count() const = 0;
	virtual Array get_surface_arrays(int p_surface) const = 0;
	virtual Ref<Material> get_surface_material(int p_surface) const = 0;

protected:
	static void _bind_methods();
};

VARIANT_ENUM_CAST(Mesh4D::ArrayType);

// ============================================================
// ArrayMesh4D - user-constructed mesh from arrays
// ============================================================
class ArrayMesh4D : public Mesh4D {
	GDCLASS(ArrayMesh4D, Mesh4D);

	struct SurfaceData {
		Array arrays;
		Ref<Material> material;
		String name;
	};

	Vector<SurfaceData> _surfaces;

protected:
	static void _bind_methods();

public:
	void add_surface(const Array &p_arrays);
	void clear_surfaces();

	int get_surface_count() const override;
	Array get_surface_arrays(int p_surface) const override;
	Ref<Material> get_surface_material(int p_surface) const override;
	void set_surface_material(int p_surface, const Ref<Material> &p_material);
	String get_surface_name(int p_surface) const;
	void set_surface_name(int p_surface, const String &p_name);
};

// ============================================================
// PrimitiveMesh4D - abstract base for procedural primitives
// ============================================================
class PrimitiveMesh4D : public Mesh4D {
	GDCLASS(PrimitiveMesh4D, Mesh4D);

	mutable Array _cached_arrays;
	mutable bool _dirty = true;
	Ref<Material> _material;

protected:
	static void _bind_methods();
	virtual Array _create_mesh_array() const = 0;
	void _request_update() { _dirty = true; emit_changed(); }

public:
	int get_surface_count() const override { return 1; }
	Array get_surface_arrays(int p_surface) const override;
	Ref<Material> get_surface_material(int p_surface) const override { return _material; }
	void set_surface_material(const Ref<Material> &p_material) { _material = p_material; }
	Ref<Material> get_material() const { return _material; }
};

// ============================================================
// HyperBoxMesh4D - tesseract with configurable size
// ============================================================
class HyperBoxMesh4D : public PrimitiveMesh4D {
	GDCLASS(HyperBoxMesh4D, PrimitiveMesh4D);
	Vector4 _size = Vector4(1, 1, 1, 1);

protected:
	static void _bind_methods();
	Array _create_mesh_array() const override;

public:
	Vector4 get_size() const { return _size; }
	void set_size(const Vector4 &p_size);
};

// ============================================================
// HyperSphereMesh4D - glome (3-sphere boundary)
// ============================================================
class HyperSphereMesh4D : public PrimitiveMesh4D {
	GDCLASS(HyperSphereMesh4D, PrimitiveMesh4D);
	real_t _radius = 0.5f;
	int _subdivisions = 2;

protected:
	static void _bind_methods();
	Array _create_mesh_array() const override;

public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
	int get_subdivisions() const { return _subdivisions; }
	void set_subdivisions(int p_subdivisions);
};

// ============================================================
// HyperCylinderMesh4D - sphere extruded along W
// ============================================================
class HyperCylinderMesh4D : public PrimitiveMesh4D {
	GDCLASS(HyperCylinderMesh4D, PrimitiveMesh4D);
	real_t _radius = 0.5f;
	real_t _height = 2.0f;
	int _radial_segments = 8;
	int _rings = 4;

protected:
	static void _bind_methods();
	Array _create_mesh_array() const override;

public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
	real_t get_height() const { return _height; }
	void set_height(real_t p_height);
	int get_radial_segments() const { return _radial_segments; }
	void set_radial_segments(int p_segments);
	int get_rings() const { return _rings; }
	void set_rings(int p_rings);
};

// ============================================================
// HyperCapsuleMesh4D - sphere extruded with hemispherical caps
// ============================================================
class HyperCapsuleMesh4D : public PrimitiveMesh4D {
	GDCLASS(HyperCapsuleMesh4D, PrimitiveMesh4D);
	real_t _radius = 0.5f;
	real_t _height = 2.0f;
	int _radial_segments = 8;
	int _rings = 4;

protected:
	static void _bind_methods();
	Array _create_mesh_array() const override;

public:
	real_t get_radius() const { return _radius; }
	void set_radius(real_t p_radius);
	real_t get_height() const { return _height; }
	void set_height(real_t p_height);
	int get_radial_segments() const { return _radial_segments; }
	void set_radial_segments(int p_segments);
	int get_rings() const { return _rings; }
	void set_rings(int p_rings);
};
