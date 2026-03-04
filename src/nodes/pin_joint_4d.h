#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

namespace godot {

class PinJoint4D : public Joint4D {
	GDCLASS(PinJoint4D, Joint4D);

protected:
	static void _bind_methods();
};

} // namespace godot
