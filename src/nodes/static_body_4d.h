#pragma once

#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// StaticBody4D — A static (non-moving) physics body in 4D space.
///
/// Used for immovable objects like walls, floors, obstacles.
class StaticBody4D : public Node4D {
	GDCLASS(StaticBody4D, Node4D)

private:
	flintlock::PhysicsRID body_rid;

public:
	StaticBody4D();
	~StaticBody4D();

	// Godot lifecycle
	void _ready() override;
	void _exit_tree() override;

	// Get the internal RID
	flintlock::PhysicsRID get_rid() const;

protected:
	static void _bind_methods();
};
