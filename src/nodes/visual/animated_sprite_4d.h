#pragma once

#include <godot_cpp/core/class_db.hpp>
#include "sprite_base_4d.h"

namespace godot {

class AnimatedSprite4D : public SpriteBase4D {
	GDCLASS(AnimatedSprite4D, SpriteBase4D);

protected:
	static void _bind_methods();
};

} // namespace godot
