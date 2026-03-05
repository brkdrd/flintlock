#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "physics_body_4d.h"

using namespace godot;

class CharacterBody4D : public PhysicsBody4D {
	GDCLASS(CharacterBody4D, PhysicsBody4D);

public:
	enum MotionMode {
		MOTION_MODE_GROUNDED  = 0,
		MOTION_MODE_FLOATING  = 1,
	};

protected:
	Vector4 _velocity;
	Vector4 _up_direction    = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	int     _max_slides      = 4;
	float   _floor_max_angle = 0.785398f; // ~45 degrees
	int     _motion_mode     = MOTION_MODE_GROUNDED;

	// State set by move_and_slide
	bool _on_floor   = false;
	bool _on_wall    = false;
	bool _on_ceiling = false;

	virtual void _create_physics_body() override;

	static void _bind_methods();

public:
	CharacterBody4D();
	virtual ~CharacterBody4D();

	void    set_velocity(const Vector4 &p_velocity);
	Vector4 get_velocity() const;

	void    set_up_direction(const Vector4 &p_direction);
	Vector4 get_up_direction() const;

	void set_max_slides(int p_max_slides);
	int  get_max_slides() const;

	void  set_floor_max_angle(float p_angle);
	float get_floor_max_angle() const;

	void set_motion_mode(int p_mode);
	int  get_motion_mode() const;

	// Movement — stub: simply translates by velocity * get_physics_process_delta_time()
	bool move_and_slide();

	bool is_on_floor()   const;
	bool is_on_wall()    const;
	bool is_on_ceiling() const;
};

VARIANT_ENUM_CAST(CharacterBody4D::MotionMode);
