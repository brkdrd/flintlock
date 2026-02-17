#pragma once

#include "node_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

/// VisualInstance4D — Base class for all visible 4D objects.
///
/// Owns a RenderingServer instance RID and mesh RID. Does NOT create child
/// MeshInstance3D nodes — everything goes through RenderingServer directly,
/// mirroring how Godot's own VisualInstance3D works.
///
/// Adds self to SceneTree group "visual_4d" on enter, removes on exit.
/// Camera4D iterates that group every frame to slice and render.
class VisualInstance4D : public Node4D {
	GDCLASS(VisualInstance4D, Node4D)

protected:
	RID instance_rid;
	RID mesh_rid;
	bool visible = true;
	uint32_t layers = 1;

public:
	VisualInstance4D();
	virtual ~VisualInstance4D();

	// -- Visibility ----------------------------------------------------------

	void set_visible(bool p_visible);
	bool is_visible() const;

	// -- Render layers -------------------------------------------------------

	void set_layers(uint32_t p_layers);
	uint32_t get_layers() const;

	// -- RenderingServer RIDs (read-only, managed internally) ----------------

	RID get_instance_rid() const;
	RID get_mesh_rid() const;

	// -- Called by Camera4D each frame to push new slice geometry -----------

	void update_render_data(const Array &p_surface_arrays, const Transform3D &p_slice_transform);

	// -- Godot lifecycle -----------------------------------------------------

	void _notification(int p_what);

protected:
	static void _bind_methods();
};
