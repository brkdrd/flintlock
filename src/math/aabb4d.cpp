#include "aabb4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <cmath>
#include <algorithm>

namespace godot {

AABB4D::AABB4D() {
	position.instantiate();
	size.instantiate();
}

Ref<AABB4D> AABB4D::create(const Ref<Vector4D> &p_pos, const Ref<Vector4D> &p_size) {
	Ref<AABB4D> a;
	a.instantiate();
	a->position = p_pos;
	a->size     = p_size;
	return a;
}

Ref<Vector4D> AABB4D::get_end() const {
	return position->added(size);
}

Ref<Vector4D> AABB4D::get_center() const {
	return position->added(size->multiplied_scalar(0.5f));
}

real_t AABB4D::get_volume() const {
	return std::abs(size->x * size->y * size->z * size->w);
}

bool AABB4D::intersects(const Ref<AABB4D> &p_b) const {
	auto ea = get_end(), eb = p_b->get_end();
	return position->x < eb->x && ea->x > p_b->position->x &&
		   position->y < eb->y && ea->y > p_b->position->y &&
		   position->z < eb->z && ea->z > p_b->position->z &&
		   position->w < eb->w && ea->w > p_b->position->w;
}

bool AABB4D::encloses(const Ref<AABB4D> &p_b) const {
	auto ea = get_end(), eb = p_b->get_end();
	return position->x <= p_b->position->x && ea->x >= eb->x &&
		   position->y <= p_b->position->y && ea->y >= eb->y &&
		   position->z <= p_b->position->z && ea->z >= eb->z &&
		   position->w <= p_b->position->w && ea->w >= eb->w;
}

bool AABB4D::has_point(const Ref<Vector4D> &p) const {
	auto end = get_end();
	return p->x >= position->x && p->x <= end->x &&
		   p->y >= position->y && p->y <= end->y &&
		   p->z >= position->z && p->z <= end->z &&
		   p->w >= position->w && p->w <= end->w;
}

Ref<AABB4D> AABB4D::merge(const Ref<AABB4D> &p_b) const {
	auto ea = get_end(), eb = p_b->get_end();
	auto np = Vector4D::_make(
		std::min(position->x, p_b->position->x),
		std::min(position->y, p_b->position->y),
		std::min(position->z, p_b->position->z),
		std::min(position->w, p_b->position->w)
	);
	auto ne = Vector4D::_make(
		std::max(ea->x, eb->x),
		std::max(ea->y, eb->y),
		std::max(ea->z, eb->z),
		std::max(ea->w, eb->w)
	);
	return create(np, ne->subtracted(np));
}

Ref<AABB4D> AABB4D::grow(real_t p_by) const {
	return create(
		position->subtracted(Vector4D::_make(p_by, p_by, p_by, p_by)),
		size->added(Vector4D::_make(p_by * 2, p_by * 2, p_by * 2, p_by * 2))
	);
}

Ref<AABB4D> AABB4D::expand(const Ref<Vector4D> &p_to) const {
	auto end = get_end();
	auto np = Vector4D::_make(
		std::min(position->x, p_to->x), std::min(position->y, p_to->y),
		std::min(position->z, p_to->z), std::min(position->w, p_to->w)
	);
	auto ne = Vector4D::_make(
		std::max(end->x, p_to->x), std::max(end->y, p_to->y),
		std::max(end->z, p_to->z), std::max(end->w, p_to->w)
	);
	return create(np, ne->subtracted(np));
}

bool AABB4D::is_equal_approx(const Ref<AABB4D> &p_b) const {
	return position->is_equal_approx(p_b->position) && size->is_equal_approx(p_b->size);
}

void AABB4D::_bind_methods() {
	ClassDB::bind_static_method("AABB4D", D_METHOD("create", "position", "size"), &AABB4D::create);

	ClassDB::bind_method(D_METHOD("get_position"), &AABB4D::get_position);
	ClassDB::bind_method(D_METHOD("set_position", "p"), &AABB4D::set_position);
	ClassDB::bind_method(D_METHOD("get_size"), &AABB4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "s"), &AABB4D::set_size);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "position", PROPERTY_HINT_RESOURCE_TYPE, "Vector4D"), "set_position", "get_position");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "size",     PROPERTY_HINT_RESOURCE_TYPE, "Vector4D"), "set_size",     "get_size");

	ClassDB::bind_method(D_METHOD("get_end"),    &AABB4D::get_end);
	ClassDB::bind_method(D_METHOD("get_center"), &AABB4D::get_center);
	ClassDB::bind_method(D_METHOD("get_volume"), &AABB4D::get_volume);

	ClassDB::bind_method(D_METHOD("intersects", "b"), &AABB4D::intersects);
	ClassDB::bind_method(D_METHOD("encloses",   "b"), &AABB4D::encloses);
	ClassDB::bind_method(D_METHOD("has_point",  "point"), &AABB4D::has_point);

	ClassDB::bind_method(D_METHOD("merge",  "b"),    &AABB4D::merge);
	ClassDB::bind_method(D_METHOD("grow",   "by"),   &AABB4D::grow);
	ClassDB::bind_method(D_METHOD("expand", "to"),   &AABB4D::expand);

	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &AABB4D::is_equal_approx);
}

} // namespace godot
