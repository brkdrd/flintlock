#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "node_4d.h"

namespace godot {

class Joint4D : public Node4D {
	GDCLASS(Joint4D, Node4D);

protected:
	static void _bind_methods();
};

} // namespace godot
