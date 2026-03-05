#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>

using namespace godot;

class PhysicsServer4D;

// PhysicsDirectBodyState4D - provides direct access to a body's state during physics callbacks.
// Analog of PhysicsDirectBodyState3D.
class PhysicsDirectBodyState4D : public RefCounted {
	GDCLASS(PhysicsDirectBodyState4D, RefCounted);

public:
	RID _body_rid;
	PhysicsServer4D *_server = nullptr;

protected:
	static void _bind_methods();

public:
	// Transform: 20 floats (4x4 basis column-major + 4 origin)
	PackedFloat32Array get_transform() const;
	void set_transform(const PackedFloat32Array &p_transform);

	Vector4 get_linear_velocity() const;
	void set_linear_velocity(const Vector4 &p_velocity);

	PackedFloat32Array get_angular_velocity() const; // 6-component bivector
	void set_angular_velocity(const PackedFloat32Array &p_angular_vel);

	real_t get_total_gravity_magnitude() const;
	Vector4 get_total_linear_damp() const;
	real_t get_step() const;

	void apply_central_force(const Vector4 &p_force);
	void apply_force(const Vector4 &p_force, const Vector4 &p_position);
	void apply_central_impulse(const Vector4 &p_impulse);
	void apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position);

	bool is_sleeping() const;
};
