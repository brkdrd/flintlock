#pragma once

#include "shape_4d_resource_base.h"
#include "shapes/hyper_sphere_shape_4d.h"

using namespace godot;

/// HyperSphereShape4D — A spherical collision shape in 4D (hypersphere).
///
/// This is a Godot Resource that can be saved and reused.
class HyperSphereShape4DResource : public Shape4DResourceBase {
	GDCLASS(HyperSphereShape4DResource, Shape4DResourceBase)

private:
	real_t radius = 1.0;
	mutable HyperSphereShape4D shape_data;

public:
	HyperSphereShape4DResource();
	~HyperSphereShape4DResource();

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	Shape4D *get_shape() const override;

protected:
	static void _bind_methods();
};
