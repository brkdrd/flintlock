#pragma once
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;

class VisualInstance4D;

// ============================================================
// Slicer4D Singleton — GPU vertex-shader based 4D→3D slicing
//
// Architecture (inspired by HackerPoet/Engine4D):
//   - Each tetrahedron's 4 vertices are packed into GPU vertex
//     attributes (VERTEX + CUSTOM0-3) and uploaded ONCE.
//   - A custom spatial shader performs per-vertex slicing:
//     1. Transform 4D verts by per-instance model matrix
//     2. Compute signed distance to the slice hyperplane
//     3. LUT lookup to find which edge to interpolate
//     4. Lerp along edge to find the w=0 intersection
//     5. Project 4D intersection to 3D via camera basis
//   - Per frame, only uniform updates happen (no CPU mesh work).
// ============================================================
class Slicer4D : public Object {
	GDCLASS(Slicer4D, Object);

	static Slicer4D *singleton;

	// Registered visual instances
	Vector<VisualInstance4D *> _instances;

	// GPU resources
	RID _shader_rid;
	RID _material_rid;
	Ref<ImageTexture> _lut_texture;
	bool _initialized = false;

	void _initialize();
	void _generate_lut();
	String _get_shader_code() const;

protected:
	static void _bind_methods();

public:
	static Slicer4D *get_singleton();

	Slicer4D();
	~Slicer4D();

	void register_instance(VisualInstance4D *p_instance);
	void unregister_instance(VisualInstance4D *p_instance);

	// Called by Camera4D every frame. Updates shader uniforms only.
	void update_frame(const Vector4 &p_plane_normal, float p_plane_d,
		const PackedFloat32Array &p_basis_cols,
		const Vector4 &p_camera_origin);

	RID get_shader_rid();
	RID get_material_rid();
	RID get_lut_rid();
	bool is_initialized() const { return _initialized; }
	void ensure_initialized();

	int get_instance_count() const { return _instances.size(); }
};
