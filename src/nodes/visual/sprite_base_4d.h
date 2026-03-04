#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "geometry_instance_4d.h"

namespace godot {

class SpriteBase4D : public GeometryInstance4D {
	GDCLASS(SpriteBase4D, GeometryInstance4D);

protected:
	static void _bind_methods();
};

} // namespace godot
