#pragma once

#include <godot_cpp/classes/editor_node3d_gizmo_plugin.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

class EditorGizmoPlugin4D : public EditorNode3DGizmoPlugin {
	GDCLASS(EditorGizmoPlugin4D, EditorNode3DGizmoPlugin);

protected:
	static void _bind_methods();

public:
	String _get_gizmo_name() const override { return "Node4D"; }
	// Node4D does not inherit Node3D, so no Node3D gizmos are attached automatically.
	// This stub provides a foundation for future gizmo work.
	bool _has_gizmo(Node3D *p_node) const override { return false; }
};
