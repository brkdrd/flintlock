#pragma once

#include "shapes/shape4d.h"
#include "shapes/hyper_sphere_shape_4d.h"
#include "shapes/hyper_box_shape_4d.h"

/// Shape4DResource — Server-side shape resource wrapper.
///
/// Owns a Shape4D instance and provides factory methods for different types.
class Shape4DResource {
public:
	enum Type {
		TYPE_HYPER_SPHERE,
		TYPE_HYPER_BOX,
		TYPE_HYPER_CAPSULE,
		TYPE_HYPER_ELLIPSOID,
		TYPE_CONVEX_HULL
	};

	Shape4DResource(Type p_type);
	~Shape4DResource();

	Type get_type() const { return type; }
	Shape4D *get_shape() const { return shape; }

	// Data setters for different shape types
	void set_sphere_radius(real_t p_radius);
	void set_box_half_extents(const Vector4 &p_extents);

private:
	Type type;
	Shape4D *shape = nullptr;
};
