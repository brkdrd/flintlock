#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// StaticBody4D — A static (non-moving) physics body in 4D space.
///
/// Used for immovable objects like walls, floors, obstacles.
class StaticBody4D : public Node3D {
	GDCLASS(StaticBody4D, Node3D)

private:
	flintlock::PhysicsRID body_rid;
	Vector4 position_4d = Vector4(0, 0, 0, 0);

public:
	StaticBody4D();
	~StaticBody4D();

	// Position in 4D
	void set_position_4d(const Vector4 &p_position);
	Vector4 get_position_4d() const;

	// Godot lifecycle
	void _ready() override;
	void _exit_tree() override;

	// Get the internal RID
	flintlock::PhysicsRID get_rid() const;

protected:
	static void _bind_methods();
};
