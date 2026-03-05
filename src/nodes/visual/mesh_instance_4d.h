#pragma once
#include "geometry_instance_4d.h"
#include "../../resources/mesh_4d.h"
#include "../../resources/material_4d.h"
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

class MeshInstance4D : public GeometryInstance4D {
	GDCLASS(MeshInstance4D, GeometryInstance4D);

	Ref<Mesh4D> _mesh;
	TypedArray<Material4D> _surface_materials;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Ref<Mesh4D> get_mesh() const { return _mesh; }
	void set_mesh(const Ref<Mesh4D> &p_mesh);

	Ref<Material4D> get_surface_material(int p_surface) const;
	void set_surface_material(int p_surface, const Ref<Material4D> &p_material);
	int get_surface_material_count() const;

	// Override from VisualInstance4D - provides mesh data to Slicer4D
	Ref<Mesh4D> get_mesh_4d() const override { return _mesh; }
};
