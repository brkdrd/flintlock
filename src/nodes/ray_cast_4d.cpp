#include "ray_cast_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include "../servers/physics/physics_direct_space_state_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

RayCast4D::RayCast4D() {}
RayCast4D::~RayCast4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void RayCast4D::set_enabled(bool p_enabled) {
	_enabled = p_enabled;
	set_physics_process(_enabled && is_inside_tree());
}

bool RayCast4D::is_enabled() const {
	return _enabled;
}

void RayCast4D::set_target_position(const Vector4 &p_pos) {
	_target_position = p_pos;
}

Vector4 RayCast4D::get_target_position() const {
	return _target_position;
}

void RayCast4D::set_collision_mask(uint32_t p_mask) {
	_collision_mask = p_mask;
}

uint32_t RayCast4D::get_collision_mask() const {
	return _collision_mask;
}

void RayCast4D::set_exclude_parent(bool p_exclude) {
	_exclude_parent = p_exclude;
}

bool RayCast4D::get_exclude_parent() const {
	return _exclude_parent;
}

void RayCast4D::set_hit_from_inside(bool p_hit) {
	_hit_from_inside = p_hit;
}

bool RayCast4D::get_hit_from_inside() const {
	return _hit_from_inside;
}

void RayCast4D::set_collide_with_areas(bool p_enable) {
	_collide_with_areas = p_enable;
}

bool RayCast4D::is_collide_with_areas_enabled() const {
	return _collide_with_areas;
}

void RayCast4D::set_collide_with_bodies(bool p_enable) {
	_collide_with_bodies = p_enable;
}

bool RayCast4D::is_collide_with_bodies_enabled() const {
	return _collide_with_bodies;
}

// ─── Query Results ────────────────────────────────────────────────────────────

bool RayCast4D::is_colliding() const {
	return _is_colliding;
}

Object *RayCast4D::get_collider() const {
	return _collider;
}

Vector4 RayCast4D::get_collision_point() const {
	return _collision_point;
}

Vector4 RayCast4D::get_collision_normal() const {
	return _collision_normal;
}

// ─── Control ──────────────────────────────────────────────────────────────────

void RayCast4D::force_raycast_update() {
	_perform_raycast();
}

void RayCast4D::add_exception(Object *p_obj) {
	if (!p_obj) return;
	CollisionObject4D *co = Object::cast_to<CollisionObject4D>(p_obj);
	if (co) {
		RID rid = co->get_rid();
		if (rid.is_valid()) {
			_exclude_rids.push_back(rid);
		}
	}
}

void RayCast4D::remove_exception(Object *p_obj) {
	if (!p_obj) return;
	CollisionObject4D *co = Object::cast_to<CollisionObject4D>(p_obj);
	if (co) {
		RID rid = co->get_rid();
		for (int i = 0; i < _exclude_rids.size(); i++) {
			RID r = _exclude_rids[i];
			if (r == rid) {
				_exclude_rids.remove_at(i);
				break;
			}
		}
	}
}

void RayCast4D::clear_exceptions() {
	_exclude_rids.clear();
}

// ─── Raycast Implementation ───────────────────────────────────────────────────

void RayCast4D::_perform_raycast() {
	// Reset result state
	_is_colliding = false;
	_collider = nullptr;
	_collision_point = Vector4();
	_collision_normal = Vector4();

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps) return;

	// Get the default space's direct state
	RID default_space = ps->get_default_space();
	Ref<PhysicsDirectSpaceState4D> space_state = ps->space_get_direct_state(default_space);
	if (!space_state.is_valid()) return;

	// Determine the world-space from/to positions using the Node4D transform
	Ref<Transform4D> global_xform = get_global_transform_4d();
	if (!global_xform.is_valid()) return;

	// Ray origin is our global position
	Ref<Vector4D> origin_ref = global_xform->get_origin();
	if (!origin_ref.is_valid()) return;
	Vector4 from = Vector4(origin_ref->x, origin_ref->y, origin_ref->z, origin_ref->w);

	// Target is _target_position transformed into world space
	Ref<Vector4D> target_local;
	target_local.instantiate();
	target_local->x = _target_position.x;
	target_local->y = _target_position.y;
	target_local->z = _target_position.z;
	target_local->w = _target_position.w;

	Ref<Vector4D> target_world_ref = global_xform->xform(target_local);
	Vector4 to = Vector4(target_world_ref->x, target_world_ref->y, target_world_ref->z, target_world_ref->w);

	// Build exclude list - add parent if needed
	TypedArray<RID> exclude = _exclude_rids.duplicate();
	if (_exclude_parent) {
		Node *parent = get_parent();
		if (parent) {
			CollisionObject4D *co = Object::cast_to<CollisionObject4D>(parent);
			if (co) {
				RID parent_rid = co->get_rid();
				if (parent_rid.is_valid()) {
					exclude.push_back(parent_rid);
				}
			}
		}
	}

	Dictionary result = space_state->intersect_ray(
		from, to, exclude,
		_collision_mask,
		_collide_with_bodies,
		_collide_with_areas,
		_hit_from_inside
	);

	if (result.is_empty()) return;

	_is_colliding = true;

	if (result.has("position")) {
		_collision_point = result["position"];
	}
	if (result.has("normal")) {
		_collision_normal = result["normal"];
	}
	if (result.has("collider")) {
		_collider = Object::cast_to<Object>(result["collider"]);
	}
}

// ─── Notification / Process ───────────────────────────────────────────────────

void RayCast4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			set_physics_process(_enabled);
			break;
		case NOTIFICATION_EXIT_TREE:
			set_physics_process(false);
			_is_colliding = false;
			_collider = nullptr;
			break;
		default:
			break;
	}
}

void RayCast4D::_physics_process(double p_delta) {
	if (_enabled) {
		_perform_raycast();
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void RayCast4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &RayCast4D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &RayCast4D::is_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");

	ClassDB::bind_method(D_METHOD("set_target_position", "position"), &RayCast4D::set_target_position);
	ClassDB::bind_method(D_METHOD("get_target_position"), &RayCast4D::get_target_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "target_position"), "set_target_position", "get_target_position");

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &RayCast4D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &RayCast4D::get_collision_mask);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS),
		"set_collision_mask", "get_collision_mask");

	ClassDB::bind_method(D_METHOD("set_exclude_parent", "exclude"), &RayCast4D::set_exclude_parent);
	ClassDB::bind_method(D_METHOD("get_exclude_parent"), &RayCast4D::get_exclude_parent);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exclude_parent"), "set_exclude_parent", "get_exclude_parent");

	ClassDB::bind_method(D_METHOD("set_hit_from_inside", "enable"), &RayCast4D::set_hit_from_inside);
	ClassDB::bind_method(D_METHOD("get_hit_from_inside"), &RayCast4D::get_hit_from_inside);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "hit_from_inside"), "set_hit_from_inside", "get_hit_from_inside");

	ClassDB::bind_method(D_METHOD("set_collide_with_areas", "enable"), &RayCast4D::set_collide_with_areas);
	ClassDB::bind_method(D_METHOD("is_collide_with_areas_enabled"), &RayCast4D::is_collide_with_areas_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_areas"), "set_collide_with_areas", "is_collide_with_areas_enabled");

	ClassDB::bind_method(D_METHOD("set_collide_with_bodies", "enable"), &RayCast4D::set_collide_with_bodies);
	ClassDB::bind_method(D_METHOD("is_collide_with_bodies_enabled"), &RayCast4D::is_collide_with_bodies_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "collide_with_bodies"), "set_collide_with_bodies", "is_collide_with_bodies_enabled");

	// Result query methods
	ClassDB::bind_method(D_METHOD("is_colliding"), &RayCast4D::is_colliding);
	ClassDB::bind_method(D_METHOD("get_collider"), &RayCast4D::get_collider);
	ClassDB::bind_method(D_METHOD("get_collision_point"), &RayCast4D::get_collision_point);
	ClassDB::bind_method(D_METHOD("get_collision_normal"), &RayCast4D::get_collision_normal);

	// Control methods
	ClassDB::bind_method(D_METHOD("force_raycast_update"), &RayCast4D::force_raycast_update);
	ClassDB::bind_method(D_METHOD("add_exception", "node"), &RayCast4D::add_exception);
	ClassDB::bind_method(D_METHOD("remove_exception", "node"), &RayCast4D::remove_exception);
	ClassDB::bind_method(D_METHOD("clear_exceptions"), &RayCast4D::clear_exceptions);
}
