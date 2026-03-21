#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "static_body_4d.h"

using namespace godot;

class AnimatableBody4D : public StaticBody4D {
	GDCLASS(AnimatableBody4D, StaticBody4D);

protected:
	bool _sync_to_physics = false;

	virtual void _create_physics_body() override;
	virtual void _configure_physics_body() override;

	static void _bind_methods();

public:
	AnimatableBody4D();
	virtual ~AnimatableBody4D();

	void set_sync_to_physics(bool p_sync);
	bool get_sync_to_physics() const;
};
