#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

/// HyperCapsuleShape4DResource — A 4D capsule collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperCapsuleShape4DResource : public Resource {
	GDCLASS(HyperCapsuleShape4DResource, Resource)

private:
	real_t radius = 1.0;
	real_t height = 2.0;

public:
	HyperCapsuleShape4DResource();
	~HyperCapsuleShape4DResource();

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_height(real_t p_height);
	real_t get_height() const;

protected:
	static void _bind_methods();
};
