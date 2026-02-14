#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

/// HyperSphereShape4D — A spherical collision shape in 4D (hypersphere).
///
/// This is a Godot Resource that can be saved and reused.
class HyperSphereShape4DResource : public Resource {
	GDCLASS(HyperSphereShape4DResource, Resource)

private:
	real_t radius = 1.0;

public:
	HyperSphereShape4DResource();
	~HyperSphereShape4DResource();

	void set_radius(real_t p_radius);
	real_t get_radius() const;

protected:
	static void _bind_methods();
};
