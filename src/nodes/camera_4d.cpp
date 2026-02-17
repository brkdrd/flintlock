#include "camera_4d.h"

#include "visual_instance_4d.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/basis.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/vector3.hpp>

using namespace godot;

Camera4D::Camera4D() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		camera_rid = rs->camera_create();
		rs->camera_set_perspective(camera_rid, fov, near_plane, far_plane);
	}
}

Camera4D::~Camera4D() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && camera_rid.is_valid()) {
		rs->free_rid(camera_rid);
	}
}

// -- Hyperplane derivation ---------------------------------------------------

Hyperplane4D Camera4D::get_hyperplane() const {
	// The W-column of the 4D basis is the hyperplane normal.
	// The 4D origin is a point on the hyperplane.
	Vector4 normal = transform_4d.basis.get_column(3).normalized();
	return Hyperplane4D(normal, transform_4d.origin);
}

// -- 3D camera transform derivation ------------------------------------------

Transform3D Camera4D::derive_camera_3d_transform() const {
	// XYZ components of the 4D origin become the 3D camera position.
	Vector3 origin_3d(transform_4d.origin.x, transform_4d.origin.y, transform_4d.origin.z);

	// Upper-left 3×3 of the 4D basis becomes the 3D camera orientation.
	Vector3 col0(transform_4d.basis.rows[0].x, transform_4d.basis.rows[1].x, transform_4d.basis.rows[2].x);
	Vector3 col1(transform_4d.basis.rows[0].y, transform_4d.basis.rows[1].y, transform_4d.basis.rows[2].y);
	Vector3 col2(transform_4d.basis.rows[0].z, transform_4d.basis.rows[1].z, transform_4d.basis.rows[2].z);
	Basis basis_3d(col0, col1, col2);

	return Transform3D(basis_3d, origin_3d);
}

// -- Camera activation -------------------------------------------------------

void Camera4D::make_current() {
	current = true;
	Viewport *vp = get_viewport();
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && vp && camera_rid.is_valid()) {
		rs->viewport_attach_camera(vp->get_viewport_rid(), camera_rid);
	}
}

void Camera4D::clear_current() {
	current = false;
	Viewport *vp = get_viewport();
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && vp) {
		rs->viewport_attach_camera(vp->get_viewport_rid(), RID());
	}
}

bool Camera4D::is_current() const {
	return current;
}

RID Camera4D::get_camera_rid() const {
	return camera_rid;
}

// -- Properties: FOV, near, far ----------------------------------------------

void Camera4D::set_fov(float p_fov) {
	fov = p_fov;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && camera_rid.is_valid()) {
		rs->camera_set_perspective(camera_rid, fov, near_plane, far_plane);
	}
}

float Camera4D::get_fov() const {
	return fov;
}

void Camera4D::set_near(float p_near) {
	near_plane = p_near;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && camera_rid.is_valid()) {
		rs->camera_set_perspective(camera_rid, fov, near_plane, far_plane);
	}
}

float Camera4D::get_near() const {
	return near_plane;
}

void Camera4D::set_far(float p_far) {
	far_plane = p_far;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && camera_rid.is_valid()) {
		rs->camera_set_perspective(camera_rid, fov, near_plane, far_plane);
	}
}

float Camera4D::get_far() const {
	return far_plane;
}

void Camera4D::set_current(bool p_current) {
	if (p_current) {
		make_current();
	} else {
		clear_current();
	}
}

// -- Godot lifecycle ---------------------------------------------------------

void Camera4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			add_to_group("cameras_4d");
			if (current) {
				make_current();
			}
			set_process(true);
			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			clear_current();
			remove_from_group("cameras_4d");
			break;
		}
		default:
			break;
	}
}

void Camera4D::_process(double delta) {
	if (Engine::get_singleton()->is_editor_hint()) {
		return; // Don't slice in editor.
	}

	RenderingServer *rs = RenderingServer::get_singleton();
	if (!rs || !camera_rid.is_valid()) {
		return;
	}

	// 1. Compute hyperplane and 3D camera transform.
	Hyperplane4D plane = get_hyperplane();
	Transform3D cam_transform = derive_camera_3d_transform();
	rs->camera_set_transform(camera_rid, cam_transform);

	// 2. Compute the 3D coordinate frame for the hyperplane slice.
	//    Tangent vectors t1,t2,t3 map the slice to 3D world axes.
	Vector4 t1, t2, t3;
	plane.get_tangent_basis(&t1, &t2, &t3);

	// Build the slice→world transform: 3D columns from the XYZ of each tangent.
	Basis basis_3d(
			Vector3(t1.x, t1.y, t1.z),
			Vector3(t2.x, t2.y, t2.z),
			Vector3(t3.x, t3.y, t3.z));
	Transform3D slice_transform(basis_3d, Vector3(0, 0, 0));

	// 3. Iterate all visible MeshInstance4D nodes and slice/render them.
	SceneTree *tree = get_tree();
	if (!tree) {
		return;
	}

	TypedArray<Node> visual_nodes = tree->get_nodes_in_group("visual_4d");
	for (int i = 0; i < visual_nodes.size(); i++) {
		VisualInstance4D *instance = Object::cast_to<VisualInstance4D>(visual_nodes[i]);
		if (!instance || !instance->is_visible()) {
			continue;
		}

		MeshInstance4D *mesh_inst = Object::cast_to<MeshInstance4D>(instance);
		if (!mesh_inst) {
			continue;
		}

		Ref<Shape4DResourceBase> shape_res = mesh_inst->get_shape();
		if (!shape_res.is_valid()) {
			continue;
		}

		Shape4D *shape = shape_res->get_shape();
		if (!shape) {
			continue;
		}

		Transform4D xform = mesh_inst->get_transform_4d();
		SliceResult result = Slicer4D::slice_shape(shape, xform, plane);
		Array surface_arrays = Slicer4D::build_surface_arrays(result);
		instance->update_render_data(surface_arrays, slice_transform);
	}
}

// -- Bind methods ------------------------------------------------------------

void Camera4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("make_current"), &Camera4D::make_current);
	ClassDB::bind_method(D_METHOD("clear_current"), &Camera4D::clear_current);
	ClassDB::bind_method(D_METHOD("is_current"), &Camera4D::is_current);
	// get_hyperplane() is C++-only (Hyperplane4D is not a Variant type).
	ClassDB::bind_method(D_METHOD("get_camera_rid"), &Camera4D::get_camera_rid);

	ClassDB::bind_method(D_METHOD("set_fov", "fov"), &Camera4D::set_fov);
	ClassDB::bind_method(D_METHOD("get_fov"), &Camera4D::get_fov);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fov", PROPERTY_HINT_RANGE, "1.0,179.0,0.1,degrees"),
			"set_fov", "get_fov");

	ClassDB::bind_method(D_METHOD("set_near", "near"), &Camera4D::set_near);
	ClassDB::bind_method(D_METHOD("get_near"), &Camera4D::get_near);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "near", PROPERTY_HINT_RANGE, "0.001,100.0,0.001"),
			"set_near", "get_near");

	ClassDB::bind_method(D_METHOD("set_far", "far"), &Camera4D::set_far);
	ClassDB::bind_method(D_METHOD("get_far"), &Camera4D::get_far);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "far", PROPERTY_HINT_RANGE, "0.01,100000.0,0.01"),
			"set_far", "get_far");

	ClassDB::bind_method(D_METHOD("set_current", "current"), &Camera4D::set_current);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "set_current", "is_current");
}
