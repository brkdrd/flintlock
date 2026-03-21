#include "character_body_4d.h"
#include "../servers/physics/physics_server_4d.h"
#include "../servers/physics/core/space_4d_internal.h"
#include "../servers/physics/core/rigid_body_4d_internal.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

CharacterBody4D::CharacterBody4D() {}
CharacterBody4D::~CharacterBody4D() {}

void CharacterBody4D::_create_physics_body() {
	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (ps) {
		_rid = ps->body_create();
		ps->body_set_mode(_rid, PhysicsServer4D::BODY_MODE_KINEMATIC);
	}
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void CharacterBody4D::set_velocity(const Vector4 &p_velocity) {
	_velocity = p_velocity;
}
Vector4 CharacterBody4D::get_velocity() const { return _velocity; }

void CharacterBody4D::set_up_direction(const Vector4 &p_direction) {
	_up_direction = p_direction;
}
Vector4 CharacterBody4D::get_up_direction() const { return _up_direction; }

void CharacterBody4D::set_max_slides(int p_max_slides) {
	_max_slides = p_max_slides;
}
int CharacterBody4D::get_max_slides() const { return _max_slides; }

void CharacterBody4D::set_floor_max_angle(float p_angle) {
	_floor_max_angle = p_angle;
}
float CharacterBody4D::get_floor_max_angle() const { return _floor_max_angle; }

void CharacterBody4D::set_motion_mode(int p_mode) {
	_motion_mode = p_mode;
}
int CharacterBody4D::get_motion_mode() const { return _motion_mode; }

// ---------------------------------------------------------------------------
// Movement using physics server motion test
// ---------------------------------------------------------------------------

bool CharacterBody4D::move_and_slide() {
	double delta = get_physics_process_delta_time();

	_on_floor = false;
	_on_wall = false;
	_on_ceiling = false;

	PhysicsServer4D *ps = PhysicsServer4D::get_singleton();
	if (!ps || !_rid.is_valid()) {
		// Fallback: just translate
		Ref<Vector4D> offset = Vector4D::create(
			_velocity.x * (float)delta,
			_velocity.y * (float)delta,
			_velocity.z * (float)delta,
			_velocity.w * (float)delta
		);
		translate(offset);
		return false;
	}

	// Get the space the body belongs to
	RID space_rid = ps->body_get_space(_rid);
	Space4DInternal *space = ps->get_space_internal(space_rid);
	RigidBody4DInternal *body = ps->get_body_internal(_rid);

	if (!space || !body) {
		Ref<Vector4D> offset = Vector4D::create(
			_velocity.x * (float)delta,
			_velocity.y * (float)delta,
			_velocity.z * (float)delta,
			_velocity.w * (float)delta
		);
		translate(offset);
		return false;
	}

	Vector4 motion = _velocity * (float)delta;
	bool collided = false;
	float margin = 0.08f;

	for (int slide = 0; slide < _max_slides; slide++) {
		float motion_len = motion.length();
		if (motion_len < 1e-6f) break;

		Space4DInternal::MotionResult result = space->test_body_motion(body->id, motion, margin);

		if (!result.colliding) {
			// Move the full remaining distance
			Ref<Vector4D> offset = Vector4D::create(motion.x, motion.y, motion.z, motion.w);
			translate(offset);
			// Update body transform in server
			PackedFloat32Array xf = _get_transform_array();
			ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, xf);
			break;
		}

		collided = true;

		// Move to the safe position
		Ref<Vector4D> travel = Vector4D::create(
			result.travel.x, result.travel.y, result.travel.z, result.travel.w
		);
		translate(travel);
		// Update body transform in server
		PackedFloat32Array xf = _get_transform_array();
		ps->body_set_state(_rid, PhysicsServer4D::BODY_STATE_TRANSFORM, xf);

		// Classify collision
		Vector4 normal = result.collision_normal;
		float up_dot = normal.dot(_up_direction);

		if (_motion_mode == MOTION_MODE_GROUNDED) {
			float floor_cos = Math::cos(_floor_max_angle);
			if (up_dot > floor_cos) {
				_on_floor = true;
			} else if (up_dot < -floor_cos) {
				_on_ceiling = true;
			} else {
				_on_wall = true;
			}
		}

		// Slide: remove the component of motion along the collision normal
		motion = result.remainder;
		float normal_component = motion.dot(normal);
		if (normal_component < 0.0f) {
			motion = motion - normal * normal_component;
		}

		// Also slide velocity for floors/ceilings
		float vel_normal = _velocity.dot(normal);
		if (vel_normal < 0.0f) {
			_velocity = _velocity - normal * vel_normal;
		}
	}

	return collided;
}

bool CharacterBody4D::is_on_floor()   const { return _on_floor; }
bool CharacterBody4D::is_on_wall()    const { return _on_wall; }
bool CharacterBody4D::is_on_ceiling() const { return _on_ceiling; }

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

void CharacterBody4D::_bind_methods() {
	BIND_ENUM_CONSTANT(MOTION_MODE_GROUNDED);
	BIND_ENUM_CONSTANT(MOTION_MODE_FLOATING);

	ClassDB::bind_method(D_METHOD("set_velocity", "velocity"), &CharacterBody4D::set_velocity);
	ClassDB::bind_method(D_METHOD("get_velocity"), &CharacterBody4D::get_velocity);
	ClassDB::bind_method(D_METHOD("set_up_direction", "direction"), &CharacterBody4D::set_up_direction);
	ClassDB::bind_method(D_METHOD("get_up_direction"), &CharacterBody4D::get_up_direction);
	ClassDB::bind_method(D_METHOD("set_max_slides", "max_slides"), &CharacterBody4D::set_max_slides);
	ClassDB::bind_method(D_METHOD("get_max_slides"), &CharacterBody4D::get_max_slides);
	ClassDB::bind_method(D_METHOD("set_floor_max_angle", "angle"), &CharacterBody4D::set_floor_max_angle);
	ClassDB::bind_method(D_METHOD("get_floor_max_angle"), &CharacterBody4D::get_floor_max_angle);
	ClassDB::bind_method(D_METHOD("set_motion_mode", "mode"), &CharacterBody4D::set_motion_mode);
	ClassDB::bind_method(D_METHOD("get_motion_mode"), &CharacterBody4D::get_motion_mode);

	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "velocity"), "set_velocity", "get_velocity");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "up_direction"), "set_up_direction", "get_up_direction");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_slides"), "set_max_slides", "get_max_slides");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "floor_max_angle"), "set_floor_max_angle", "get_floor_max_angle");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "motion_mode", PROPERTY_HINT_ENUM, "Grounded,Floating"),
		"set_motion_mode", "get_motion_mode");

	ClassDB::bind_method(D_METHOD("move_and_slide"), &CharacterBody4D::move_and_slide);
	ClassDB::bind_method(D_METHOD("is_on_floor"), &CharacterBody4D::is_on_floor);
	ClassDB::bind_method(D_METHOD("is_on_wall"), &CharacterBody4D::is_on_wall);
	ClassDB::bind_method(D_METHOD("is_on_ceiling"), &CharacterBody4D::is_on_ceiling);
}
