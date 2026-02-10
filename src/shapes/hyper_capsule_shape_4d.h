#pragma once

#include "shape4d.h"

/// HyperCapsuleShape4D — A 4-dimensional capsule (swept hypersphere).
///
/// Defined by a radius and height. The capsule consists of:
/// - A cylindrical section: points within `radius` of the W-axis segment from -height/2 to +height/2
/// - Two hemispherical caps at the ends
///
/// This is the Minkowski sum of a line segment (along W-axis) and a hypersphere.
class HyperCapsuleShape4D : public Shape4D {
public:
	// -- Constructor ---------------------------------------------------------

	explicit HyperCapsuleShape4D(real_t p_radius = 1.0, real_t p_height = 2.0);

	// -- Accessors -----------------------------------------------------------

	void set_radius(real_t p_radius);
	real_t get_radius() const;

	void set_height(real_t p_height);
	real_t get_height() const;

	// -- Shape4D interface ---------------------------------------------------

	virtual Vector4 get_support(const Vector4 &p_direction) const override;
	virtual AABB4D get_aabb(const Transform4D &p_transform) const override;
	virtual bool contains_point(const Vector4 &p_point) const override;

private:
	real_t radius;
	real_t height;
};
