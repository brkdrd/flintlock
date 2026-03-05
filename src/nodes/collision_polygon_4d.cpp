#include "collision_polygon_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

CollisionPolygon4D::CollisionPolygon4D() {}
CollisionPolygon4D::~CollisionPolygon4D() {}

// ─── Polygon ──────────────────────────────────────────────────────────────────

void CollisionPolygon4D::set_polygon(const PackedVector3Array &p_polygon) {
	_polygon = p_polygon;
	_build_shape();
	_update_in_server();
}

PackedVector3Array CollisionPolygon4D::get_polygon() const {
	return _polygon;
}

// ─── Depth ────────────────────────────────────────────────────────────────────

void CollisionPolygon4D::set_depth(float p_depth) {
	_depth = p_depth;
	_build_shape();
	_update_in_server();
}

float CollisionPolygon4D::get_depth() const {
	return _depth;
}

// ─── Disabled ─────────────────────────────────────────────────────────────────

void CollisionPolygon4D::set_disabled(bool p_disabled) {
	_disabled = p_disabled;
	_update_in_server();
}

bool CollisionPolygon4D::is_disabled() const {
	return _disabled;
}

// ─── Margin ───────────────────────────────────────────────────────────────────

void CollisionPolygon4D::set_margin(float p_margin) {
	_margin = p_margin;
	_update_in_server();
}

float CollisionPolygon4D::get_margin() const {
	return _margin;
}

// ─── Shape Building ───────────────────────────────────────────────────────────

void CollisionPolygon4D::_build_shape() {
	if (_polygon.is_empty()) {
		_generated_shape = Ref<ConvexPolygonShape4D>();
		return;
	}

	// Generate a ConvexPolygonShape4D by extruding the 3D polygon along the W axis.
	// Each 3D vertex (x, y, z) becomes two 4D vertices: (x,y,z,0) and (x,y,z,depth).
	// The resulting point set approximates a 4D prism shape.

	PackedFloat32Array points_4d;
	int poly_size = _polygon.size();
	// Two layers: w=0 and w=_depth
	points_4d.resize(poly_size * 2 * 4);

	for (int i = 0; i < poly_size; i++) {
		Vector3 v = _polygon[i];
		// Bottom layer (w = 0)
		points_4d[i * 4 + 0] = v.x;
		points_4d[i * 4 + 1] = v.y;
		points_4d[i * 4 + 2] = v.z;
		points_4d[i * 4 + 3] = 0.0f;
		// Top layer (w = depth)
		int offset = poly_size * 4;
		points_4d[offset + i * 4 + 0] = v.x;
		points_4d[offset + i * 4 + 1] = v.y;
		points_4d[offset + i * 4 + 2] = v.z;
		points_4d[offset + i * 4 + 3] = _depth;
	}

	if (!_generated_shape.is_valid()) {
		_generated_shape.instantiate();
	}
	_generated_shape->set_points(points_4d);
	_generated_shape->set_margin(_margin);
}

// ─── Server Registration ──────────────────────────────────────────────────────

void CollisionPolygon4D::_register_with_parent() {
	Node *parent = get_parent();
	if (!parent) {
		return;
	}
	_parent_collision_object = Object::cast_to<CollisionObject4D>(parent);
	if (!_parent_collision_object) {
		return;
	}

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_generated_shape.is_valid()) {
		return;
	}

	// Create a convex polygon shape on the server (type 4 = convex)
	_shape_rid = ps->shape_create(4);
	ps->shape_set_data(_shape_rid, _generated_shape->get_points());
	// body_add_shape would be called here once CollisionObject4D exposes get_body_rid()
}

void CollisionPolygon4D::_unregister_from_parent() {
	if (!_parent_collision_object) {
		return;
	}

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps && _shape_rid.is_valid()) {
		ps->free_rid(_shape_rid);
		_shape_rid = RID();
	}

	_parent_collision_object = nullptr;
}

void CollisionPolygon4D::_update_in_server() {
	if (!is_inside_tree()) {
		return;
	}
	_unregister_from_parent();
	_register_with_parent();
}

// ─── Notification ─────────────────────────────────────────────────────────────

void CollisionPolygon4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_PARENTED:
			_build_shape();
			_register_with_parent();
			break;
		case NOTIFICATION_UNPARENTED:
			_unregister_from_parent();
			break;
		default:
			break;
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void CollisionPolygon4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_polygon", "polygon"), &CollisionPolygon4D::set_polygon);
	ClassDB::bind_method(D_METHOD("get_polygon"), &CollisionPolygon4D::get_polygon);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR3_ARRAY, "polygon"), "set_polygon", "get_polygon");

	ClassDB::bind_method(D_METHOD("set_depth", "depth"), &CollisionPolygon4D::set_depth);
	ClassDB::bind_method(D_METHOD("get_depth"), &CollisionPolygon4D::get_depth);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "depth"), "set_depth", "get_depth");

	ClassDB::bind_method(D_METHOD("set_disabled", "disabled"), &CollisionPolygon4D::set_disabled);
	ClassDB::bind_method(D_METHOD("is_disabled"), &CollisionPolygon4D::is_disabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "disabled"), "set_disabled", "is_disabled");

	ClassDB::bind_method(D_METHOD("set_margin", "margin"), &CollisionPolygon4D::set_margin);
	ClassDB::bind_method(D_METHOD("get_margin"), &CollisionPolygon4D::get_margin);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "margin", PROPERTY_HINT_RANGE, "0.001,10,0.001,or_greater"),
		"set_margin", "get_margin");
}
