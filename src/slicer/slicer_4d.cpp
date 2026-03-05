#include "slicer_4d.h"
#include "../nodes/visual/visual_instance_4d.h"
#include "../resources/mesh_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/core/math.hpp>
#include <cmath>

Slicer4D *Slicer4D::singleton = nullptr;

void Slicer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_instance", "instance"), &Slicer4D::register_instance);
	ClassDB::bind_method(D_METHOD("unregister_instance", "instance"), &Slicer4D::unregister_instance);
	ClassDB::bind_method(D_METHOD("mark_dirty", "instance"), &Slicer4D::mark_dirty);
	ClassDB::bind_method(D_METHOD("mark_all_dirty"), &Slicer4D::mark_all_dirty);
	ClassDB::bind_method(D_METHOD("get_instance_count"), &Slicer4D::get_instance_count);
}

Slicer4D::Slicer4D() {
	singleton = this;
}

Slicer4D::~Slicer4D() {
	singleton = nullptr;
}

Slicer4D *Slicer4D::get_singleton() {
	return singleton;
}

void Slicer4D::register_instance(VisualInstance4D *p_instance) {
	if (!_instances.has(p_instance)) {
		_instances.push_back(p_instance);
		_dirty_set.insert(p_instance);
	}
}

void Slicer4D::unregister_instance(VisualInstance4D *p_instance) {
	_instances.erase(p_instance);
	_dirty_set.erase(p_instance);
}

void Slicer4D::mark_dirty(VisualInstance4D *p_instance) {
	_dirty_set.insert(p_instance);
}

void Slicer4D::mark_all_dirty() {
	for (VisualInstance4D *inst : _instances) {
		_dirty_set.insert(inst);
	}
}

// CPU-based tetrahedron-hyperplane intersection for one instance.
// For each tetrahedron in the mesh, compute intersection with the hyperplane.
void Slicer4D::_slice_instance_cpu(VisualInstance4D *p_instance,
	const Vector4 &p_plane_normal, float p_plane_d,
	const PackedFloat32Array &p_basis_cols,
	const Vector4 &p_camera_origin) {

	Ref<Mesh4D> mesh = p_instance->get_mesh_4d();
	if (mesh.is_null() || mesh->get_surface_count() == 0) return;

	// Basis columns: col0 = basis_cols[0..3], col1 = basis_cols[4..7], col2 = basis_cols[8..11]
	Vector4 col0(p_basis_cols[0], p_basis_cols[1], p_basis_cols[2], p_basis_cols[3]);
	Vector4 col1(p_basis_cols[4], p_basis_cols[5], p_basis_cols[6], p_basis_cols[7]);
	Vector4 col2(p_basis_cols[8], p_basis_cols[9], p_basis_cols[10], p_basis_cols[11]);

	// Output 3D vertices and indices
	PackedFloat32Array out_verts, out_normals, out_uvs;
	PackedInt32Array out_indices;
	int tri_idx = 0;

	// Process each surface
	for (int surf = 0; surf < mesh->get_surface_count(); surf++) {
		Array arrays = mesh->get_surface_arrays(surf);
		if (arrays.size() < Mesh4D::ARRAY_MAX) continue;

		PackedFloat32Array verts4d = arrays[Mesh4D::ARRAY_VERTEX];
		PackedFloat32Array norms4d = arrays[Mesh4D::ARRAY_NORMAL];
		PackedInt32Array indices4d = arrays[Mesh4D::ARRAY_INDEX];

		if (verts4d.is_empty() || indices4d.is_empty()) continue;

		// Apply instance 4D transform to vertices
		// Get world transform from the instance
		PackedFloat32Array world_verts = verts4d; // TODO: apply Transform4D

		// Process each tetrahedron (4 indices)
		int tet_count = indices4d.size() / 4;
		for (int t = 0; t < tet_count; t++) {
			int i0 = indices4d[t * 4 + 0];
			int i1 = indices4d[t * 4 + 1];
			int i2 = indices4d[t * 4 + 2];
			int i3 = indices4d[t * 4 + 3];

			// Get 4D vertices
			Vector4 v[4];
			int vis[4] = {i0, i1, i2, i3};
			for (int k = 0; k < 4; k++) {
				int idx = vis[k] * 4;
				if (idx + 3 >= world_verts.size()) { v[k] = Vector4(); continue; }
				v[k] = Vector4(world_verts[idx], world_verts[idx+1], world_verts[idx+2], world_verts[idx+3]);
			}

			// Get 4D normals
			Vector4 n[4];
			for (int k = 0; k < 4; k++) {
				int idx = vis[k] * 4;
				if (norms4d.size() > idx + 3) {
					n[k] = Vector4(norms4d[idx], norms4d[idx+1], norms4d[idx+2], norms4d[idx+3]);
				}
			}

			// Compute signed distances to hyperplane
			float d[4];
			for (int k = 0; k < 4; k++) {
				d[k] = p_plane_normal.dot(v[k]) - p_plane_d;
			}

			// Classify vertices
			int pos_count = 0, neg_count = 0;
			for (int k = 0; k < 4; k++) {
				if (d[k] > 0.0001f) pos_count++;
				else if (d[k] < -0.0001f) neg_count++;
			}

			if (pos_count == 4 || neg_count == 4) continue; // No intersection

			// Find edge intersections
			// Edges of a tetrahedron: (0,1),(0,2),(0,3),(1,2),(1,3),(2,3) = 6 edges
			Vector4 inter_pts[6];
			Vector4 inter_nrm[6];
			int inter_count = 0;

			static const int edges[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};
			for (int e = 0; e < 6; e++) {
				int a = edges[e][0], b = edges[e][1];
				if ((d[a] > 0) == (d[b] > 0)) continue; // Same side
				if (Math::abs(d[a] - d[b]) < 1e-8f) continue;
				float t = d[a] / (d[a] - d[b]);
				inter_pts[inter_count] = v[a].lerp(v[b], t);
				inter_nrm[inter_count] = n[a].lerp(n[b], t);
				inter_count++;
			}

			if (inter_count < 3) continue;

			// Project intersection points to 3D
			auto project_to_3d = [&](const Vector4 &p4) -> Vector3 {
				Vector4 rel = p4 - p_camera_origin;
				return Vector3(col0.dot(rel), col1.dot(rel), col2.dot(rel));
			};

			auto project_normal_to_3d = [&](const Vector4 &n4) -> Vector3 {
				return Vector3(col0.dot(n4), col1.dot(n4), col2.dot(n4)).normalized();
			};

			// Output triangle(s)
			if (inter_count == 3) {
				// One triangle
				for (int k = 0; k < 3; k++) {
					Vector3 p3 = project_to_3d(inter_pts[k]);
					Vector3 n3 = project_normal_to_3d(inter_nrm[k]);
					out_verts.push_back(p3.x); out_verts.push_back(p3.y); out_verts.push_back(p3.z);
					out_normals.push_back(n3.x); out_normals.push_back(n3.y); out_normals.push_back(n3.z);
					out_uvs.push_back(0.0f); out_uvs.push_back(0.0f);
					out_indices.push_back(tri_idx++);
				}
			} else {
				// Quadrilateral (4 or more points) - split into 2 triangles
				// Fan triangulation from first point
				for (int k = 1; k < inter_count - 1; k++) {
					int pts[3] = {0, k, k+1};
					for (int p = 0; p < 3; p++) {
						Vector3 p3 = project_to_3d(inter_pts[pts[p]]);
						Vector3 n3 = project_normal_to_3d(inter_nrm[pts[p]]);
						out_verts.push_back(p3.x); out_verts.push_back(p3.y); out_verts.push_back(p3.z);
						out_normals.push_back(n3.x); out_normals.push_back(n3.y); out_normals.push_back(n3.z);
						out_uvs.push_back(0.0f); out_uvs.push_back(0.0f);
						out_indices.push_back(tri_idx++);
					}
				}
			}
		}
	}

	// Upload to RenderingServer
	if (out_indices.is_empty()) {
		p_instance->clear_rendering_mesh();
		return;
	}

	p_instance->update_rendering_mesh(out_verts, out_normals, out_uvs, out_indices);
}

void Slicer4D::slice_all(const Vector4 &p_plane_normal, float p_plane_d,
	const PackedFloat32Array &p_basis_cols,
	const Vector4 &p_camera_origin) {

	// Check if plane changed - if so, mark all dirty
	if (p_plane_normal != _cached_plane_normal || p_plane_d != _cached_plane_d) {
		mark_all_dirty();
		_cached_plane_normal = p_plane_normal;
		_cached_plane_d = p_plane_d;
	}

	// Process dirty instances
	for (VisualInstance4D *inst : _instances) {
		if (_dirty_set.has(inst)) {
			_slice_instance_cpu(inst, p_plane_normal, p_plane_d, p_basis_cols, p_camera_origin);
		}
	}
	_dirty_set.clear();
}
