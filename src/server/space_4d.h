#pragma once

#include "math/math_defs_4d.h"
#include <vector>

// Forward declaration
class Body4D;

/// Space4D — Internal container for a physics world.
///
/// Holds all bodies in the simulation space and manages stepping.
class Space4D {
public:
	Space4D() = default;
	~Space4D() = default;

	void set_active(bool p_active) { active = p_active; }
	bool is_active() const { return active; }

	void add_body(Body4D *p_body);
	void remove_body(Body4D *p_body);

	void step(real_t p_delta);

	const std::vector<Body4D*> &get_bodies() const { return bodies; }

private:
	bool active = false;
	std::vector<Body4D*> bodies;
};
