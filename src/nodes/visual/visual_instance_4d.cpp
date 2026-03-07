#include "visual_instance_4d.h"
#include "../../slicer/slicer_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/viewport.hpp>

void VisualInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_layers"), &VisualInstance4D::get_layers);
	ClassDB::bind_method(D_METHOD("set_layers", "layers"), &VisualInstance4D::set_layers);
	ClassDB::bind_method(D_METHOD("get_sorting_offset"), &VisualInstance4D::get_sorting_offset);
	ClassDB::bind_method(D_METHOD("set_sorting_offset", "offset"), &VisualInstance4D::set_sorting_offset);
	ClassDB::bind_method(D_METHOD("get_instance_rid"), &VisualInstance4D::get_instance_rid);
	ClassDB::bind_method(D_METHOD("get_mesh_rid"), &VisualInstance4D::get_mesh_rid);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "layers", PROPERTY_HINT_LAYERS_3D_RENDER), "set_layers", "get_layers");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sorting_offset"), "set_sorting_offset", "get_sorting_offset");
}

void VisualInstance4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			RenderingServer *rs = RenderingServer::get_singleton();
			_rs_mesh = rs->mesh_create();
			_rs_instance = rs->instance_create();
			rs->instance_set_base(_rs_instance, _rs_mesh);

			// Add to the current world's scenario
			Viewport *vp = get_viewport();
			if (vp) {
				Ref<World3D> world = vp->find_world_3d();
				if (world.is_valid()) {
					rs->instance_set_scenario(_rs_instance, world->get_scenario());
				}
			}

			rs->instance_set_layer_mask(_rs_instance, _layers);

			// Register with Slicer4D
			if (Slicer4D::get_singleton()) {
				Slicer4D::get_singleton()->register_instance(this);
			}
		} break;

		case NOTIFICATION_EXIT_TREE: {
			// Unregister from Slicer4D
			if (Slicer4D::get_singleton()) {
				Slicer4D::get_singleton()->unregister_instance(this);
			}

			// Free RenderingServer resources
			RenderingServer *rs = RenderingServer::get_singleton();
			if (_rs_instance.is_valid()) {
				rs->free_rid(_rs_instance);
				_rs_instance = RID();
			}
			if (_rs_mesh.is_valid()) {
				rs->free_rid(_rs_mesh);
				_rs_mesh = RID();
			}
		} break;

		case NOTIFICATION_VISIBILITY_4D_CHANGED: {
			if (_rs_instance.is_valid()) {
				RenderingServer::get_singleton()->instance_set_visible(_rs_instance, is_visible_in_tree());
			}
		} break;
	}
}

void VisualInstance4D::_on_transform_4d_changed() {
	if (Slicer4D::get_singleton()) {
		Slicer4D::get_singleton()->mark_dirty(this);
	}
}

void VisualInstance4D::update_rendering_mesh(
	const PackedFloat32Array &p_vertices,
	const PackedFloat32Array &p_normals,
	const PackedFloat32Array &p_uvs,
	const PackedInt32Array &p_indices) {

	if (!_rs_mesh.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();

	// Clear existing surfaces
	rs->mesh_clear(_rs_mesh);

	if (p_vertices.is_empty()) return;

	// Build surface arrays for 3D mesh
	Array arrays;
	arrays.resize(RenderingServer::ARRAY_MAX);

	// Convert vertex buffer
	PackedVector3Array verts3d;
	int vert_count = p_vertices.size() / 3;
	verts3d.resize(vert_count);
	for (int i = 0; i < vert_count; i++) {
		verts3d[i] = Vector3(p_vertices[i*3], p_vertices[i*3+1], p_vertices[i*3+2]);
	}
	arrays[RenderingServer::ARRAY_VERTEX] = verts3d;

	// Normals
	if (!p_normals.is_empty()) {
		PackedVector3Array norms3d;
		int norm_count = p_normals.size() / 3;
		norms3d.resize(norm_count);
		for (int i = 0; i < norm_count; i++) {
			norms3d[i] = Vector3(p_normals[i*3], p_normals[i*3+1], p_normals[i*3+2]);
		}
		arrays[RenderingServer::ARRAY_NORMAL] = norms3d;
	}

	// UVs
	if (!p_uvs.is_empty()) {
		PackedVector2Array uvs2d;
		int uv_count = p_uvs.size() / 2;
		uvs2d.resize(uv_count);
		for (int i = 0; i < uv_count; i++) {
			uvs2d[i] = Vector2(p_uvs[i*2], p_uvs[i*2+1]);
		}
		arrays[RenderingServer::ARRAY_TEX_UV] = uvs2d;
	}

	// Indices
	if (!p_indices.is_empty()) {
		arrays[RenderingServer::ARRAY_INDEX] = p_indices;
	}

	rs->mesh_add_surface_from_arrays(_rs_mesh, RenderingServer::PRIMITIVE_TRIANGLES, arrays);

	// Apply material from Material4D (if any)
	Ref<Material> mat = get_active_material_3d();
	if (mat.is_valid()) {
		rs->mesh_surface_set_material(_rs_mesh, 0, mat->get_rid());
	}

	// Place camera-relative geometry into world 3D space using camera's 3D transform.
	Transform3D cam_xform;
	if (Slicer4D::get_singleton()) {
		cam_xform = Slicer4D::get_singleton()->get_camera_3d_transform();
	}
	rs->instance_set_transform(_rs_instance, cam_xform);
}

void VisualInstance4D::clear_rendering_mesh() {
	if (_rs_mesh.is_valid()) {
		RenderingServer::get_singleton()->mesh_clear(_rs_mesh);
	}
}

void VisualInstance4D::set_layers(uint32_t p_layers) {
	_layers = p_layers;
	if (_rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_layer_mask(_rs_instance, _layers);
	}
}

void VisualInstance4D::set_sorting_offset(real_t p_offset) {
	_sorting_offset = p_offset;
}
