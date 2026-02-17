#pragma once

#include "collision_object_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// PhysicsBody4D — Abstract base class for all physics-enabled 4D bodies.
///
/// Inherits from CollisionObject4D and adds physics server body management.
/// Similar to PhysicsBody2D and PhysicsBody3D in Godot.
class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D)

public:
	PhysicsBody4D();
	virtual ~PhysicsBody4D();

	// Override get_rid to return the body RID.
	RID get_rid() const override;

	// -- Godot lifecycle -----------------------------------------------------

	virtual void _ready() override;
	virtual void _exit_tree() override;

protected:
	static void _bind_methods();
};
