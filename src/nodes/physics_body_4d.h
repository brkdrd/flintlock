#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/variant.hpp>
#include "collision_object_4d.h"

using namespace godot;

class PhysicsBody4D : public CollisionObject4D {
	GDCLASS(PhysicsBody4D, CollisionObject4D);

protected:
	// Axis lock flags (linear: x=0, y=1, z=2, w=3)
	bool _axis_lock_linear_x  = false;
	bool _axis_lock_linear_y  = false;
	bool _axis_lock_linear_z  = false;
	bool _axis_lock_linear_w  = false;
	// Axis lock flags (angular planes: xy=0, xz=1, xw=2, yz=3, yw=4, zw=5)
	bool _axis_lock_angular_xy = false;
	bool _axis_lock_angular_xz = false;
	bool _axis_lock_angular_xw = false;
	bool _axis_lock_angular_yz = false;
	bool _axis_lock_angular_yw = false;
	bool _axis_lock_angular_zw = false;

	// Creates a body RID via PhysicsServer4D::body_create() and stores it in _rid.
	virtual void _create_physics_body() override;

	static void _bind_methods();

public:
	PhysicsBody4D();
	virtual ~PhysicsBody4D();

	// Axis locks - linear
	void set_axis_lock_linear_x(bool p_lock);
	bool get_axis_lock_linear_x() const;
	void set_axis_lock_linear_y(bool p_lock);
	bool get_axis_lock_linear_y() const;
	void set_axis_lock_linear_z(bool p_lock);
	bool get_axis_lock_linear_z() const;
	void set_axis_lock_linear_w(bool p_lock);
	bool get_axis_lock_linear_w() const;

	// Axis locks - angular
	void set_axis_lock_angular_xy(bool p_lock);
	bool get_axis_lock_angular_xy() const;
	void set_axis_lock_angular_xz(bool p_lock);
	bool get_axis_lock_angular_xz() const;
	void set_axis_lock_angular_xw(bool p_lock);
	bool get_axis_lock_angular_xw() const;
	void set_axis_lock_angular_yz(bool p_lock);
	bool get_axis_lock_angular_yz() const;
	void set_axis_lock_angular_yw(bool p_lock);
	bool get_axis_lock_angular_yw() const;
	void set_axis_lock_angular_zw(bool p_lock);
	bool get_axis_lock_angular_zw() const;

	// Stubs
	Variant move_and_collide(const Vector4 &p_motion);
	bool    test_move(const Vector4 &p_motion);
};
