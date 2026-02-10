#pragma once

#include "shape4d.h"

/// HyperSphereShape4D — A 4-dimensional sphere (3-sphere embedded in 4D space).
///
/// Defined by a single radius parameter. Center is always at origin in local space.
/// Transforms are applied externally via Transform4D.
class HyperSphereShape4D : public Shape4D {
public:
	// -- Constructor ---------------------------------------------------------

	explicit HyperSphereShape4D(real_t p_radius = 1.0);

	// -- Accessors -----------------------------------------------------------

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	// -- Shape4D interface ---------------------------------------------------

	virtual Vector4 get_support(const Vector4 &p_direction) const override;
	virtual AABB4D get_aabb(const Transform4D &p_transform) const override;
	virtual bool contains_point(const Vector4 &p_point) const override;

private:
	real_t radius;
};
