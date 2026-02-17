#pragma once

#include "visual_instance_4d.h"
#include <godot_cpp/classes/material.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref.hpp>

using namespace godot;

/// GeometryInstance4D — Extends VisualInstance4D with material support.
///
/// Applies a Material to the slice mesh via RenderingServer.
/// The material is set once when changed, not every frame.
class GeometryInstance4D : public VisualInstance4D {
	GDCLASS(GeometryInstance4D, VisualInstance4D)

private:
	Ref<Material> material;

public:
	GeometryInstance4D();
	~GeometryInstance4D();

	void set_material(const Ref<Material> &p_material);
	Ref<Material> get_material() const;

protected:
	static void _bind_methods();
};
