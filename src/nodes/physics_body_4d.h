#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "collision_object_4d.h"

namespace godot {

class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D);

protected:
	static void _bind_methods();
};

} // namespace godot
