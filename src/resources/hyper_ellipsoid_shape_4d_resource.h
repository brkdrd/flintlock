#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

/// HyperEllipsoidShape4DResource — A 4D ellipsoid collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperEllipsoidShape4DResource : public Resource {
	GDCLASS(HyperEllipsoidShape4DResource, Resource)

private:
	Vector4 radii = Vector4(1, 1, 1, 1);

public:
	HyperEllipsoidShape4DResource();
	~HyperEllipsoidShape4DResource();

	void set_radii(const Vector4 &p_radii);
	Vector4 get_radii() const;

protected:
	static void _bind_methods();
};
