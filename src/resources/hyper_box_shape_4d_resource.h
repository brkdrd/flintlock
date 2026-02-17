#pragma once

#include "shape_4d_resource_base.h"
#include "shapes/hyper_box_shape_4d.h"

using namespace godot;

/// HyperBoxShape4DResource — A 4D box (tesseract) collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperBoxShape4DResource : public Shape4DResourceBase {
	GDCLASS(HyperBoxShape4DResource, Shape4DResourceBase)

private:
	Vector4 half_extents = Vector4(1, 1, 1, 1);
	mutable HyperBoxShape4D shape_data;

public:
	HyperBoxShape4DResource();
	~HyperBoxShape4DResource();

	void set_half_extents(const Vector4 &p_half_extents);
	Vector4 get_half_extents() const;

	Shape4D *get_shape() const override;

protected:
	static void _bind_methods();
};
