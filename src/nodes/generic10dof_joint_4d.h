#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "joint_4d.h"

namespace godot {

class Generic10DOFJoint4D : public Joint4D {
	GDCLASS(Generic10DOFJoint4D, Joint4D);

protected:
	static void _bind_methods();
};

} // namespace godot
