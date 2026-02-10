#pragma once

#include "shape4d.h"

/// HyperEllipsoidShape4D — A 4-dimensional ellipsoid with non-uniform radii.
///
/// Defined by semi-axes (radii) along each coordinate axis.
/// The ellipsoid surface satisfies: (x/a)² + (y/b)² + (z/c)² + (w/d)² = 1
/// where (a,b,c,d) are the radii.
///
/// Useful for modeling non-uniform astronomical bodies or scaled spheres.
class HyperEllipsoidShape4D : public Shape4D {
public:
	// -- Constructor ---------------------------------------------------------

	explicit HyperEllipsoidShape4D(const Vector4 &p_radii = Vector4(1, 1, 1, 1));

	// -- Accessors -----------------------------------------------------------

	void set_radii(const Vector4 &p_radii);
	Vector4 get_radii() const;

	// -- Shape4D interface ---------------------------------------------------

	virtual Vector4 get_support(const Vector4 &p_direction) const override;
	virtual AABB4D get_aabb(const Transform4D &p_transform) const override;
	virtual bool contains_point(const Vector4 &p_point) const override;

private:
	Vector4 radii;
};
