#include "collision_object_4d.h"

using namespace godot;

CollisionObject4D::CollisionObject4D() {}
CollisionObject4D::~CollisionObject4D() {}

RID CollisionObject4D::get_rid() const {
	return base_rid;
}

void CollisionObject4D::set_collision_layer(uint32_t p_layer) {
	collision_layer = p_layer;
}

uint32_t CollisionObject4D::get_collision_layer() const {
	return collision_layer;
}

void CollisionObject4D::set_collision_mask(uint32_t p_mask) {
	collision_mask = p_mask;
}

uint32_t CollisionObject4D::get_collision_mask() const {
	return collision_mask;
}

void CollisionObject4D::set_collision_layer_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32,
			"Collision layer number must be between 1 and 32.");
	uint32_t mask = 1u << (p_layer_number - 1);
	if (p_value) {
		collision_layer |= mask;
	} else {
		collision_layer &= ~mask;
	}
}

bool CollisionObject4D::get_collision_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false,
			"Collision layer number must be between 1 and 32.");
	return (collision_layer >> (p_layer_number - 1)) & 1;
}

void CollisionObject4D::set_collision_mask_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1 || p_layer_number > 32,
			"Collision layer number must be between 1 and 32.");
	uint32_t mask = 1u << (p_layer_number - 1);
	if (p_value) {
		collision_mask |= mask;
	} else {
		collision_mask &= ~mask;
	}
}

bool CollisionObject4D::get_collision_mask_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1 || p_layer_number > 32, false,
			"Collision layer number must be between 1 and 32.");
	return (collision_mask >> (p_layer_number - 1)) & 1;
}

void CollisionObject4D::set_collision_priority(real_t p_priority) {
	collision_priority = p_priority;
}

real_t CollisionObject4D::get_collision_priority() const {
	return collision_priority;
}

void CollisionObject4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &CollisionObject4D::get_rid);

	ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &CollisionObject4D::set_collision_layer);
	ClassDB::bind_method(D_METHOD("get_collision_layer"), &CollisionObject4D::get_collision_layer);

	ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &CollisionObject4D::set_collision_mask);
	ClassDB::bind_method(D_METHOD("get_collision_mask"), &CollisionObject4D::get_collision_mask);

	ClassDB::bind_method(D_METHOD("set_collision_layer_value", "layer_number", "value"),
			&CollisionObject4D::set_collision_layer_value);
	ClassDB::bind_method(D_METHOD("get_collision_layer_value", "layer_number"),
			&CollisionObject4D::get_collision_layer_value);

	ClassDB::bind_method(D_METHOD("set_collision_mask_value", "layer_number", "value"),
			&CollisionObject4D::set_collision_mask_value);
	ClassDB::bind_method(D_METHOD("get_collision_mask_value", "layer_number"),
			&CollisionObject4D::get_collision_mask_value);

	ClassDB::bind_method(D_METHOD("set_collision_priority", "priority"),
			&CollisionObject4D::set_collision_priority);
	ClassDB::bind_method(D_METHOD("get_collision_priority"),
			&CollisionObject4D::get_collision_priority);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_3D_PHYSICS),
			"set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_3D_PHYSICS),
			"set_collision_mask", "get_collision_mask");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"),
			"set_collision_priority", "get_collision_priority");
}
