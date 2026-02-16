#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

/// HyperBoxShape4DResource — A 4D box (tesseract) collision shape.
///
/// This is a Godot Resource that can be saved and reused.
class HyperBoxShape4DResource : public Resource {
	GDCLASS(HyperBoxShape4DResource, Resource)

private:
	Vector4 half_extents = Vector4(1, 1, 1, 1);

public:
	HyperBoxShape4DResource();
	~HyperBoxShape4DResource();

	void set_half_extents(const Vector4 &p_half_extents);
	Vector4 get_half_extents() const;

protected:
	static void _bind_methods();
};
