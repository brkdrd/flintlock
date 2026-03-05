#pragma once

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/control.hpp>
#include "inspector_plugin_4d.h"
#include "gizmo_plugin_4d.h"
#include "viewport_4d_panel.h"

using namespace godot;

class EditorPlugin4D : public EditorPlugin {
	GDCLASS(EditorPlugin4D, EditorPlugin);

	Ref<EditorInspectorPlugin4D> _inspector_plugin;
	Ref<EditorGizmoPlugin4D> _gizmo_plugin;
	Control *_viewport_panel = nullptr;

protected:
	static void _bind_methods();

public:
	void _enter_tree() override;
	void _exit_tree() override;
	String _get_plugin_name() const override { return "Godot 4D"; }
};
