#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "shapes/shape4d.h"

using namespace godot;

/// Shape4DResourceBase — Abstract base for all saveable 4D shape resources.
///
/// Provides the bridge between Godot's Resource system and the internal
/// Shape4D* used by the physics engine and slicer. Subclasses cache their
/// Shape4D* and recreate it when parameters change.
class Shape4DResourceBase : public Resource {
	GDCLASS(Shape4DResourceBase, Resource)

public:
	Shape4DResourceBase() {}
	virtual ~Shape4DResourceBase() {}

	/// Returns the underlying Shape4D* for physics and slicing.
	/// Ownership is retained by this resource; do NOT delete the pointer.
	virtual Shape4D *get_shape() const = 0;

protected:
	static void _bind_methods() {}
};
