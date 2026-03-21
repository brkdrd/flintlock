#include "pin_joint_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

PinJoint4D::PinJoint4D() {}
PinJoint4D::~PinJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void PinJoint4D::set_damping(float p_damping) {
	_damping = p_damping;
	if (_joint_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->joint_set_param(_joint_rid, "damping", p_damping);
	}
}

float PinJoint4D::get_damping() const {
	return _damping;
}

void PinJoint4D::set_impulse_clamp(float p_clamp) {
	_impulse_clamp = p_clamp;
	if (_joint_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->joint_set_param(_joint_rid, "impulse_clamp", p_clamp);
	}
}

float PinJoint4D::get_impulse_clamp() const {
	return _impulse_clamp;
}

void PinJoint4D::set_bias(float p_bias) {
	_bias = p_bias;
	if (_joint_rid.is_valid()) {
		PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
		if (ps) ps->joint_set_param(_joint_rid, "bias", p_bias);
	}
}

float PinJoint4D::get_bias() const {
	return _bias;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void PinJoint4D::_configure_joint() {
	NodePath node_a_path = get_node_a();
	NodePath node_b_path = get_node_b();

	if (node_a_path.is_empty() || node_b_path.is_empty()) return;

	Node *node_a = get_node_or_null(node_a_path);
	Node *node_b = get_node_or_null(node_b_path);
	if (!node_a || !node_b) return;

	CollisionObject4D *co_a = Object::cast_to<CollisionObject4D>(node_a);
	CollisionObject4D *co_b = Object::cast_to<CollisionObject4D>(node_b);
	if (!co_a || !co_b) return;

	RID body_a = co_a->get_rid();
	RID body_b = co_b->get_rid();
	if (!body_a.is_valid() || !body_b.is_valid()) return;

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps) return;

	_joint_rid = ps->joint_create(PhysicsServer4D::JOINT_TYPE_PIN, body_a, body_b);

	if (_joint_rid.is_valid()) {
		// Set anchors based on joint node position relative to each body
		Ref<Transform4D> joint_global = get_global_transform_4d();
		Ref<Transform4D> a_global = co_a->get_global_transform_4d();
		Ref<Transform4D> b_global = co_b->get_global_transform_4d();

		if (joint_global.is_valid()) {
			Ref<Vector4D> joint_origin = joint_global->get_origin();
			if (joint_origin.is_valid()) {
				Vector4 joint_pos = Vector4(joint_origin->x, joint_origin->y, joint_origin->z, joint_origin->w);

				// Compute anchor in body A's local space
				if (a_global.is_valid()) {
					Ref<Vector4D> a_origin = a_global->get_origin();
					if (a_origin.is_valid()) {
						Vector4 anchor_a = joint_pos - Vector4(a_origin->x, a_origin->y, a_origin->z, a_origin->w);
						ps->joint_set_anchor_a(_joint_rid, anchor_a);
					}
				}

				// Compute anchor in body B's local space
				if (b_global.is_valid()) {
					Ref<Vector4D> b_origin = b_global->get_origin();
					if (b_origin.is_valid()) {
						Vector4 anchor_b = joint_pos - Vector4(b_origin->x, b_origin->y, b_origin->z, b_origin->w);
						ps->joint_set_anchor_b(_joint_rid, anchor_b);
					}
				}
			}
		}

		ps->joint_set_param(_joint_rid, "damping", _damping);
		ps->joint_set_param(_joint_rid, "impulse_clamp", _impulse_clamp);
		ps->joint_set_param(_joint_rid, "bias", _bias);
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void PinJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_damping", "damping"), &PinJoint4D::set_damping);
	ClassDB::bind_method(D_METHOD("get_damping"), &PinJoint4D::get_damping);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "damping", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater"),
		"set_damping", "get_damping");

	ClassDB::bind_method(D_METHOD("set_impulse_clamp", "clamp"), &PinJoint4D::set_impulse_clamp);
	ClassDB::bind_method(D_METHOD("get_impulse_clamp"), &PinJoint4D::get_impulse_clamp);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "impulse_clamp", PROPERTY_HINT_RANGE, "0,64,0.001,or_greater"),
		"set_impulse_clamp", "get_impulse_clamp");

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &PinJoint4D::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &PinJoint4D::get_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bias", PROPERTY_HINT_RANGE, "0.01,0.99,0.001"),
		"set_bias", "get_bias");
}
