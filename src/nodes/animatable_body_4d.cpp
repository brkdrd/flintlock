#include "animatable_body_4d.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

AnimatableBody4D::AnimatableBody4D() {}
AnimatableBody4D::~AnimatableBody4D() {}

void AnimatableBody4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		_rid = ps->body_create();
	}
}

void AnimatableBody4D::_configure_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_rid.is_valid()) return;
	ps->body_set_mode(_rid, PhysicsServer4D::BODY_MODE_KINEMATIC);
}

void AnimatableBody4D::set_sync_to_physics(bool p_sync) {
	_sync_to_physics = p_sync;
}

bool AnimatableBody4D::get_sync_to_physics() const {
	return _sync_to_physics;
}

void AnimatableBody4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_sync_to_physics", "sync"), &AnimatableBody4D::set_sync_to_physics);
	ClassDB::bind_method(D_METHOD("get_sync_to_physics"), &AnimatableBody4D::get_sync_to_physics);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "sync_to_physics"), "set_sync_to_physics", "get_sync_to_physics");
}
