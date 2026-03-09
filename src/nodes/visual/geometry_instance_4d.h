#pragma once
#include "visual_instance_4d.h"
#include "../../resources/material_4d.h"

using namespace godot;

class GeometryInstance4D : public VisualInstance4D {
	GDCLASS(GeometryInstance4D, VisualInstance4D);

protected:
	Ref<Material4D> _material_override;
	real_t _transparency = 0.0f;

	static void _bind_methods();

public:
	Ref<Material4D> get_material_override() const { return _material_override; }
	void set_material_override(const Ref<Material4D> &p_material);

	Ref<Material> get_active_material_3d() const override;
	void apply_material_params() override;

	real_t get_transparency() const { return _transparency; }
	void set_transparency(real_t p_transparency);
};
