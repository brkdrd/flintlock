#include "cone_twist_joint_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

ConeTwistJoint4D::ConeTwistJoint4D() {}
ConeTwistJoint4D::~ConeTwistJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void ConeTwistJoint4D::set_swing_span(float p_swing_span) {
	_swing_span = p_swing_span;
}

float ConeTwistJoint4D::get_swing_span() const {
	return _swing_span;
}

void ConeTwistJoint4D::set_twist_span(float p_twist_span) {
	_twist_span = p_twist_span;
}

float ConeTwistJoint4D::get_twist_span() const {
	return _twist_span;
}

void ConeTwistJoint4D::set_bias(float p_bias) {
	_bias = p_bias;
}

float ConeTwistJoint4D::get_bias() const {
	return _bias;
}

void ConeTwistJoint4D::set_softness(float p_softness) {
	_softness = p_softness;
}

float ConeTwistJoint4D::get_softness() const {
	return _softness;
}

void ConeTwistJoint4D::set_relaxation(float p_relaxation) {
	_relaxation = p_relaxation;
}

float ConeTwistJoint4D::get_relaxation() const {
	return _relaxation;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void ConeTwistJoint4D::_configure_joint() {
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

	_joint_rid = ps->joint_create(PhysicsServer4D::JOINT_TYPE_CONE_TWIST, body_a, body_b);

	if (_joint_rid.is_valid()) {
		Ref<Transform4D> joint_global = get_global_transform_4d();
		if (joint_global.is_valid()) {
			Ref<Vector4D> joint_origin = joint_global->get_origin();
			if (joint_origin.is_valid()) {
				Vector4 joint_pos = Vector4(joint_origin->x, joint_origin->y, joint_origin->z, joint_origin->w);

				Ref<Transform4D> a_global = co_a->get_global_transform_4d();
				if (a_global.is_valid()) {
					Ref<Vector4D> a_origin = a_global->get_origin();
					if (a_origin.is_valid()) {
						ps->joint_set_anchor_a(_joint_rid, joint_pos - Vector4(a_origin->x, a_origin->y, a_origin->z, a_origin->w));
					}
				}

				Ref<Transform4D> b_global = co_b->get_global_transform_4d();
				if (b_global.is_valid()) {
					Ref<Vector4D> b_origin = b_global->get_origin();
					if (b_origin.is_valid()) {
						ps->joint_set_anchor_b(_joint_rid, joint_pos - Vector4(b_origin->x, b_origin->y, b_origin->z, b_origin->w));
					}
				}
			}
		}
	}
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void ConeTwistJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_swing_span", "swing_span"), &ConeTwistJoint4D::set_swing_span);
	ClassDB::bind_method(D_METHOD("get_swing_span"), &ConeTwistJoint4D::get_swing_span);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "swing_span", PROPERTY_HINT_RANGE, "0,6.2832,0.001,radians"),
		"set_swing_span", "get_swing_span");

	ClassDB::bind_method(D_METHOD("set_twist_span", "twist_span"), &ConeTwistJoint4D::set_twist_span);
	ClassDB::bind_method(D_METHOD("get_twist_span"), &ConeTwistJoint4D::get_twist_span);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "twist_span", PROPERTY_HINT_RANGE, "0,6.2832,0.001,radians"),
		"set_twist_span", "get_twist_span");

	ClassDB::bind_method(D_METHOD("set_bias", "bias"), &ConeTwistJoint4D::set_bias);
	ClassDB::bind_method(D_METHOD("get_bias"), &ConeTwistJoint4D::get_bias);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "bias", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_bias", "get_bias");

	ClassDB::bind_method(D_METHOD("set_softness", "softness"), &ConeTwistJoint4D::set_softness);
	ClassDB::bind_method(D_METHOD("get_softness"), &ConeTwistJoint4D::get_softness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "softness", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_softness", "get_softness");

	ClassDB::bind_method(D_METHOD("set_relaxation", "relaxation"), &ConeTwistJoint4D::set_relaxation);
	ClassDB::bind_method(D_METHOD("get_relaxation"), &ConeTwistJoint4D::get_relaxation);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "relaxation", PROPERTY_HINT_RANGE, "0.01,1,0.001"),
		"set_relaxation", "get_relaxation");
}
