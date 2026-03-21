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
	if (_parent_collision_object && _shape_index >= 0) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) {
			RID body_rid = _parent_collision_object->get_rid();
			ps->body_set_shape_disabled(body_rid, _shape_index, _disabled);
		}
	}
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

static int _get_shape_type(const Ref<Shape4D> &shape) {
	if (Object::cast_to<BoxShape4D>(*shape)) return 0;
	if (Object::cast_to<SphereShape4D>(*shape)) return 1;
	if (Object::cast_to<CapsuleShape4D>(*shape)) return 2;
	if (Object::cast_to<CylinderShape4D>(*shape)) return 3;
	if (Object::cast_to<ConvexPolygonShape4D>(*shape)) return 4;
	if (Object::cast_to<ConcavePolygonShape4D>(*shape)) return 5;
	if (Object::cast_to<WorldBoundaryShape4D>(*shape)) return 6;
	if (Object::cast_to<SeparationRayShape4D>(*shape)) return 7;
	return 4; // default to convex
}

static PackedFloat32Array _get_shape_data(const Ref<Shape4D> &shape) {
	PackedFloat32Array data;

	if (auto *box = Object::cast_to<BoxShape4D>(*shape)) {
		Vector4 size = box->get_size();
		data.resize(4);
		data.set(0, size.x);
		data.set(1, size.y);
		data.set(2, size.z);
		data.set(3, size.w);
	} else if (auto *sphere = Object::cast_to<SphereShape4D>(*shape)) {
		data.resize(1);
		data.set(0, sphere->get_radius());
	} else if (auto *capsule = Object::cast_to<CapsuleShape4D>(*shape)) {
		data.resize(2);
		data.set(0, capsule->get_radius());
		data.set(1, capsule->get_height());
	} else if (auto *cylinder = Object::cast_to<CylinderShape4D>(*shape)) {
		data.resize(2);
		data.set(0, cylinder->get_radius());
		data.set(1, cylinder->get_height());
	} else if (auto *convex = Object::cast_to<ConvexPolygonShape4D>(*shape)) {
		data = convex->get_points();
	} else if (auto *concave = Object::cast_to<ConcavePolygonShape4D>(*shape)) {
		data = concave->get_faces();
	} else if (auto *wb = Object::cast_to<WorldBoundaryShape4D>(*shape)) {
		Vector4 n = wb->get_normal();
		data.resize(5);
		data.set(0, n.x);
		data.set(1, n.y);
		data.set(2, n.z);
		data.set(3, n.w);
		data.set(4, wb->get_distance());
	} else if (auto *ray = Object::cast_to<SeparationRayShape4D>(*shape)) {
		data.resize(1);
		data.set(0, ray->get_length());
	}

	return data;
}

void CollisionShape4D::_register_with_parent() {
	Node *parent = get_parent();
	if (!parent) return;

	_parent_collision_object = Object::cast_to<CollisionObject4D>(parent);
	if (!_parent_collision_object) return;

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_shape.is_valid()) return;

	RID body_rid = _parent_collision_object->get_rid();
	if (!body_rid.is_valid()) return;

	// Create shape RID with correct type and data
	int shape_type = _get_shape_type(_shape);
	_shape_rid = ps->shape_create(shape_type);
	PackedFloat32Array shape_data = _get_shape_data(_shape);
	ps->shape_set_data(_shape_rid, shape_data);

	// Build local transform as PackedFloat32Array (20 floats)
	// Use this node's local transform relative to parent
	PackedFloat32Array local_xf;
	local_xf.resize(20);
	Ref<Transform4D> xform = get_transform_4d();
	if (xform.is_valid()) {
		Ref<Basis4D> basis = xform->get_basis();
		if (basis.is_valid()) {
			for (int col = 0; col < 4; col++) {
				local_xf.set(col * 4 + 0, basis->data[col][0]);
				local_xf.set(col * 4 + 1, basis->data[col][1]);
				local_xf.set(col * 4 + 2, basis->data[col][2]);
				local_xf.set(col * 4 + 3, basis->data[col][3]);
			}
		} else {
			for (int col = 0; col < 4; col++)
				for (int row = 0; row < 4; row++)
					local_xf.set(col * 4 + row, (col == row) ? 1.0f : 0.0f);
		}
		Ref<Vector4D> origin = xform->get_origin();
		if (origin.is_valid()) {
			local_xf.set(16, origin->x);
			local_xf.set(17, origin->y);
			local_xf.set(18, origin->z);
			local_xf.set(19, origin->w);
		}
	} else {
		for (int col = 0; col < 4; col++)
			for (int row = 0; row < 4; row++)
				local_xf.set(col * 4 + row, (col == row) ? 1.0f : 0.0f);
		local_xf.set(16, 0.0f);
		local_xf.set(17, 0.0f);
		local_xf.set(18, 0.0f);
		local_xf.set(19, 0.0f);
	}

	// Get the shape index before adding
	_shape_index = ps->body_get_shape_count(body_rid);

	// Add shape to the body
	ps->body_add_shape(body_rid, _shape_rid, local_xf, _disabled);
}

void CollisionShape4D::_unregister_from_parent() {
	if (!_parent_collision_object) return;

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		RID body_rid = _parent_collision_object->get_rid();
		if (body_rid.is_valid() && _shape_index >= 0) {
			ps->body_remove_shape(body_rid, _shape_index);
		}
		if (_shape_rid.is_valid()) {
			ps->free_rid(_shape_rid);
			_shape_rid = RID();
		}
	}

	_parent_collision_object = nullptr;
	_shape_index = -1;
}

void CollisionShape4D::_update_in_server() {
	if (!is_inside_tree()) return;
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
