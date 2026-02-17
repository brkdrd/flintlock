#pragma once

#include "shape_4d_resource_base.h"
#include "shapes/hyper_capsule_shape_4d.h"

using namespace godot;

/// HyperCapsuleShape4DResource — A 4D capsule collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperCapsuleShape4DResource : public Shape4DResourceBase {
	GDCLASS(HyperCapsuleShape4DResource, Shape4DResourceBase)

private:
	real_t radius = 1.0;
	real_t height = 2.0;
	mutable HyperCapsuleShape4D shape_data;

public:
	HyperCapsuleShape4DResource();
	~HyperCapsuleShape4DResource();

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_height(real_t p_height);
	real_t get_height() const;

	Shape4D *get_shape() const override;

protected:
	static void _bind_methods();
};
