#include "slider_joint_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

SliderJoint4D::SliderJoint4D() {}
SliderJoint4D::~SliderJoint4D() {}

// ─── Properties ───────────────────────────────────────────────────────────────

void SliderJoint4D::set_linear_limit_upper(float p_upper) {
	_linear_limit_upper = p_upper;
}

float SliderJoint4D::get_linear_limit_upper() const {
	return _linear_limit_upper;
}

void SliderJoint4D::set_linear_limit_lower(float p_lower) {
	_linear_limit_lower = p_lower;
}

float SliderJoint4D::get_linear_limit_lower() const {
	return _linear_limit_lower;
}

void SliderJoint4D::set_linear_softness(float p_softness) {
	_linear_softness = p_softness;
}

float SliderJoint4D::get_linear_softness() const {
	return _linear_softness;
}

void SliderJoint4D::set_linear_restitution(float p_restitution) {
	_linear_restitution = p_restitution;
}

float SliderJoint4D::get_linear_restitution() const {
	return _linear_restitution;
}

void SliderJoint4D::set_linear_damping(float p_damping) {
	_linear_damping = p_damping;
}

float SliderJoint4D::get_linear_damping() const {
	return _linear_damping;
}

void SliderJoint4D::set_motor_linear_enabled(bool p_enabled) {
	_motor_linear_enabled = p_enabled;
}

bool SliderJoint4D::is_motor_linear_enabled() const {
	return _motor_linear_enabled;
}

// ─── Joint Configuration ──────────────────────────────────────────────────────

void SliderJoint4D::_configure_joint() {
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

	_joint_rid = ps->joint_create(PhysicsServer4D::JOINT_TYPE_SLIDER, body_a, body_b);

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

void SliderJoint4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_linear_limit_upper", "upper"), &SliderJoint4D::set_linear_limit_upper);
	ClassDB::bind_method(D_METHOD("get_linear_limit_upper"), &SliderJoint4D::get_linear_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_limit_upper", PROPERTY_HINT_RANGE, "-1024,1024,0.001,or_lesser,or_greater"),
		"set_linear_limit_upper", "get_linear_limit_upper");

	ClassDB::bind_method(D_METHOD("set_linear_limit_lower", "lower"), &SliderJoint4D::set_linear_limit_lower);
	ClassDB::bind_method(D_METHOD("get_linear_limit_lower"), &SliderJoint4D::get_linear_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_limit_lower", PROPERTY_HINT_RANGE, "-1024,1024,0.001,or_lesser,or_greater"),
		"set_linear_limit_lower", "get_linear_limit_lower");

	ClassDB::bind_method(D_METHOD("set_linear_softness", "softness"), &SliderJoint4D::set_linear_softness);
	ClassDB::bind_method(D_METHOD("get_linear_softness"), &SliderJoint4D::get_linear_softness);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_softness", PROPERTY_HINT_RANGE, "0,1,0.001"),
		"set_linear_softness", "get_linear_softness");

	ClassDB::bind_method(D_METHOD("set_linear_restitution", "restitution"), &SliderJoint4D::set_linear_restitution);
	ClassDB::bind_method(D_METHOD("get_linear_restitution"), &SliderJoint4D::get_linear_restitution);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_restitution", PROPERTY_HINT_RANGE, "0,1,0.001"),
		"set_linear_restitution", "get_linear_restitution");

	ClassDB::bind_method(D_METHOD("set_linear_damping", "damping"), &SliderJoint4D::set_linear_damping);
	ClassDB::bind_method(D_METHOD("get_linear_damping"), &SliderJoint4D::get_linear_damping);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_damping", PROPERTY_HINT_RANGE, "0,1,0.001,or_greater"),
		"set_linear_damping", "get_linear_damping");

	ClassDB::bind_method(D_METHOD("set_motor_linear_enabled", "enabled"), &SliderJoint4D::set_motor_linear_enabled);
	ClassDB::bind_method(D_METHOD("is_motor_linear_enabled"), &SliderJoint4D::is_motor_linear_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "motor_linear_enabled"),
		"set_motor_linear_enabled", "is_motor_linear_enabled");
}
