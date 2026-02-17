#include "visual_instance_4d.h"

#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/transform3d.hpp>

using namespace godot;

VisualInstance4D::VisualInstance4D() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		instance_rid = rs->instance_create();
		mesh_rid = rs->mesh_create();
	}
}

VisualInstance4D::~VisualInstance4D() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		if (instance_rid.is_valid()) {
			rs->free_rid(instance_rid);
		}
		if (mesh_rid.is_valid()) {
			rs->free_rid(mesh_rid);
		}
	}
}

void VisualInstance4D::set_visible(bool p_visible) {
	visible = p_visible;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && instance_rid.is_valid()) {
		rs->instance_set_visible(instance_rid, visible);
	}
}

bool VisualInstance4D::is_visible() const {
	return visible;
}

void VisualInstance4D::set_layers(uint32_t p_layers) {
	layers = p_layers;
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs && instance_rid.is_valid()) {
		rs->instance_set_layer_mask(instance_rid, layers);
	}
}

uint32_t VisualInstance4D::get_layers() const {
	return layers;
}

RID VisualInstance4D::get_instance_rid() const {
	return instance_rid;
}

RID VisualInstance4D::get_mesh_rid() const {
	return mesh_rid;
}

void VisualInstance4D::update_render_data(const Array &p_surface_arrays, const Transform3D &p_slice_transform) {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (!rs || !instance_rid.is_valid() || !mesh_rid.is_valid()) {
		return;
	}

	// Clear previous frame's mesh data.
	rs->mesh_clear(mesh_rid);

	// Only add surface if there is actual geometry.
	if (p_surface_arrays.size() > 0 && p_surface_arrays[Mesh::ARRAY_VERTEX].get_type() != Variant::NIL) {
		rs->mesh_add_surface_from_arrays(mesh_rid, RenderingServer::PRIMITIVE_TRIANGLES, p_surface_arrays);
		rs->instance_set_base(instance_rid, mesh_rid);
	}

	rs->instance_set_transform(instance_rid, p_slice_transform);
}

void VisualInstance4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	RenderingServer *rs = RenderingServer::get_singleton();

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Attach to the 3D scenario so the instance is rendered.
			Viewport *vp = get_viewport();
			if (rs && vp && instance_rid.is_valid()) {
				Ref<World3D> world = vp->find_world_3d();
				if (world.is_valid()) {
					rs->instance_set_scenario(instance_rid, world->get_scenario());
				}
			}
			// Register in group so Camera4D can find us.
			add_to_group("visual_4d");
			break;
		}
		case NOTIFICATION_EXIT_TREE: {
			// Detach from scenario.
			if (rs && instance_rid.is_valid()) {
				rs->instance_set_scenario(instance_rid, RID());
			}
			remove_from_group("visual_4d");
			break;
		}
		default:
			break;
	}
}

void VisualInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &VisualInstance4D::set_visible);
	ClassDB::bind_method(D_METHOD("is_visible"), &VisualInstance4D::is_visible);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");

	ClassDB::bind_method(D_METHOD("set_layers", "layers"), &VisualInstance4D::set_layers);
	ClassDB::bind_method(D_METHOD("get_layers"), &VisualInstance4D::get_layers);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "layers", PROPERTY_HINT_LAYERS_3D_RENDER),
			"set_layers", "get_layers");

	ClassDB::bind_method(D_METHOD("get_instance_rid"), &VisualInstance4D::get_instance_rid);
	ClassDB::bind_method(D_METHOD("get_mesh_rid"), &VisualInstance4D::get_mesh_rid);
}
