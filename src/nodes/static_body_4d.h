#pragma once

#include "physics_body_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// StaticBody4D — A static (non-moving) physics body in 4D space.
///
/// Used for immovable objects like walls, floors, obstacles.
class StaticBody4D : public PhysicsBody4D {
	GDCLASS(StaticBody4D, PhysicsBody4D)

private:

public:
	StaticBody4D();
	~StaticBody4D();

	// Godot lifecycle
	void _ready() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();
};
