#pragma once

#include "geometry_instance_4d.h"
#include "resources/shape_4d_resource_base.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref.hpp>

using namespace godot;

/// MeshInstance4D — A 4D mesh node sliced each frame by Camera4D.
///
/// Holds a Shape4DResourceBase (one of the 5 physics shape resources).
/// Camera4D reads the shape and this node's Transform4D each frame to
/// compute the 3D cross-section and push it to VisualInstance4D.
///
/// MeshInstance4D has no frame logic of its own — Camera4D drives everything.
class MeshInstance4D : public GeometryInstance4D {
	GDCLASS(MeshInstance4D, GeometryInstance4D)

private:
	Ref<Shape4DResourceBase> shape;

public:
	MeshInstance4D();
	~MeshInstance4D();

	void set_shape(const Ref<Shape4DResourceBase> &p_shape);
	Ref<Shape4DResourceBase> get_shape() const;

protected:
	static void _bind_methods();
};
