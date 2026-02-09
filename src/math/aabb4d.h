#pragma once

#include "math_defs_4d.h"

// AABB4D — 4D axis-aligned bounding box defined by a min-corner position + size.

class AABB4D {
public:
	Vector4 position; // Min corner.
	Vector4 size; // Extent (must be non-negative).

	// -- Constructors --------------------------------------------------------

	AABB4D();
	AABB4D(const Vector4 &p_position, const Vector4 &p_size);

	// -- End point -----------------------------------------------------------

	Vector4 get_end() const;

	// -- Containment ---------------------------------------------------------

	bool has_point(const Vector4 &p_point) const;

	// -- Intersection --------------------------------------------------------

	bool intersects(const AABB4D &p_other) const;
	AABB4D intersection(const AABB4D &p_other) const;

	// -- Merge ---------------------------------------------------------------

	AABB4D merge(const AABB4D &p_other) const;

	// -- Grow ----------------------------------------------------------------

	AABB4D grow(real_t p_amount) const;

	// -- Queries -------------------------------------------------------------

	Vector4 get_center() const;
	real_t get_volume() const;
	int get_longest_axis() const;
	Vector4 get_support(const Vector4 &p_dir) const;
};
