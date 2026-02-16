#pragma once

#include "node_4d.h"
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Forward declaration
class Shape4DResource;

/// CollisionShape4D — Defines the collision shape for a physics body.
///
/// Must be a child of RigidBody4D or StaticBody4D.
/// The shape property determines the collision geometry.
class CollisionShape4D : public Node4D {
	GDCLASS(CollisionShape4D, Node4D)

private:
	Ref<Resource> shape;
	bool disabled = false;

public:
	CollisionShape4D();
	~CollisionShape4D();

	// Shape resource
	void set_shape(const Ref<Resource> &p_shape);
	Ref<Resource> get_shape() const;

	// Disabled state
	void set_disabled(bool p_disabled);
	bool is_disabled() const;

	// Godot lifecycle
	void _ready() override;

protected:
	static void _bind_methods();
};
