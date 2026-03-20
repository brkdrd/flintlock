#pragma once
#include "../node_4d.h"
#include "../../resources/mesh_4d.h"
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

using namespace godot;

class VisualServer4D;

// VisualInstance4D - base for all renderable 4D objects.
// Communicates with VisualServer4D via RIDs. No direct RS calls.
class VisualInstance4D : public Node4D {
	GDCLASS(VisualInstance4D, Node4D);

protected:
	RID _vs_instance;  // VisualServer4D instance RID
	RID _vs_mesh;      // VisualServer4D mesh RID

	uint32_t _layers = 1;
	real_t _sorting_offset = 0.0f;

	static void _bind_methods();
	void _notification(int p_what);

public:
	virtual Ref<Mesh4D> get_mesh_4d() const { return Ref<Mesh4D>(); }
	virtual Ref<Material> get_active_material_3d() const { return Ref<Material>(); }

	void _on_transform_4d_changed() override;

	// Upload mesh data to VS4D
	void upload_mesh();

	// Push 4D transform to VS4D
	void update_transform();

	// Apply material properties to VS4D
	virtual void apply_material_params();

	RID get_instance_rid() const { return _vs_instance; }
	RID get_mesh_rid() const { return _vs_mesh; }

	uint32_t get_layers() const { return _layers; }
	void set_layers(uint32_t p_layers);

	real_t get_sorting_offset() const { return _sorting_offset; }
	void set_sorting_offset(real_t p_offset);
};
