#include "collision_object_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

CollisionObject4D::CollisionObject4D() {}
CollisionObject4D::~CollisionObject4D() {}

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

PackedFloat32Array CollisionObject4D::_get_transform_array() const {
	// Build 20-float array: 16 basis floats (column-major 4x4) + 4 origin floats.
	PackedFloat32Array arr;
	arr.resize(20);

	Ref<Transform4D> xform = get_global_transform_4d();
	if (xform.is_null()) {
		// Identity: columns 0-3 are identity, origin is zero
		for (int i = 0; i < 20; i++) arr.set(i, (i % 5 == 0 && i < 16) ? 1.0f : 0.0f);
		return arr;
	}

	Ref<Basis4D> basis = xform->get_basis();
	if (basis.is_valid()) {
		// Column-major: col 0 -> arr[0..3], col 1 -> arr[4..7], ...
		for (int col = 0; col < 4; col++) {
			arr.set(col * 4 + 0, basis->data[col][0]);
			arr.set(col * 4 + 1, basis->data[col][1]);
			arr.set(col * 4 + 2, basis->data[col][2]);
			arr.set(col * 4 + 3, basis->data[col][3]);
		}
	} else {
		// Identity basis
		for (int col = 0; col < 4; col++) {
			for (int row = 0; row < 4; row++) {
				arr.set(col * 4 + row, (col == row) ? 1.0f : 0.0f);
			}
		}
	}

	Ref<Vector4D> origin = xform->get_origin();
	if (origin.is_valid()) {
		arr.set(16, origin->x);
		arr.set(17, origin->y);
		arr.set(18, origin->z);
		arr.set(19, origin->w);
	} else {
		arr.set(16, 0.0f);
		arr.set(17, 0.0f);
		arr.set(18, 0.0f);
		arr.set(19, 0.0f);
	}

	return arr;
}

// ---------------------------------------------------------------------------
// Notifications
// ---------------------------------------------------------------------------

void CollisionObject4D::_notification(int p_what) {
	// Always propagate to Node4D first.
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Let the subclass create the physics object (body or area).
			_create_physics_body();

			if (_rid.is_valid()) {
				PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
				if (ps) {
					// Assign to the default space
					RID default_space = ps->get_default_space();
					ps->body_set_space(_rid, default_space);

					// Register the instance ID so the server can call back to us.
					ps->body_set_object_instance_id(_rid, get_instance_id());

					// Set collision layer and mask
					ps->body_set_collision_layer(_rid, _collision_layer);
					ps->body_set_collision_mask(_rid, _collision_mask);

					// Push current transform.
					PackedFloat32Array xf = _get_transform_array();
					ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, xf);
				}
			}

			// Opt in to transform change notifications.
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
					ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, xf);
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

void CollisionObject4D::set_collision_layer(uint32_t p_layer) {
	_collision_layer = p_layer;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_collision_layer(_rid, p_layer);
	}
}
uint32_t CollisionObject4D::get_collision_layer() const {
	return _collision_layer;
}

void CollisionObject4D::set_collision_mask(uint32_t p_mask) {
	_collision_mask = p_mask;
	if (_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->body_set_collision_mask(_rid, p_mask);
	}
}
uint32_t CollisionObject4D::get_collision_mask() const {
	return _collision_mask;
}

void CollisionObject4D::set_collision_priority(float p_priority) {
	_collision_priority = p_priority;
}
float CollisionObject4D::get_collision_priority() const {
	return _collision_priority;
}

// ---------------------------------------------------------------------------
// Shape owner helpers
// ---------------------------------------------------------------------------

void CollisionObject4D::shape_owner_add_shape(const RID &p_shape) {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		ps->body_add_shape(_rid, p_shape);
	}
}

int CollisionObject4D::shape_owner_get_count() const {
	if (!_rid.is_valid()) return 0;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		return ps->body_get_shape_count(_rid);
	}
	return 0;
}

void CollisionObject4D::shape_owner_clear() {
	if (!_rid.is_valid()) return;
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		ps->body_clear_shapes(_rid);
	}
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void CollisionObject4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &CollisionObject4D::get_rid);

	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &CollisionObject4D::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &CollisionObject4D::get_collision_layer);
	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &CollisionObject4D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &CollisionObject4D::get_collision_mask);
	ClassDB::bind_method(D_METHOD("set_collision_priority", "priority"), &CollisionObject4D::set_collision_priority);
	ClassDB::bind_method(D_METHOD("get_collision_priority"), &CollisionObject4D::get_collision_priority);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer"), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask"), "set_collision_mask", "get_collision_mask");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"), "set_collision_priority", "get_collision_priority");

	ClassDB::bind_method(D_METHOD("shape_owner_add_shape", "shape"), &CollisionObject4D::shape_owner_add_shape);
	ClassDB::bind_method(D_METHOD("shape_owner_get_count"), &CollisionObject4D::shape_owner_get_count);
	ClassDB::bind_method(D_METHOD("shape_owner_clear"), &CollisionObject4D::shape_owner_clear);
}
