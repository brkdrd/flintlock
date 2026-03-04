#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "light_4d.h"

namespace godot {

class OmniLight4D : public Light4D {
	GDCLASS(OmniLight4D, Light4D);

protected:
	static void _bind_methods();
};

} // namespace godot
