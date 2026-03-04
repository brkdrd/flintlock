#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "sprite_base_4d.h"

namespace godot {

class Sprite4D : public SpriteBase4D {
	GDCLASS(Sprite4D, SpriteBase4D);

protected:
	static void _bind_methods();
};

} // namespace godot
