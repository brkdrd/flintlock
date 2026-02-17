#pragma once

#include "shape_4d_resource_base.h"
#include "shapes/hyper_ellipsoid_shape_4d.h"

using namespace godot;

/// HyperEllipsoidShape4DResource — A 4D ellipsoid collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperEllipsoidShape4DResource : public Shape4DResourceBase {
	GDCLASS(HyperEllipsoidShape4DResource, Shape4DResourceBase)

private:
	Vector4 radii = Vector4(1, 1, 1, 1);
	mutable HyperEllipsoidShape4D shape_data;

public:
	HyperEllipsoidShape4DResource();
	~HyperEllipsoidShape4DResource();

	void set_radii(const Vector4 &p_radii);
	Vector4 get_radii() const;

	Shape4D *get_shape() const override;

protected:
	static void _bind_methods();
};
