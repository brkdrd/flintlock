#pragma once

// In GDExtension builds, Godot types come from godot-cpp.
// In standalone test builds, the compatibility shim provides them instead.
#if __has_include(<godot_cpp/variant/vector4.hpp>)
#include <godot_cpp/variant/vector4.hpp>
#include <godot_cpp/variant/vector3.hpp>
using namespace godot;
#elif __has_include("godot_compat.h")
#include "godot_compat.h"
#endif

#include <cmath>
#include <cstdint>

// Number of spatial dimensions.
constexpr int DIMENSIONS_4D = 4;

// Number of rotation planes in 4D: C(4,2) = 6.
constexpr int ROTATION_PLANES_4D = 6;
