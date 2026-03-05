#pragma once
#include "../node_4d.h"
#include "../../resources/mesh_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>

using namespace godot;

// VisualInstance4D - base for all renderable 4D objects.
// Owns RenderingServer RIDs for the 3D instance and mesh.
// On enter tree: creates RIDs, registers with Slicer4D.
// On exit tree: unregisters from Slicer4D, frees RIDs.
// On transform change: notifies Slicer4D to re-slice.
class VisualInstance4D : public Node4D {
	GDCLASS(VisualInstance4D, Node4D);

protected:
	RID _rs_instance; // RenderingServer instance RID
	RID _rs_mesh;     // RenderingServer mesh RID

	uint32_t _layers = 1;
	real_t _sorting_offset = 0.0f;

	static void _bind_methods();
	void _notification(int p_what);

public:
	// Called by Slicer4D to get the 4D mesh data
	virtual Ref<Mesh4D> get_mesh_4d() const { return Ref<Mesh4D>(); }

	// Called by Slicer4D to update the rendered 3D geometry
	void update_rendering_mesh(
		const PackedFloat32Array &p_vertices,
		const PackedFloat32Array &p_normals,
		const PackedFloat32Array &p_uvs,
		const PackedInt32Array &p_indices);

	void clear_rendering_mesh();

	RID get_instance_rid() const { return _rs_instance; }
	RID get_mesh_rid() const { return _rs_mesh; }

	uint32_t get_layers() const { return _layers; }
	void set_layers(uint32_t p_layers);

	real_t get_sorting_offset() const { return _sorting_offset; }
	void set_sorting_offset(real_t p_offset);
};
