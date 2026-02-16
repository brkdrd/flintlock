#pragma once

#include "physics_body_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "server/physics_server_4d.h"

using namespace godot;

/// CharacterBody4D — A kinematic physics body for character control in 4D.
///
/// Provides move_and_slide functionality for character movement with collision response.
/// Does not respond to forces/gravity automatically - movement is script-controlled.
class CharacterBody4D : public PhysicsBody4D {
	GDCLASS(CharacterBody4D, PhysicsBody4D)

private:
	Vector4 velocity = Vector4(0, 0, 0, 0);
	Vector4 up_direction = Vector4(0, 0, 0, 1);
	real_t floor_max_angle = 0.785398; // 45 degrees in radians
	bool floor_stop_on_slope = true;
	bool floor_constant_speed = false;
	bool floor_block_on_wall = true;
	int max_slides = 4;
	real_t safe_margin = 0.001;

public:
	CharacterBody4D();
	~CharacterBody4D();

	// -- Velocity ------------------------------------------------------------

	void set_velocity(const Vector4 &p_velocity);
	Vector4 get_velocity() const;

	// -- Movement ------------------------------------------------------------

	void set_up_direction(const Vector4 &p_direction);
	Vector4 get_up_direction() const;

	void set_floor_max_angle(real_t p_angle);
	real_t get_floor_max_angle() const;

	void set_floor_stop_on_slope(bool p_enabled);
	bool is_floor_stop_on_slope_enabled() const;

	void set_floor_constant_speed(bool p_enabled);
	bool is_floor_constant_speed_enabled() const;

	void set_floor_block_on_wall(bool p_enabled);
	bool is_floor_block_on_wall_enabled() const;

	void set_max_slides(int p_max_slides);
	int get_max_slides() const;

	void set_safe_margin(real_t p_margin);
	real_t get_safe_margin() const;

	// -- Kinematic motion ----------------------------------------------------

	void move_and_slide();
	bool is_on_floor() const;
	bool is_on_wall() const;
	bool is_on_ceiling() const;
	Vector4 get_floor_normal() const;
	Vector4 get_wall_normal() const;

	// -- Godot lifecycle -----------------------------------------------------

	void _ready() override;
	void _exit_tree() override;

protected:
	static void _bind_methods();
};
