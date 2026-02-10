#pragma once

#include "math/math_defs_4d.h"
#include "math/transform4d.h"
#include "math/aabb4d.h"

/// Shape4D — Abstract base class for all 4D geometric shapes.
///
/// Provides the three core queries needed by physics simulation:
/// 1. Support function (for GJK/EPA collision detection)
/// 2. AABB calculation (for broadphase)
/// 3. Point containment (for spatial queries)
class Shape4D {
public:
	// Virtual destructor essential for abstract base class
	virtual ~Shape4D() = default;

	// -- Core shape queries --------------------------------------------------

	/// Support function: returns the farthest point on the shape surface
	/// in the given direction. Used by GJK algorithm.
	/// Direction vector does NOT need to be normalized.
	virtual Vector4 get_support(const Vector4 &p_direction) const = 0;

	/// Compute axis-aligned bounding box in world space after applying transform.
	/// Must handle rotation, translation, and scale correctly.
	virtual AABB4D get_aabb(const Transform4D &p_transform) const = 0;

	/// Test if a point (in local shape space) is contained within the shape.
	/// Used for spatial queries and trigger volumes.
	virtual bool contains_point(const Vector4 &p_point) const = 0;
};
