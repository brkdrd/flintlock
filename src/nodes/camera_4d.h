#pragma once

#include "node_4d.h"
#include "mesh_instance_4d.h"
#include "math/hyperplane4d.h"
#include "slicer/slicer4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/transform3d.hpp>

using namespace godot;

/// Camera4D — Drives the per-frame 4D→3D slicing and rendering loop.
///
/// Owns a RenderingServer camera RID. Derives the hyperplane from its own
/// Transform4D (the W-column of the basis is the hyperplane normal, the
/// origin is the point on the plane). Makes itself the active camera when
/// `current` is true.
///
/// Each _process frame:
///   1. Computes hyperplane from own Transform4D.
///   2. Iterates every node in group "visual_4d".
///   3. Slices each visible MeshInstance4D's shape.
///   4. Calls VisualInstance4D::update_render_data() with the result.
///   5. Updates the RS camera transform from the 3D projection of own position.
class Camera4D : public Node4D {
	GDCLASS(Camera4D, Node4D)

private:
	RID camera_rid;
	float fov = 75.0f;
	float near_plane = 0.05f;
	float far_plane = 4000.0f;
	bool current = false;

	// Derive the camera's 3D Transform from the 4D transform.
	Transform3D derive_camera_3d_transform() const;

public:
	Camera4D();
	~Camera4D();

	// -- Camera activation ---------------------------------------------------

	void make_current();
	void clear_current();
	bool is_current() const;

	// -- Hyperplane ----------------------------------------------------------

	Hyperplane4D get_hyperplane() const;

	// -- Camera RID (read-only) ----------------------------------------------

	RID get_camera_rid() const;

	// -- Properties: FOV, near, far ------------------------------------------

	void set_fov(float p_fov);
	float get_fov() const;

	void set_near(float p_near);
	float get_near() const;

	void set_far(float p_far);
	float get_far() const;

	void set_current(bool p_current);

	// -- Godot lifecycle -----------------------------------------------------

	void _process(double delta) override;
	void _notification(int p_what);

protected:
	static void _bind_methods();
};
