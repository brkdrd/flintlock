#include "visual_instance_4d.h"
#include "../../slicer/slicer_4d.h"
#include "../../resources/material_4d.h"
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

			// Register with slicer (shared material is applied in upload_gpu_mesh)
			if (Slicer4D::get_singleton()) {
				Slicer4D::get_singleton()->register_instance(this);
			}

			// Upload mesh data to GPU
			upload_gpu_mesh();

			// Do NOT set instance_valid=1.0 here. The shader default is 0.0,
			// which collapses all vertices to degenerate zero-area triangles
			// (discarded in the fragment shader). Slicer4D::update_frame()
			// sets correct camera uniforms FIRST, then iterates all instances
			// calling update_shader_transforms() which sets instance_valid=1.0.
			// Setting it eagerly here would render one+ frames with stale
			// camera globals, producing a phantom clone at the origin.
			apply_material_params();
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
			_gpu_mesh_uploaded = false;
		} break;

		case NOTIFICATION_VISIBILITY_4D_CHANGED: {
			if (_rs_instance.is_valid()) {
				RenderingServer::get_singleton()->instance_set_visible(_rs_instance, is_visible_in_tree());
			}
		} break;
	}
}

void VisualInstance4D::_on_transform_4d_changed() {
	// GPU slicer handles transforms via per-instance shader uniforms.
	// No need to mark dirty — update_shader_transforms() is called each frame
	// by Slicer4D::update_frame().
}

// ============================================================
// upload_gpu_mesh — Pack Mesh4D tetrahedra into GPU vertex format
//
// Each tetrahedron (4 vertices in 4D) becomes 6 GPU vertices
// (2 triangles). All 4 tet vertices + normals are packed into
// every GPU vertex across VERTEX, NORMAL, TANGENT, COLOR,
// UV, UV2, and CUSTOM0-3.
//
// Positions:
//   VERTEX  (vec3)  = va.xyz
//   CUSTOM0 (vec4)  = (va.w, vb.x, vb.y, vb.z)
//   CUSTOM1 (vec4)  = (vb.w, vc.x, vc.y, vc.z)
//   CUSTOM2 (vec4)  = (vc.w, vd.x, vd.y, vd.z)
//   CUSTOM3 (vec4)  = (vd.w, vertex_id, nd.z, nd.w)
//
// Normals (4D, 4 per tet vertex = 16 floats):
//   NORMAL  (vec3)  = na.xyz
//   TANGENT (vec3)  = (na.w, nb.x, nb.y)  [4th tangent float = binormal sign, inaccessible]
//   COLOR   (rgba)  = (nb.z, nb.w, nc.x, nc.y) bias-encoded: (n+1)/2
//   UV      (vec2)  = (nc.z, nc.w)
//   UV2     (vec2)  = (nd.x, nd.y)
//   CUSTOM3.zw      = (nd.z, nd.w)
//
// COLOR is 8-bit RGBA so normals there use bias encoding
// ((n+1)/2 maps [-1,1] to [0,1]). Precision loss is ~0.008
// per component, acceptable after normalize().
// ============================================================
void VisualInstance4D::upload_gpu_mesh() {
	if (!_rs_mesh.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();
	rs->mesh_clear(_rs_mesh);

	Ref<Mesh4D> mesh = get_mesh_4d();
	if (mesh.is_null() || mesh->get_surface_count() == 0) {
		_gpu_mesh_uploaded = false;
		return;
	}

	if (!Slicer4D::get_singleton()) return;
	RID shared_material = Slicer4D::get_singleton()->get_material_rid();

	// Accumulate all tetrahedra from all surfaces
	PackedVector3Array gpu_verts;
	PackedVector3Array gpu_normals;
	PackedFloat32Array gpu_tangents;
	PackedColorArray gpu_colors;
	PackedVector2Array gpu_uvs;
	PackedVector2Array gpu_uv2s;
	PackedFloat32Array gpu_custom0, gpu_custom1, gpu_custom2, gpu_custom3;
	PackedInt32Array gpu_indices;

	int gpu_vert_offset = 0;

	for (int surf = 0; surf < mesh->get_surface_count(); surf++) {
		Array arrays = mesh->get_surface_arrays(surf);
		if (arrays.size() < Mesh4D::ARRAY_MAX) continue;

		PackedFloat32Array verts4d = arrays[Mesh4D::ARRAY_VERTEX];
		PackedInt32Array indices4d = arrays[Mesh4D::ARRAY_INDEX];

		if (verts4d.is_empty() || indices4d.is_empty()) continue;

		// Read normals (4 floats per vertex, may be empty)
		PackedFloat32Array normals4d;
		if (arrays[Mesh4D::ARRAY_NORMAL].get_type() == Variant::PACKED_FLOAT32_ARRAY) {
			normals4d = arrays[Mesh4D::ARRAY_NORMAL];
		}
		bool has_normals = !normals4d.is_empty();

		int tet_count = indices4d.size() / 4;

		// Pre-allocate (6 GPU verts per tet)
		int new_verts = tet_count * 6;
		int base = gpu_verts.size();
		gpu_verts.resize(base + new_verts);
		gpu_normals.resize(base + new_verts);
		gpu_tangents.resize((base + new_verts) * 4);
		gpu_colors.resize(base + new_verts);
		gpu_uvs.resize(base + new_verts);
		gpu_uv2s.resize(base + new_verts);
		gpu_custom0.resize((base + new_verts) * 4);
		gpu_custom1.resize((base + new_verts) * 4);
		gpu_custom2.resize((base + new_verts) * 4);
		gpu_custom3.resize((base + new_verts) * 4);

		for (int t = 0; t < tet_count; t++) {
			int i0 = indices4d[t * 4 + 0];
			int i1 = indices4d[t * 4 + 1];
			int i2 = indices4d[t * 4 + 2];
			int i3 = indices4d[t * 4 + 3];

			// Read the 4 tetrahedron vertices (each 4 floats: x,y,z,w)
			float va[4], vb[4], vc[4], vd[4];
			for (int k = 0; k < 4; k++) {
				va[k] = verts4d[i0 * 4 + k];
				vb[k] = verts4d[i1 * 4 + k];
				vc[k] = verts4d[i2 * 4 + k];
				vd[k] = verts4d[i3 * 4 + k];
			}

			// Read the 4 tetrahedron normals (each 4 floats: x,y,z,w)
			float na[4] = {0,1,0,0}, nb[4] = {0,1,0,0};
			float nc[4] = {0,1,0,0}, nd[4] = {0,1,0,0};
			if (has_normals) {
				for (int k = 0; k < 4; k++) {
					na[k] = normals4d[i0 * 4 + k];
					nb[k] = normals4d[i1 * 4 + k];
					nc[k] = normals4d[i2 * 4 + k];
					nd[k] = normals4d[i3 * 4 + k];
				}
			}

			// Write 6 GPU vertices for this tetrahedron
			for (int v = 0; v < 6; v++) {
				int gi = base + t * 6 + v;

				// === Positions ===
				// VERTEX = va.xyz
				gpu_verts[gi] = Vector3(va[0], va[1], va[2]);

				// CUSTOM0 = (va.w, vb.x, vb.y, vb.z)
				int c0 = gi * 4;
				gpu_custom0[c0 + 0] = va[3];
				gpu_custom0[c0 + 1] = vb[0];
				gpu_custom0[c0 + 2] = vb[1];
				gpu_custom0[c0 + 3] = vb[2];

				// CUSTOM1 = (vb.w, vc.x, vc.y, vc.z)
				gpu_custom1[c0 + 0] = vb[3];
				gpu_custom1[c0 + 1] = vc[0];
				gpu_custom1[c0 + 2] = vc[1];
				gpu_custom1[c0 + 3] = vc[2];

				// CUSTOM2 = (vc.w, vd.x, vd.y, vd.z)
				gpu_custom2[c0 + 0] = vc[3];
				gpu_custom2[c0 + 1] = vd[0];
				gpu_custom2[c0 + 2] = vd[1];
				gpu_custom2[c0 + 3] = vd[2];

				// CUSTOM3 = (vd.w, vertex_id, nd.z, nd.w)
				gpu_custom3[c0 + 0] = vd[3];
				gpu_custom3[c0 + 1] = (float)v;
				gpu_custom3[c0 + 2] = nd[2]; // nd.z packed here
				gpu_custom3[c0 + 3] = nd[3]; // nd.w packed here

				// === Normals ===
				// NORMAL = na.xyz (full precision)
				gpu_normals[gi] = Vector3(na[0], na[1], na[2]);

				// TANGENT = (na.w, nb.x, nb.y, sign) — 4th float is binormal sign, not accessible as TANGENT.w
				gpu_tangents[c0 + 0] = na[3];
				gpu_tangents[c0 + 1] = nb[0];
				gpu_tangents[c0 + 2] = nb[1];
				gpu_tangents[c0 + 3] = 1.0f;

				// COLOR = (nb.z, nb.w, nc.x, nc.y) bias-encoded: (n+1)/2
				gpu_colors[gi] = Color(
					(nb[2] + 1.0f) * 0.5f,
					(nb[3] + 1.0f) * 0.5f,
					(nc[0] + 1.0f) * 0.5f,
					(nc[1] + 1.0f) * 0.5f
				);

				// UV = (nc.z, nc.w)
				gpu_uvs[gi] = Vector2(nc[2], nc[3]);

				// UV2 = (nd.x, nd.y)
				gpu_uv2s[gi] = Vector2(nd[0], nd[1]);
				// nd.z and nd.w are in CUSTOM3.zw
			}

			// Indices: two triangles per tet — (0,1,2) and (3,4,5)
			int vi_base = gpu_vert_offset + t * 6;
			gpu_indices.push_back(vi_base + 0);
			gpu_indices.push_back(vi_base + 1);
			gpu_indices.push_back(vi_base + 2);
			gpu_indices.push_back(vi_base + 3);
			gpu_indices.push_back(vi_base + 4);
			gpu_indices.push_back(vi_base + 5);
		}

		gpu_vert_offset += new_verts;
	}

	if (gpu_verts.is_empty()) {
		_gpu_mesh_uploaded = false;
		return;
	}

	// Build surface arrays
	Array arrays;
	arrays.resize(RenderingServer::ARRAY_MAX);
	arrays[RenderingServer::ARRAY_VERTEX] = gpu_verts;
	arrays[RenderingServer::ARRAY_NORMAL] = gpu_normals;
	arrays[RenderingServer::ARRAY_TANGENT] = gpu_tangents;
	arrays[RenderingServer::ARRAY_COLOR] = gpu_colors;
	arrays[RenderingServer::ARRAY_TEX_UV] = gpu_uvs;
	arrays[RenderingServer::ARRAY_TEX_UV2] = gpu_uv2s;
	arrays[RenderingServer::ARRAY_CUSTOM0] = gpu_custom0;
	arrays[RenderingServer::ARRAY_CUSTOM1] = gpu_custom1;
	arrays[RenderingServer::ARRAY_CUSTOM2] = gpu_custom2;
	arrays[RenderingServer::ARRAY_CUSTOM3] = gpu_custom3;
	arrays[RenderingServer::ARRAY_INDEX] = gpu_indices;

	uint64_t fmt = (uint64_t)RenderingServer::ARRAY_FORMAT_VERTEX
		| (uint64_t)RenderingServer::ARRAY_FORMAT_NORMAL
		| (uint64_t)RenderingServer::ARRAY_FORMAT_TANGENT
		| (uint64_t)RenderingServer::ARRAY_FORMAT_COLOR
		| (uint64_t)RenderingServer::ARRAY_FORMAT_TEX_UV
		| (uint64_t)RenderingServer::ARRAY_FORMAT_TEX_UV2
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM0
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM1
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM2
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM3
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM0_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM1_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM2_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM3_SHIFT);

	rs->mesh_add_surface_from_arrays(_rs_mesh, RenderingServer::PRIMITIVE_TRIANGLES, arrays,
		Array(), Dictionary(), (BitField<RenderingServer::ArrayFormat>)fmt);

	// Apply the shared slicer material
	rs->mesh_surface_set_material(_rs_mesh, 0, shared_material);

	// AABB must be large enough that frustum culling never clips our
	// shader-moved vertices, but small enough for usable shadow maps.
	// 500 units covers any realistic scene while keeping shadow precision.
	rs->mesh_set_custom_aabb(_rs_mesh, AABB(Vector3(-250, -250, -250), Vector3(500, 500, 500)));

	// Identity transform — vertex shader handles positioning
	rs->instance_set_transform(_rs_instance, Transform3D());

	_gpu_mesh_uploaded = true;
}

// ============================================================
// update_shader_transforms — sets per-instance 4D model matrix
// ============================================================
void VisualInstance4D::update_shader_transforms() {
	if (!_rs_instance.is_valid() || !_gpu_mesh_uploaded) return;

	RenderingServer *rs = RenderingServer::get_singleton();

	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return;
	Ref<Basis4D> basis = gt->get_basis();
	Ref<Vector4D> origin = gt->get_origin();
	if (basis.is_null() || origin.is_null()) return;

	// Pack 4x4 basis as 4 vec4 columns
	Vector4 col0(basis->data[0][0], basis->data[0][1], basis->data[0][2], basis->data[0][3]);
	Vector4 col1(basis->data[1][0], basis->data[1][1], basis->data[1][2], basis->data[1][3]);
	Vector4 col2(basis->data[2][0], basis->data[2][1], basis->data[2][2], basis->data[2][3]);
	Vector4 col3(basis->data[3][0], basis->data[3][1], basis->data[3][2], basis->data[3][3]);
	Vector4 model_origin(origin->x, origin->y, origin->z, origin->w);

	rs->instance_geometry_set_shader_parameter(_rs_instance, "model_4d_col0", col0);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "model_4d_col1", col1);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "model_4d_col2", col2);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "model_4d_col3", col3);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "model_4d_origin", model_origin);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "instance_valid", 1.0f);
}

// ============================================================
// apply_material_params — sets per-instance material properties
// ============================================================
void VisualInstance4D::apply_material_params() {
	if (!_rs_instance.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();

	// Default values
	Color albedo(1.0f, 1.0f, 1.0f, 1.0f);
	float roughness = 1.0f;
	float metallic = 0.0f;

	rs->instance_geometry_set_shader_parameter(_rs_instance, "albedo_color", albedo);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "roughness_value", roughness);
	rs->instance_geometry_set_shader_parameter(_rs_instance, "metallic_value", metallic);
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
