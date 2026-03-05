#include "collision_shape_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

CollisionShape4D::CollisionShape4D() {}
CollisionShape4D::~CollisionShape4D() {}

// ─── Shape ────────────────────────────────────────────────────────────────────

void CollisionShape4D::set_shape(const Ref<Shape4D> &p_shape) {
	_shape = p_shape;
	_update_in_server();
}

Ref<Shape4D> CollisionShape4D::get_shape() const {
	return _shape;
}

// ─── Disabled ─────────────────────────────────────────────────────────────────

void CollisionShape4D::set_disabled(bool p_disabled) {
	_disabled = p_disabled;
	_update_in_server();
}

bool CollisionShape4D::is_disabled() const {
	return _disabled;
}

// ─── Debug ────────────────────────────────────────────────────────────────────

void CollisionShape4D::set_debug_color(const Color &p_color) {
	_debug_color = p_color;
}

Color CollisionShape4D::get_debug_color() const {
	return _debug_color;
}

void CollisionShape4D::set_debug_fill(bool p_fill) {
	_debug_fill = p_fill;
}

bool CollisionShape4D::get_debug_fill() const {
	return _debug_fill;
}

// ─── Internal ─────────────────────────────────────────────────────────────────

void CollisionShape4D::_register_with_parent() {
	Node *parent = get_parent();
	if (!parent) {
		return;
	}
	_parent_collision_object = Object::cast_to<CollisionObject4D>(parent);
	if (!_parent_collision_object) {
		return;
	}

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_shape.is_valid()) {
		return;
	}

	// Determine shape type from the Shape4D subclass
	// Type constants: 0=box, 1=sphere, 2=capsule, 3=cylinder, 4=convex, 5=concave, 6=world_boundary, 7=ray
	int shape_type = 4; // default to convex
	if (Object::cast_to<BoxShape4D>(*_shape)) {
		shape_type = 0;
	} else if (Object::cast_to<SphereShape4D>(*_shape)) {
		shape_type = 1;
	} else if (Object::cast_to<CapsuleShape4D>(*_shape)) {
		shape_type = 2;
	} else if (Object::cast_to<CylinderShape4D>(*_shape)) {
		shape_type = 3;
	} else if (Object::cast_to<ConvexPolygonShape4D>(*_shape)) {
		shape_type = 4;
	} else if (Object::cast_to<ConcavePolygonShape4D>(*_shape)) {
		shape_type = 5;
	} else if (Object::cast_to<WorldBoundaryShape4D>(*_shape)) {
		shape_type = 6;
	} else if (Object::cast_to<SeparationRayShape4D>(*_shape)) {
		shape_type = 7;
	}

	// Create shape RID on the server
	_shape_rid = ps->shape_create(shape_type);
	ps->shape_set_data(_shape_rid, _shape->get_margin() >= 0 ? PackedFloat32Array() : PackedFloat32Array());

	// Build local transform as PackedFloat32Array (20 floats: 4x4 basis + 4 origin)
	// For now we pass an empty transform (identity) — full transform wiring requires
	// CollisionObject4D to expose a body/area RID. This is forward-compatible.
	// NOTE: CollisionObject4D is currently a stub. body_add_shape is called here
	// with the assumption that CollisionObject4D will expose get_body_rid() in Stage 6.
	// ps->body_add_shape(body_rid, _shape_rid, local_transform_packed, _disabled);
}

void CollisionShape4D::_unregister_from_parent() {
	if (!_parent_collision_object) {
		return;
	}

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps && _shape_rid.is_valid()) {
		// Simplified cleanup — free the shape RID we created
		ps->free_rid(_shape_rid);
		_shape_rid = RID();
	}

	_parent_collision_object = nullptr;
	_shape_index = -1;
}

void CollisionShape4D::_update_in_server() {
	if (!is_inside_tree()) {
		return;
	}
	_unregister_from_parent();
	_register_with_parent();
}

// ─── Notification ─────────────────────────────────────────────────────────────

void CollisionShape4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_PARENTED:
			_register_with_parent();
			break;
		case NOTIFICATION_UNPARENTED:
			_unregister_from_parent();
			break;
		case NOTIFICATION_TRANSFORM_4D_CHANGED:
			_update_in_server();
			break;
		default:
			break;
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void CollisionShape4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_shape", "shape"), &CollisionShape4D::set_shape);
	ClassDB::bind_method(D_METHOD("get_shape"), &CollisionShape4D::get_shape);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shape", PROPERTY_HINT_RESOURCE_TYPE, "Shape4D"),
		"set_shape", "get_shape");

	ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &CollisionShape4D::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &CollisionShape4D::is_disabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");

	ClassDB::bind_method(D_METHOD("set_debug_color", "color"), &CollisionShape4D::set_debug_color);
	ClassDB::bind_method(D_METHOD("get_debug_color"), &CollisionShape4D::get_debug_color);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "debug_color"), "set_debug_color", "get_debug_color");

	ClassDB::bind_method(D_METHOD("set_debug_fill", "fill"), &CollisionShape4D::set_debug_fill);
	ClassDB::bind_method(D_METHOD("get_debug_fill"), &CollisionShape4D::get_debug_fill);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "debug_fill"), "set_debug_fill", "get_debug_fill");
}
