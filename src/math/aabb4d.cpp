#include "aabb4d.h"

#include <algorithm>

// -- Constructors ------------------------------------------------------------

AABB4D::AABB4D() :
		position(Vector4(0, 0, 0, 0)), size(Vector4(0, 0, 0, 0)) {}

AABB4D::AABB4D(const Vector4 &p_position, const Vector4 &p_size) :
		position(p_position), size(p_size) {}

// -- End point ---------------------------------------------------------------

Vector4 AABB4D::get_end() const {
	return position + size;
}

// -- Containment -------------------------------------------------------------

bool AABB4D::has_point(const Vector4 &p_point) const {
	Vector4 end = get_end();
	for (int i = 0; i < 4; i++) {
		if (p_point.components[i] < position.components[i] || p_point.components[i] > end.components[i]) {
			return false;
		}
	}
	return true;
}

// -- Intersection ------------------------------------------------------------

bool AABB4D::intersects(const AABB4D &p_other) const {
	Vector4 end_a = get_end();
	Vector4 end_b = p_other.get_end();
	for (int i = 0; i < 4; i++) {
		if (position.components[i] >= end_b.components[i] || p_other.position.components[i] >= end_a.components[i]) {
			return false;
		}
	}
	return true;
}

AABB4D AABB4D::intersection(const AABB4D &p_other) const {
	Vector4 new_pos, new_end;
	Vector4 end_a = get_end();
	Vector4 end_b = p_other.get_end();
	for (int i = 0; i < 4; i++) {
		new_pos.components[i] = std::max(position.components[i], p_other.position.components[i]);
		new_end.components[i] = std::min(end_a.components[i], end_b.components[i]);
		if (new_end.components[i] < new_pos.components[i]) {
			return AABB4D(); // No overlap.
		}
	}
	return AABB4D(new_pos, new_end - new_pos);
}

// -- Merge -------------------------------------------------------------------

AABB4D AABB4D::merge(const AABB4D &p_other) const {
	Vector4 new_pos, new_end;
	Vector4 end_a = get_end();
	Vector4 end_b = p_other.get_end();
	for (int i = 0; i < 4; i++) {
		new_pos.components[i] = std::min(position.components[i], p_other.position.components[i]);
		new_end.components[i] = std::max(end_a.components[i], end_b.components[i]);
	}
	return AABB4D(new_pos, new_end - new_pos);
}

// -- Grow --------------------------------------------------------------------

AABB4D AABB4D::grow(real_t p_amount) const {
	Vector4 margin(p_amount, p_amount, p_amount, p_amount);
	return AABB4D(position - margin, size + margin * 2.0);
}

// -- Queries -----------------------------------------------------------------

Vector4 AABB4D::get_center() const {
	return position + size * 0.5;
}

real_t AABB4D::get_volume() const {
	return size.x * size.y * size.z * size.w;
}

int AABB4D::get_longest_axis() const {
	int longest = 0;
	real_t max_len = size.x;
	for (int i = 1; i < 4; i++) {
		if (size.components[i] > max_len) {
			max_len = size.components[i];
			longest = i;
		}
	}
	return longest;
}

Vector4 AABB4D::get_support(const Vector4 &p_dir) const {
	Vector4 end = get_end();
	Vector4 result;
	for (int i = 0; i < 4; i++) {
		result.components[i] = (p_dir.components[i] >= 0) ? end.components[i] : position.components[i];
	}
	return result;
}
