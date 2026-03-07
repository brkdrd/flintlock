#pragma once
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/rendering_device.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_set.hpp>

using namespace godot;

// Forward declarations
class VisualInstance4D;

// ============================================================
// Slicer4D Singleton
//
// Central GPU compute pipeline for slicing 4D geometry into 3D.
// Every VisualInstance4D registers with this singleton.
// Each frame, Camera4D calls slice_all() with the current hyperplane.
// The slicer produces 3D triangle data and updates each instance's
// RenderingServer mesh RID.
// ============================================================
class Slicer4D : public Object {
	GDCLASS(Slicer4D, Object);

	static Slicer4D *singleton;

	// Registered visual instances
	Vector<VisualInstance4D *> _instances;
	HashSet<VisualInstance4D *> _dirty_set;

	// GPU compute resources
	RID _compute_shader;
	RID _compute_pipeline;
	bool _pipeline_initialized = false;

	// Cached slice state to detect changes
	Vector4 _cached_plane_normal = Vector4(0, 0, 0, 1);
	float _cached_plane_d = 0.0f;

	void _initialize_pipeline();
	void _slice_instance_cpu(VisualInstance4D *p_instance,
		const Vector4 &p_plane_normal, float p_plane_d,
		const PackedFloat32Array &p_basis_cols,
		const Vector4 &p_camera_origin);

protected:
	static void _bind_methods();

public:
	static Slicer4D *get_singleton();

	Slicer4D();
	~Slicer4D();

	void register_instance(VisualInstance4D *p_instance);
	void unregister_instance(VisualInstance4D *p_instance);
	void mark_dirty(VisualInstance4D *p_instance);
	void mark_all_dirty();

	// Called by Camera4D every frame to slice all dirty instances.
	// p_basis_cols: 12 floats = 3 Vector4s (columns 0,1,2 of camera basis)
	// p_plane_normal: W column of camera basis (hyperplane normal)
	// p_plane_d: dot(p_plane_normal, camera_origin)
	// p_camera_origin: 4D camera position
	void slice_all(const Vector4 &p_plane_normal, float p_plane_d,
		const PackedFloat32Array &p_basis_cols,
		const Vector4 &p_camera_origin);

	int get_instance_count() const { return _instances.size(); }
};
