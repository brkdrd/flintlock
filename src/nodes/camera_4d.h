#pragma once
#include "node_4d.h"
#include "../slicer/slicer_4d.h"
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/camera_attributes.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

using namespace godot;

// Camera4D - dual role:
//   1. 4D: Its Transform4D defines the slice hyperplane. W column = slice normal.
//   2. 3D: Owns an internal Camera3D that renders the sliced scene.
//
// Each frame, calls Slicer4D::update_frame() with the current hyperplane.
class Camera4D : public Node4D {
	GDCLASS(Camera4D, Node4D);

public:
	enum ProjectionType {
		PROJECTION_PERSPECTIVE = 0,
		PROJECTION_ORTHOGONAL = 1,
		PROJECTION_FRUSTUM = 2,
	};

	enum KeepAspect {
		KEEP_WIDTH = 0,
		KEEP_HEIGHT = 1,
	};

private:
	Camera3D *_internal_camera = nullptr;
	bool _current = false;
	real_t _fov = 75.0f;
	real_t _size = 1.0f;
	real_t _near = 0.05f;
	real_t _far = 4000.0f;
	ProjectionType _projection = PROJECTION_PERSPECTIVE;
	KeepAspect _keep_aspect = KEEP_HEIGHT;
	uint32_t _cull_mask = 0xFFFFF;
	real_t _h_offset = 0.0f;
	real_t _v_offset = 0.0f;
	real_t _slice_offset = 0.0f;
	Ref<Environment> _environment;
	Ref<CameraAttributes> _attributes;

	// Cached slice state
	Vector4 _cached_plane_normal;
	float _cached_plane_d = 0.0f;

	void _update_camera3d_properties();
	void _perform_slice();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	Camera4D();

	// Camera management
	void make_current();
	void clear_current(bool p_enable_next = true);
	bool is_current() const { return _current; }

	// 3D camera properties (forwarded to internal Camera3D)
	real_t get_fov() const { return _fov; }
	void set_fov(real_t p_fov);

	real_t get_size() const { return _size; }
	void set_size(real_t p_size);

	real_t get_near() const { return _near; }
	void set_near(real_t p_near);

	real_t get_far() const { return _far; }
	void set_far(real_t p_far);

	ProjectionType get_projection() const { return _projection; }
	void set_projection(ProjectionType p_projection);

	KeepAspect get_keep_aspect() const { return _keep_aspect; }
	void set_keep_aspect(KeepAspect p_keep);

	uint32_t get_cull_mask() const { return _cull_mask; }
	void set_cull_mask(uint32_t p_mask);

	real_t get_h_offset() const { return _h_offset; }
	void set_h_offset(real_t p_offset);

	real_t get_v_offset() const { return _v_offset; }
	void set_v_offset(real_t p_offset);

	real_t get_slice_offset() const { return _slice_offset; }
	void set_slice_offset(real_t p_offset);

	Ref<Environment> get_environment() const { return _environment; }
	void set_environment(const Ref<Environment> &p_env);

	Ref<CameraAttributes> get_attributes() const { return _attributes; }
	void set_attributes(const Ref<CameraAttributes> &p_attrs);

	// 4D slice plane access
	Vector4 get_slice_plane_normal() const;
	float get_slice_plane_distance() const;
};

VARIANT_ENUM_CAST(Camera4D::ProjectionType);
VARIANT_ENUM_CAST(Camera4D::KeepAspect);
