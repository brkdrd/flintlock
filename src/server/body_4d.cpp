#include "body_4d.h"
#include "space_4d.h"
#include "shape_4d_resource.h"

Body4D::Body4D() {
	state.mass = 1.0;
	state.inv_mass = 1.0;
}

void Body4D::set_space(Space4D *p_space) {
	if (space == p_space) {
		return;
	}

	// Remove from old space
	if (space) {
		space->remove_body(this);
	}

	space = p_space;

	// Add to new space
	if (space) {
		space->add_body(this);
	}
}

void Body4D::apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position) {
	// Impulse directly changes velocity: Δv = J / m
	if (state.inv_mass > CMP_EPSILON) {
		state.linear_velocity = state.linear_velocity + p_impulse * state.inv_mass;
	}
	// TODO: angular impulse when rotation is implemented
}

void Body4D::apply_force(const Vector4 &p_force, const Vector4 &p_position) {
	// Force accumulates and is integrated during step
	state.force = state.force + p_force;
	// TODO: torque when rotation is implemented
}

void Body4D::add_shape(Shape4DResource *p_shape_resource, Shape4D *p_shape, const Transform4D &p_transform) {
	ShapeData data;
	data.shape_resource = p_shape_resource;
	data.shape = p_shape;
	data.local_transform = p_transform;
	shapes.push_back(data);
}

void Body4D::remove_shape(int p_index) {
	if (p_index >= 0 && p_index < (int)shapes.size()) {
		shapes.erase(shapes.begin() + p_index);
	}
}
