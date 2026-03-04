#pragma once

#include <godot_cpp/core/defs.hpp>

namespace godot {

// 6 rotation planes in 4D space
enum PlaneAxis {
	PLANE_XY = 0,
	PLANE_XZ = 1,
	PLANE_XW = 2,
	PLANE_YZ = 3,
	PLANE_YW = 4,
	PLANE_ZW = 5,
};

} // namespace godot
