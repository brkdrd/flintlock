#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "physics_body_4d.h"

namespace godot {

class RigidBody4D : public PhysicsBody4D {
	GDCLASS(RigidBody4D, PhysicsBody4D);

protected:
	static void _bind_methods();
};

} // namespace godot
