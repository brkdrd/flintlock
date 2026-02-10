#pragma once

#include "shape4d.h"

/// HyperBoxShape4D — A 4-dimensional box (tesseract / 8-cell).
///
/// Defined by half-extents (distance from center to face) along each axis.
/// The box extends from -half_extents to +half_extents in local space.
///
/// Topology: 16 vertices, 32 edges, 24 faces (2D squares), 8 cells (3D cubes).
class HyperBoxShape4D : public Shape4D {
public:
	// -- Constructor ---------------------------------------------------------

	explicit HyperBoxShape4D(const Vector4 &p_half_extents = Vector4(1, 1, 1, 1));

	// -- Accessors -----------------------------------------------------------

	void set_half_extents(const Vector4 &p_half_extents);
	Vector4 get_half_extents() const;

	// -- Shape4D interface ---------------------------------------------------

	virtual Vector4 get_support(const Vector4 &p_direction) const override;
	virtual AABB4D get_aabb(const Transform4D &p_transform) const override;
	virtual bool contains_point(const Vector4 &p_point) const override;

private:
	Vector4 half_extents;
};
