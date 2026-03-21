#include "area_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Area4D::Area4D() {}
Area4D::~Area4D() {}

// ---------------------------------------------------------------------------
// Create physics area
// ---------------------------------------------------------------------------

void Area4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		_rid = ps->area_create();
	}
}

// ---------------------------------------------------------------------------
// Notification override — uses area API rather than body API
// ---------------------------------------------------------------------------

void Area4D::_notification(int p_what) {
	// Still need Node4D's base bookkeeping.
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_create_physics_body();

			if (_rid.is_valid()) {
				PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
				if (ps) {
					// Assign to the default space
					RID default_space = ps->get_default_space();
					ps->area_set_space(_rid, default_space);

					ps->area_set_object_instance_id(_rid, get_instance_id());
					ps->area_set_monitorable(_rid, _monitorable);
					ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_GRAVITY, _gravity);
					ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_GRAVITY_VECTOR, _gravity_direction);
					ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_PRIORITY, _priority);

					PackedFloat32Array xf = _get_transform_array();
					ps->area_set_transform(_rid, xf);

					// Register monitor callbacks.
					ps->area_set_monitor_callback(_rid, Callable(this, "_body_monitor_callback"));
					ps->area_set_area_monitor_callback(_rid, Callable(this, "_area_monitor_callback"));
				}
			}
			set_notify_transform_4d(true);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			if (_rid.is_valid()) {
				PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
				if (ps) {
					ps->free_rid(_rid);
				}
				_rid = RID();
			}
			set_notify_transform_4d(false);
		} break;

		case NOTIFICATION_TRANSFORM_4D_CHANGED: {
			if (_rid.is_valid()) {
				PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
				if (ps) {
					PackedFloat32Array xf = _get_transform_array();
					ps->area_set_transform(_rid, xf);
				}
			}
		} break;

		default:
			break;
	}
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void Area4D::set_monitoring(bool p_enable) {
	_monitoring = p_enable;
}
bool Area4D::get_monitoring() const { return _monitoring; }

void Area4D::set_monitorable(bool p_enable) {
	_monitorable = p_enable;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->area_set_monitorable(_rid, p_enable);
	}
}
bool Area4D::get_monitorable() const { return _monitorable; }

void Area4D::set_gravity(float p_gravity) {
	_gravity = p_gravity;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_GRAVITY, p_gravity);
	}
}
float Area4D::get_gravity() const { return _gravity; }

void Area4D::set_gravity_direction(const Vector4 &p_direction) {
	_gravity_direction = p_direction;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_GRAVITY_VECTOR, p_direction);
	}
}
Vector4 Area4D::get_gravity_direction() const { return _gravity_direction; }

void Area4D::set_priority(int p_priority) {
	_priority = p_priority;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->area_set_param(_rid, PhysicsServer4D::AREA_PARAM_PRIORITY, p_priority);
	}
}
int Area4D::get_priority() const { return _priority; }

// ---------------------------------------------------------------------------
// Monitor callbacks
// ---------------------------------------------------------------------------

void Area4D::_body_monitor_callback(int p_status, const RID &p_rid, int p_instance_id, int p_body_shape, int p_area_shape) {
	if (p_instance_id == 0) return;

	Object *obj = ObjectDB::get_instance(ObjectID((uint64_t)p_instance_id));
	if (!obj) return;

	if (p_status == 1) {
		emit_signal("body_entered", obj);
	} else {
		emit_signal("body_exited", obj);
	}
}

void Area4D::_area_monitor_callback(int p_status, const RID &p_rid, int p_instance_id, int p_area_shape, int p_self_shape) {
	if (p_instance_id == 0) return;

	Object *obj = ObjectDB::get_instance(ObjectID((uint64_t)p_instance_id));
	if (!obj) return;

	if (p_status == 1) {
		emit_signal("area_entered", obj);
	} else {
		emit_signal("area_exited", obj);
	}
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void Area4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_monitoring", "enable"), &Area4D::set_monitoring);
	ClassDB::bind_method(D_METHOD("get_monitoring"), &Area4D::get_monitoring);
	ClassDB::bind_method(D_METHOD("set_monitorable", "enable"), &Area4D::set_monitorable);
	ClassDB::bind_method(D_METHOD("get_monitorable"), &Area4D::get_monitorable);
	ClassDB::bind_method(D_METHOD("set_gravity", "gravity"), &Area4D::set_gravity);
	ClassDB::bind_method(D_METHOD("get_gravity"), &Area4D::get_gravity);
	ClassDB::bind_method(D_METHOD("set_gravity_direction", "direction"), &Area4D::set_gravity_direction);
	ClassDB::bind_method(D_METHOD("get_gravity_direction"), &Area4D::get_gravity_direction);
	ClassDB::bind_method(D_METHOD("set_priority", "priority"), &Area4D::set_priority);
	ClassDB::bind_method(D_METHOD("get_priority"), &Area4D::get_priority);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitoring"), "set_monitoring", "get_monitoring");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "monitorable"), "set_monitorable", "get_monitorable");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "gravity_direction"), "set_gravity_direction", "get_gravity_direction");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "priority"), "set_priority", "get_priority");

	// Internal callbacks (must be bound so Callable(this, "...") works)
	ClassDB::bind_method(D_METHOD("_body_monitor_callback", "status", "rid", "instance_id", "body_shape", "area_shape"),
		&Area4D::_body_monitor_callback);
	ClassDB::bind_method(D_METHOD("_area_monitor_callback", "status", "rid", "instance_id", "area_shape", "self_shape"),
		&Area4D::_area_monitor_callback);

	// Signals
	ADD_SIGNAL(MethodInfo("body_entered", PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_NODE_TYPE, "Node")));
	ADD_SIGNAL(MethodInfo("body_exited",  PropertyInfo(Variant::OBJECT, "body", PROPERTY_HINT_NODE_TYPE, "Node")));
	ADD_SIGNAL(MethodInfo("area_entered", PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_NODE_TYPE, "Area4D")));
	ADD_SIGNAL(MethodInfo("area_exited",  PropertyInfo(Variant::OBJECT, "area", PROPERTY_HINT_NODE_TYPE, "Area4D")));
}
