#include "visual_instance_4d.h"
#include "../../servers/visual/visual_server_4d.h"
#include "../../resources/material_4d.h"
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

	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs) return;

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Create VS4D resources
			_vs_mesh = vs->mesh_create();
			_vs_instance = vs->instance_create();
			vs->instance_set_base(_vs_instance, _vs_mesh);

			// Set scenario from viewport
			Viewport *vp = get_viewport();
			if (vp) {
				Ref<World3D> world = vp->find_world_3d();
				if (world.is_valid()) {
					vs->instance_set_scenario(_vs_instance, world->get_scenario());
				}
			}

			vs->instance_set_layer_mask(_vs_instance, _layers);

			// Register this node as the transform source for the instance.
			// Transforms are read LIVE during process_frame(), not cached.
			vs->instance_set_source_node(_vs_instance, this);

			// Upload mesh data and material params.
			// Do NOT push transforms here — process_frame() handles it.
			upload_mesh();
			apply_material_params();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (_vs_instance.is_valid()) {
				vs->free_rid(_vs_instance);
				_vs_instance = RID();
			}
			if (_vs_mesh.is_valid()) {
				vs->free_rid(_vs_mesh);
				_vs_mesh = RID();
			}
		} break;

		case NOTIFICATION_VISIBILITY_4D_CHANGED: {
			if (_vs_instance.is_valid()) {
				vs->instance_set_visible(_vs_instance, is_visible_in_tree());
			}
		} break;
	}
}

void VisualInstance4D::_on_transform_4d_changed() {
	// No-op: transforms are read LIVE from the node each frame during
	// VisualServer4D::process_frame() → _update_instance_shader_transforms().
	// Eagerly caching transforms here can cause phantom meshes when the
	// cache is read before camera globals are set.
}

void VisualInstance4D::upload_mesh() {
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs || !_vs_mesh.is_valid()) return;

	Ref<Mesh4D> mesh = get_mesh_4d();
	vs->mesh_set_data(_vs_mesh, mesh);
}

void VisualInstance4D::update_transform() {
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs || !_vs_instance.is_valid()) return;

	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return;
	Ref<Basis4D> basis = gt->get_basis();
	Ref<Vector4D> origin = gt->get_origin();
	if (basis.is_null() || origin.is_null()) return;

	PackedFloat32Array basis_cols;
	basis_cols.resize(16);
	for (int col = 0; col < 4; col++) {
		basis_cols[col * 4 + 0] = basis->data[col][0];
		basis_cols[col * 4 + 1] = basis->data[col][1];
		basis_cols[col * 4 + 2] = basis->data[col][2];
		basis_cols[col * 4 + 3] = basis->data[col][3];
	}

	Vector4 orig(origin->x, origin->y, origin->z, origin->w);
	vs->instance_set_transform_4d(_vs_instance, basis_cols, orig);
}

void VisualInstance4D::apply_material_params() {
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (!vs || !_vs_instance.is_valid()) return;

	vs->instance_set_material_params(_vs_instance,
		Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 0.0f);
}

void VisualInstance4D::set_layers(uint32_t p_layers) {
	_layers = p_layers;
	VisualServer4D *vs = VisualServer4D::get_singleton();
	if (vs && _vs_instance.is_valid()) {
		vs->instance_set_layer_mask(_vs_instance, _layers);
	}
}

void VisualInstance4D::set_sorting_offset(real_t p_offset) {
	_sorting_offset = p_offset;
}
