#include "generic10dof_joint_4d.h"
#include "collision_object_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Generic10DOFJoint4D::Generic10DOFJoint4D() {}
Generic10DOFJoint4D::~Generic10DOFJoint4D() {}

// ─── Linear X ─────────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_linear_x_limit_enabled(bool p_enabled) { _linear_x.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_x_limit_enabled() const { return _linear_x.limit_enabled; }
void Generic10DOFJoint4D::set_linear_x_limit_upper(float p_upper) { _linear_x.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_linear_x_limit_upper() const { return _linear_x.limit_upper; }
void Generic10DOFJoint4D::set_linear_x_limit_lower(float p_lower) { _linear_x.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_linear_x_limit_lower() const { return _linear_x.limit_lower; }
void Generic10DOFJoint4D::set_linear_x_motor_enabled(bool p_enabled) { _linear_x.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_x_motor_enabled() const { return _linear_x.motor_enabled; }

// ─── Linear Y ─────────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_linear_y_limit_enabled(bool p_enabled) { _linear_y.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_y_limit_enabled() const { return _linear_y.limit_enabled; }
void Generic10DOFJoint4D::set_linear_y_limit_upper(float p_upper) { _linear_y.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_linear_y_limit_upper() const { return _linear_y.limit_upper; }
void Generic10DOFJoint4D::set_linear_y_limit_lower(float p_lower) { _linear_y.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_linear_y_limit_lower() const { return _linear_y.limit_lower; }
void Generic10DOFJoint4D::set_linear_y_motor_enabled(bool p_enabled) { _linear_y.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_y_motor_enabled() const { return _linear_y.motor_enabled; }

// ─── Linear Z ─────────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_linear_z_limit_enabled(bool p_enabled) { _linear_z.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_z_limit_enabled() const { return _linear_z.limit_enabled; }
void Generic10DOFJoint4D::set_linear_z_limit_upper(float p_upper) { _linear_z.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_linear_z_limit_upper() const { return _linear_z.limit_upper; }
void Generic10DOFJoint4D::set_linear_z_limit_lower(float p_lower) { _linear_z.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_linear_z_limit_lower() const { return _linear_z.limit_lower; }
void Generic10DOFJoint4D::set_linear_z_motor_enabled(bool p_enabled) { _linear_z.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_z_motor_enabled() const { return _linear_z.motor_enabled; }

// ─── Linear W ─────────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_linear_w_limit_enabled(bool p_enabled) { _linear_w.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_w_limit_enabled() const { return _linear_w.limit_enabled; }
void Generic10DOFJoint4D::set_linear_w_limit_upper(float p_upper) { _linear_w.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_linear_w_limit_upper() const { return _linear_w.limit_upper; }
void Generic10DOFJoint4D::set_linear_w_limit_lower(float p_lower) { _linear_w.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_linear_w_limit_lower() const { return _linear_w.limit_lower; }
void Generic10DOFJoint4D::set_linear_w_motor_enabled(bool p_enabled) { _linear_w.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_linear_w_motor_enabled() const { return _linear_w.motor_enabled; }

// ─── Angular XY ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_xy_limit_enabled(bool p_enabled) { _angular_xy.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xy_limit_enabled() const { return _angular_xy.limit_enabled; }
void Generic10DOFJoint4D::set_angular_xy_limit_upper(float p_upper) { _angular_xy.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_xy_limit_upper() const { return _angular_xy.limit_upper; }
void Generic10DOFJoint4D::set_angular_xy_limit_lower(float p_lower) { _angular_xy.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_xy_limit_lower() const { return _angular_xy.limit_lower; }
void Generic10DOFJoint4D::set_angular_xy_motor_enabled(bool p_enabled) { _angular_xy.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xy_motor_enabled() const { return _angular_xy.motor_enabled; }

// ─── Angular XZ ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_xz_limit_enabled(bool p_enabled) { _angular_xz.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xz_limit_enabled() const { return _angular_xz.limit_enabled; }
void Generic10DOFJoint4D::set_angular_xz_limit_upper(float p_upper) { _angular_xz.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_xz_limit_upper() const { return _angular_xz.limit_upper; }
void Generic10DOFJoint4D::set_angular_xz_limit_lower(float p_lower) { _angular_xz.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_xz_limit_lower() const { return _angular_xz.limit_lower; }
void Generic10DOFJoint4D::set_angular_xz_motor_enabled(bool p_enabled) { _angular_xz.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xz_motor_enabled() const { return _angular_xz.motor_enabled; }

// ─── Angular XW ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_xw_limit_enabled(bool p_enabled) { _angular_xw.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xw_limit_enabled() const { return _angular_xw.limit_enabled; }
void Generic10DOFJoint4D::set_angular_xw_limit_upper(float p_upper) { _angular_xw.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_xw_limit_upper() const { return _angular_xw.limit_upper; }
void Generic10DOFJoint4D::set_angular_xw_limit_lower(float p_lower) { _angular_xw.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_xw_limit_lower() const { return _angular_xw.limit_lower; }
void Generic10DOFJoint4D::set_angular_xw_motor_enabled(bool p_enabled) { _angular_xw.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_xw_motor_enabled() const { return _angular_xw.motor_enabled; }

// ─── Angular YZ ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_yz_limit_enabled(bool p_enabled) { _angular_yz.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_yz_limit_enabled() const { return _angular_yz.limit_enabled; }
void Generic10DOFJoint4D::set_angular_yz_limit_upper(float p_upper) { _angular_yz.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_yz_limit_upper() const { return _angular_yz.limit_upper; }
void Generic10DOFJoint4D::set_angular_yz_limit_lower(float p_lower) { _angular_yz.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_yz_limit_lower() const { return _angular_yz.limit_lower; }
void Generic10DOFJoint4D::set_angular_yz_motor_enabled(bool p_enabled) { _angular_yz.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_yz_motor_enabled() const { return _angular_yz.motor_enabled; }

// ─── Angular YW ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_yw_limit_enabled(bool p_enabled) { _angular_yw.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_yw_limit_enabled() const { return _angular_yw.limit_enabled; }
void Generic10DOFJoint4D::set_angular_yw_limit_upper(float p_upper) { _angular_yw.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_yw_limit_upper() const { return _angular_yw.limit_upper; }
void Generic10DOFJoint4D::set_angular_yw_limit_lower(float p_lower) { _angular_yw.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_yw_limit_lower() const { return _angular_yw.limit_lower; }
void Generic10DOFJoint4D::set_angular_yw_motor_enabled(bool p_enabled) { _angular_yw.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_yw_motor_enabled() const { return _angular_yw.motor_enabled; }

// ─── Angular ZW ───────────────────────────────────────────────────────────────

void Generic10DOFJoint4D::set_angular_zw_limit_enabled(bool p_enabled) { _angular_zw.limit_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_zw_limit_enabled() const { return _angular_zw.limit_enabled; }
void Generic10DOFJoint4D::set_angular_zw_limit_upper(float p_upper) { _angular_zw.limit_upper = p_upper; }
float Generic10DOFJoint4D::get_angular_zw_limit_upper() const { return _angular_zw.limit_upper; }
void Generic10DOFJoint4D::set_angular_zw_limit_lower(float p_lower) { _angular_zw.limit_lower = p_lower; }
float Generic10DOFJoint4D::get_angular_zw_limit_lower() const { return _angular_zw.limit_lower; }
void Generic10DOFJoint4D::set_angular_zw_motor_enabled(bool p_enabled) { _angular_zw.motor_enabled = p_enabled; }
bool Generic10DOFJoint4D::is_angular_zw_motor_enabled() const { return _angular_zw.motor_enabled; }

// ─── Joint Configuration ──────────────────────────────────────────────────────

void Generic10DOFJoint4D::_configure_joint() {
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

	_joint_rid = ps->joint_create(PhysicsServer4D::JOINT_TYPE_GENERIC_10DOF, body_a, body_b);

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

void Generic10DOFJoint4D::_bind_methods() {
	// Helpers for property hint ranges
	const char *linear_range = "-1024,1024,0.001,or_lesser,or_greater";
	const char *angular_range = "-6.2832,6.2832,0.001,radians";

	// ── Linear X ────────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_linear_x_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_x_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_x_limit_enabled"), &Generic10DOFJoint4D::is_linear_x_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_x/limit_enabled"), "set_linear_x_limit_enabled", "is_linear_x_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_linear_x_limit_upper", "upper"), &Generic10DOFJoint4D::set_linear_x_limit_upper);
	ClassDB::bind_method(D_METHOD("get_linear_x_limit_upper"), &Generic10DOFJoint4D::get_linear_x_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_x/limit_upper", PROPERTY_HINT_RANGE, linear_range), "set_linear_x_limit_upper", "get_linear_x_limit_upper");

	ClassDB::bind_method(D_METHOD("set_linear_x_limit_lower", "lower"), &Generic10DOFJoint4D::set_linear_x_limit_lower);
	ClassDB::bind_method(D_METHOD("get_linear_x_limit_lower"), &Generic10DOFJoint4D::get_linear_x_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_x/limit_lower", PROPERTY_HINT_RANGE, linear_range), "set_linear_x_limit_lower", "get_linear_x_limit_lower");

	ClassDB::bind_method(D_METHOD("set_linear_x_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_x_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_x_motor_enabled"), &Generic10DOFJoint4D::is_linear_x_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_x/motor_enabled"), "set_linear_x_motor_enabled", "is_linear_x_motor_enabled");

	// ── Linear Y ────────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_linear_y_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_y_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_y_limit_enabled"), &Generic10DOFJoint4D::is_linear_y_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_y/limit_enabled"), "set_linear_y_limit_enabled", "is_linear_y_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_linear_y_limit_upper", "upper"), &Generic10DOFJoint4D::set_linear_y_limit_upper);
	ClassDB::bind_method(D_METHOD("get_linear_y_limit_upper"), &Generic10DOFJoint4D::get_linear_y_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_y/limit_upper", PROPERTY_HINT_RANGE, linear_range), "set_linear_y_limit_upper", "get_linear_y_limit_upper");

	ClassDB::bind_method(D_METHOD("set_linear_y_limit_lower", "lower"), &Generic10DOFJoint4D::set_linear_y_limit_lower);
	ClassDB::bind_method(D_METHOD("get_linear_y_limit_lower"), &Generic10DOFJoint4D::get_linear_y_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_y/limit_lower", PROPERTY_HINT_RANGE, linear_range), "set_linear_y_limit_lower", "get_linear_y_limit_lower");

	ClassDB::bind_method(D_METHOD("set_linear_y_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_y_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_y_motor_enabled"), &Generic10DOFJoint4D::is_linear_y_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_y/motor_enabled"), "set_linear_y_motor_enabled", "is_linear_y_motor_enabled");

	// ── Linear Z ────────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_linear_z_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_z_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_z_limit_enabled"), &Generic10DOFJoint4D::is_linear_z_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_z/limit_enabled"), "set_linear_z_limit_enabled", "is_linear_z_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_linear_z_limit_upper", "upper"), &Generic10DOFJoint4D::set_linear_z_limit_upper);
	ClassDB::bind_method(D_METHOD("get_linear_z_limit_upper"), &Generic10DOFJoint4D::get_linear_z_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_z/limit_upper", PROPERTY_HINT_RANGE, linear_range), "set_linear_z_limit_upper", "get_linear_z_limit_upper");

	ClassDB::bind_method(D_METHOD("set_linear_z_limit_lower", "lower"), &Generic10DOFJoint4D::set_linear_z_limit_lower);
	ClassDB::bind_method(D_METHOD("get_linear_z_limit_lower"), &Generic10DOFJoint4D::get_linear_z_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_z/limit_lower", PROPERTY_HINT_RANGE, linear_range), "set_linear_z_limit_lower", "get_linear_z_limit_lower");

	ClassDB::bind_method(D_METHOD("set_linear_z_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_z_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_z_motor_enabled"), &Generic10DOFJoint4D::is_linear_z_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_z/motor_enabled"), "set_linear_z_motor_enabled", "is_linear_z_motor_enabled");

	// ── Linear W ────────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_linear_w_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_w_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_w_limit_enabled"), &Generic10DOFJoint4D::is_linear_w_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_w/limit_enabled"), "set_linear_w_limit_enabled", "is_linear_w_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_linear_w_limit_upper", "upper"), &Generic10DOFJoint4D::set_linear_w_limit_upper);
	ClassDB::bind_method(D_METHOD("get_linear_w_limit_upper"), &Generic10DOFJoint4D::get_linear_w_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_w/limit_upper", PROPERTY_HINT_RANGE, linear_range), "set_linear_w_limit_upper", "get_linear_w_limit_upper");

	ClassDB::bind_method(D_METHOD("set_linear_w_limit_lower", "lower"), &Generic10DOFJoint4D::set_linear_w_limit_lower);
	ClassDB::bind_method(D_METHOD("get_linear_w_limit_lower"), &Generic10DOFJoint4D::get_linear_w_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "linear_w/limit_lower", PROPERTY_HINT_RANGE, linear_range), "set_linear_w_limit_lower", "get_linear_w_limit_lower");

	ClassDB::bind_method(D_METHOD("set_linear_w_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_linear_w_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_linear_w_motor_enabled"), &Generic10DOFJoint4D::is_linear_w_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "linear_w/motor_enabled"), "set_linear_w_motor_enabled", "is_linear_w_motor_enabled");

	// ── Angular XY ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_xy_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xy_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xy_limit_enabled"), &Generic10DOFJoint4D::is_angular_xy_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xy/limit_enabled"), "set_angular_xy_limit_enabled", "is_angular_xy_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_xy_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_xy_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_xy_limit_upper"), &Generic10DOFJoint4D::get_angular_xy_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xy/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_xy_limit_upper", "get_angular_xy_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_xy_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_xy_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_xy_limit_lower"), &Generic10DOFJoint4D::get_angular_xy_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xy/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_xy_limit_lower", "get_angular_xy_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_xy_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xy_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xy_motor_enabled"), &Generic10DOFJoint4D::is_angular_xy_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xy/motor_enabled"), "set_angular_xy_motor_enabled", "is_angular_xy_motor_enabled");

	// ── Angular XZ ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_xz_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xz_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xz_limit_enabled"), &Generic10DOFJoint4D::is_angular_xz_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xz/limit_enabled"), "set_angular_xz_limit_enabled", "is_angular_xz_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_xz_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_xz_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_xz_limit_upper"), &Generic10DOFJoint4D::get_angular_xz_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xz/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_xz_limit_upper", "get_angular_xz_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_xz_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_xz_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_xz_limit_lower"), &Generic10DOFJoint4D::get_angular_xz_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xz/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_xz_limit_lower", "get_angular_xz_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_xz_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xz_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xz_motor_enabled"), &Generic10DOFJoint4D::is_angular_xz_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xz/motor_enabled"), "set_angular_xz_motor_enabled", "is_angular_xz_motor_enabled");

	// ── Angular XW ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_xw_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xw_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xw_limit_enabled"), &Generic10DOFJoint4D::is_angular_xw_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xw/limit_enabled"), "set_angular_xw_limit_enabled", "is_angular_xw_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_xw_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_xw_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_xw_limit_upper"), &Generic10DOFJoint4D::get_angular_xw_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xw/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_xw_limit_upper", "get_angular_xw_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_xw_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_xw_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_xw_limit_lower"), &Generic10DOFJoint4D::get_angular_xw_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_xw/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_xw_limit_lower", "get_angular_xw_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_xw_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_xw_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_xw_motor_enabled"), &Generic10DOFJoint4D::is_angular_xw_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_xw/motor_enabled"), "set_angular_xw_motor_enabled", "is_angular_xw_motor_enabled");

	// ── Angular YZ ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_yz_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_yz_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_yz_limit_enabled"), &Generic10DOFJoint4D::is_angular_yz_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_yz/limit_enabled"), "set_angular_yz_limit_enabled", "is_angular_yz_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_yz_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_yz_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_yz_limit_upper"), &Generic10DOFJoint4D::get_angular_yz_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_yz/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_yz_limit_upper", "get_angular_yz_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_yz_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_yz_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_yz_limit_lower"), &Generic10DOFJoint4D::get_angular_yz_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_yz/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_yz_limit_lower", "get_angular_yz_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_yz_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_yz_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_yz_motor_enabled"), &Generic10DOFJoint4D::is_angular_yz_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_yz/motor_enabled"), "set_angular_yz_motor_enabled", "is_angular_yz_motor_enabled");

	// ── Angular YW ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_yw_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_yw_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_yw_limit_enabled"), &Generic10DOFJoint4D::is_angular_yw_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_yw/limit_enabled"), "set_angular_yw_limit_enabled", "is_angular_yw_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_yw_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_yw_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_yw_limit_upper"), &Generic10DOFJoint4D::get_angular_yw_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_yw/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_yw_limit_upper", "get_angular_yw_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_yw_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_yw_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_yw_limit_lower"), &Generic10DOFJoint4D::get_angular_yw_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_yw/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_yw_limit_lower", "get_angular_yw_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_yw_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_yw_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_yw_motor_enabled"), &Generic10DOFJoint4D::is_angular_yw_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_yw/motor_enabled"), "set_angular_yw_motor_enabled", "is_angular_yw_motor_enabled");

	// ── Angular ZW ──────────────────────────────────────────────────────────
	ClassDB::bind_method(D_METHOD("set_angular_zw_limit_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_zw_limit_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_zw_limit_enabled"), &Generic10DOFJoint4D::is_angular_zw_limit_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_zw/limit_enabled"), "set_angular_zw_limit_enabled", "is_angular_zw_limit_enabled");

	ClassDB::bind_method(D_METHOD("set_angular_zw_limit_upper", "upper"), &Generic10DOFJoint4D::set_angular_zw_limit_upper);
	ClassDB::bind_method(D_METHOD("get_angular_zw_limit_upper"), &Generic10DOFJoint4D::get_angular_zw_limit_upper);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_zw/limit_upper", PROPERTY_HINT_RANGE, angular_range), "set_angular_zw_limit_upper", "get_angular_zw_limit_upper");

	ClassDB::bind_method(D_METHOD("set_angular_zw_limit_lower", "lower"), &Generic10DOFJoint4D::set_angular_zw_limit_lower);
	ClassDB::bind_method(D_METHOD("get_angular_zw_limit_lower"), &Generic10DOFJoint4D::get_angular_zw_limit_lower);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "angular_zw/limit_lower", PROPERTY_HINT_RANGE, angular_range), "set_angular_zw_limit_lower", "get_angular_zw_limit_lower");

	ClassDB::bind_method(D_METHOD("set_angular_zw_motor_enabled", "enabled"), &Generic10DOFJoint4D::set_angular_zw_motor_enabled);
	ClassDB::bind_method(D_METHOD("is_angular_zw_motor_enabled"), &Generic10DOFJoint4D::is_angular_zw_motor_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "angular_zw/motor_enabled"), "set_angular_zw_motor_enabled", "is_angular_zw_motor_enabled");
}
