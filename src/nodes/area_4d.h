#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/callable.hpp>
#include "collision_object_4d.h"

using namespace godot;

class Area4D : public CollisionObject4D {
	GDCLASS(Area4D, CollisionObject4D);

protected:
	bool    _monitoring    = true;
	bool    _monitorable   = true;
	float   _gravity       = 9.8f;
	Vector4 _gravity_direction = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	int     _priority      = 0;

	// Override to create an area RID (not a body RID).
	virtual void _create_physics_body() override;

	// Override notification to use area API instead of body API.
	void _notification(int p_what);

	static void _bind_methods();

public:
	Area4D();
	virtual ~Area4D();

	void set_monitoring(bool p_enable);
	bool get_monitoring() const;

	void set_monitorable(bool p_enable);
	bool get_monitorable() const;

	void  set_gravity(float p_gravity);
	float get_gravity() const;

	void    set_gravity_direction(const Vector4 &p_direction);
	Vector4 get_gravity_direction() const;

	void set_priority(int p_priority);
	int  get_priority() const;

	// Internal monitor callbacks — called by PhysicsServer4D when overlap begins/ends.
	void _body_monitor_callback(int p_status, const RID &p_rid, int p_instance_id, int p_body_shape, int p_area_shape);
	void _area_monitor_callback(int p_status, const RID &p_rid, int p_instance_id, int p_area_shape, int p_self_shape);
};
