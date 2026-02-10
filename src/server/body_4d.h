#pragma once

#include "dynamics/body_state_4d.h"
#include "math/transform4d.h"
#include "shapes/shape4d.h"
#include <vector>

class Space4D;
class Shape4DResource;

/// Body4D — Server-managed rigid body.
///
/// Wraps BodyState4D and adds shape management, space membership,
/// and server-side bookkeeping.
class Body4D {
public:
	Body4D();
	~Body4D() = default;

	// Space membership
	void set_space(Space4D *p_space);
	Space4D *get_space() const { return space; }

	// Mode
	void set_mode(BodyState4D::Mode p_mode) { state.mode = p_mode; }
	BodyState4D::Mode get_mode() const { return state.mode; }

	// Transform
	void set_transform(const Transform4D &p_transform) {
		state.position = p_transform.origin;
		// TODO: set orientation from transform.basis when rotation is implemented
	}
	Transform4D get_transform() const {
		Transform4D t;
		t.origin = state.position;
		// TODO: construct basis from state.orientation when rotation is implemented
		return t;
	}

	// Velocity
	void set_linear_velocity(const Vector4 &p_velocity) { state.linear_velocity = p_velocity; }
	Vector4 get_linear_velocity() const { return state.linear_velocity; }

	// Parameters
	void set_mass(real_t p_mass) { state.update_mass(p_mass); }
	real_t get_mass() const { return state.mass; }

	void set_linear_damp(real_t p_damp) { state.linear_damp = p_damp; }
	real_t get_linear_damp() const { return state.linear_damp; }

	// Forces
	void apply_impulse(const Vector4 &p_impulse, const Vector4 &p_position = Vector4());
	void apply_force(const Vector4 &p_force, const Vector4 &p_position = Vector4());
	void add_force(const Vector4 &p_force) { state.force = state.force + p_force; }

	// Shape management
	struct ShapeData {
		Shape4DResource *shape_resource;
		Shape4D *shape; // Actual shape instance
		Transform4D local_transform;
	};

	void add_shape(Shape4DResource *p_shape_resource, Shape4D *p_shape, const Transform4D &p_transform);
	void remove_shape(int p_index);
	int get_shape_count() const { return shapes.size(); }
	const ShapeData &get_shape_data(int p_index) const { return shapes[p_index]; }

	// Direct state access
	BodyState4D &get_state() { return state; }
	const BodyState4D &get_state() const { return state; }

private:
	Space4D *space = nullptr;
	BodyState4D state;
	std::vector<ShapeData> shapes;
};
