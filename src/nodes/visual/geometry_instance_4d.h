#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "visual_instance_4d.h"

namespace godot {

class GeometryInstance4D : public VisualInstance4D {
	GDCLASS(GeometryInstance4D, VisualInstance4D);

protected:
	static void _bind_methods();
};

} // namespace godot
