#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "visual_instance_4d.h"

namespace godot {

class Light4D : public VisualInstance4D {
	GDCLASS(Light4D, VisualInstance4D);

protected:
	static void _bind_methods();
};

} // namespace godot
