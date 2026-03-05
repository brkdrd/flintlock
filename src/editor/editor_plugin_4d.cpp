#include "editor_plugin_4d.h"
#include "inspector_plugin_4d.h"
#include "gizmo_plugin_4d.h"
#include "viewport_4d_panel.h"

using namespace godot;

void EditorPlugin4D::_bind_methods() {}

void EditorPlugin4D::_enter_tree() {
	_inspector_plugin.instantiate();
	add_inspector_plugin(_inspector_plugin);

	_gizmo_plugin.instantiate();
	add_node_3d_gizmo_plugin(_gizmo_plugin);

	_viewport_panel = memnew(Viewport4DPanel);
	add_control_to_bottom_panel(_viewport_panel, "4D View");
}

void EditorPlugin4D::_exit_tree() {
	if (_inspector_plugin.is_valid()) {
		remove_inspector_plugin(_inspector_plugin);
		_inspector_plugin.unref();
	}
	if (_gizmo_plugin.is_valid()) {
		remove_node_3d_gizmo_plugin(_gizmo_plugin);
		_gizmo_plugin.unref();
	}
	if (_viewport_panel) {
		remove_control_from_bottom_panel(_viewport_panel);
		memdelete(_viewport_panel);
		_viewport_panel = nullptr;
	}
}
