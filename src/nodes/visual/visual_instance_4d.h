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
// On enter tree: creates RIDs, registers with Slicer4D, uploads GPU mesh.
// On exit tree: unregisters from Slicer4D, frees RIDs.
// GPU mesh is uploaded once; per-frame slicing happens in the vertex shader.
class VisualInstance4D : public Node4D {
	GDCLASS(VisualInstance4D, Node4D);

protected:
	RID _rs_instance;  // RenderingServer instance RID
	RID _rs_mesh;      // RenderingServer mesh RID
	RID _rs_material;  // Per-instance material RID (owns its own uniforms)

	uint32_t _layers = 1;
	real_t _sorting_offset = 0.0f;
	bool _gpu_mesh_uploaded = false;

	static void _bind_methods();
	void _notification(int p_what);

public:
	// Called to get the 4D mesh data for GPU upload
	virtual Ref<Mesh4D> get_mesh_4d() const { return Ref<Mesh4D>(); }

	// Returns the Material4D (if any) for applying per-instance material params
	virtual Ref<Material> get_active_material_3d() const { return Ref<Material>(); }

	// No-op for GPU slicer — transforms are handled via shader uniforms
	void _on_transform_4d_changed() override;

	// Packs Mesh4D tetrahedra into GPU vertex format and uploads to _rs_mesh
	void upload_gpu_mesh();

	// Sets per-instance 4D model matrix on the RenderingServer instance
	void update_shader_transforms();

	// Apply material properties as per-instance shader parameters
	virtual void apply_material_params();

	RID get_instance_rid() const { return _rs_instance; }
	RID get_mesh_rid() const { return _rs_mesh; }
	RID get_material_rid() const { return _rs_material; }

	uint32_t get_layers() const { return _layers; }
	void set_layers(uint32_t p_layers);

	real_t get_sorting_offset() const { return _sorting_offset; }
	void set_sorting_offset(real_t p_offset);
};
