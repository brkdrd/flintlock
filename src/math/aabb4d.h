#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "vector4d.h"

namespace godot {

// 4D axis-aligned bounding hyperbox. Analog of Godot's AABB.
class AABB4D : public RefCounted {
	GDCLASS(AABB4D, RefCounted);

public:
	Ref<Vector4D> position;
	Ref<Vector4D> size;

	AABB4D();

	static Ref<AABB4D> create(const Ref<Vector4D> &p_pos, const Ref<Vector4D> &p_size);

	Ref<Vector4D> get_position() const { return position; }
	Ref<Vector4D> get_size()     const { return size; }
	void set_position(const Ref<Vector4D> &p) { position = p; }
	void set_size(const Ref<Vector4D> &s)     { size = s; }

	// Property adapters using built-in Vector4 (avoids ClassDB default-value warnings)
	Vector4 get_position_v4() const { return Vector4(position->x, position->y, position->z, position->w); }
	Vector4 get_size_v4()     const { return Vector4(size->x, size->y, size->z, size->w); }
	void set_position_v4(const Vector4 &p) { position->x = p.x; position->y = p.y; position->z = p.z; position->w = p.w; }
	void set_size_v4(const Vector4 &s)     { size->x = s.x; size->y = s.y; size->z = s.z; size->w = s.w; }

	Ref<Vector4D> get_end() const;
	Ref<Vector4D> get_center() const;
	real_t get_volume() const; // hypervolume (x*y*z*w)

	bool intersects(const Ref<AABB4D> &p_b) const;
	bool encloses(const Ref<AABB4D> &p_b) const;
	bool has_point(const Ref<Vector4D> &p_point) const;

	Ref<AABB4D> merge(const Ref<AABB4D> &p_b) const;
	Ref<AABB4D> grow(real_t p_by) const;
	Ref<AABB4D> expand(const Ref<Vector4D> &p_to) const;

	bool is_equal_approx(const Ref<AABB4D> &p_b) const;

protected:
	static void _bind_methods();
};

} // namespace godot
