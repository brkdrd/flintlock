#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "geometry_instance_4d.h"

namespace godot {

class MeshInstance4D : public GeometryInstance4D {
	GDCLASS(MeshInstance4D, GeometryInstance4D);

protected:
	static void _bind_methods();
};

} // namespace godot
