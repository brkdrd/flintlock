#include "node_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot {

Node4D::Node4D() {
	_local_transform.instantiate();
	_global_transform_cache.instantiate();
}
Node4D::~Node4D() {}

// ─── Internal helpers ─────────────────────────────────────────────────────────

void Node4D::_update_parent_node_4d() {
	Node *parent = get_parent();
	_parent_node_4d = parent ? Object::cast_to<Node4D>(parent) : nullptr;
}

void Node4D::_update_global_transform() const {
	if (!_global_transform_dirty) return;
	if (_parent_node_4d && !_top_level) {
		Ref<Transform4D> parent_global = _parent_node_4d->get_global_transform_4d();
		_global_transform_cache = parent_global->multiplied(_local_transform);
	} else {
		// Copy local into cache (deep copy via creating new)
		_global_transform_cache = Transform4D::create(_local_transform->get_basis(), _local_transform->get_origin());
	}
	_global_transform_dirty = false;
}

void Node4D::_propagate_transform_changed() {
	_global_transform_dirty = true;

	if (_notify_transform) {
		notification(NOTIFICATION_TRANSFORM_4D_CHANGED);
	}

	// Propagate to child Node4D nodes
	for (int i = 0; i < get_child_count(); i++) {
		Node4D *child = Object::cast_to<Node4D>(get_child(i));
		if (child && !child->_top_level) {
			child->_propagate_transform_changed();
		}
	}
}

// ─── Notifications ────────────────────────────────────────────────────────────

void Node4D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			_update_parent_node_4d();
			_global_transform_dirty = true;
			break;
		case NOTIFICATION_EXIT_TREE:
			_parent_node_4d = nullptr;
			break;
		case NOTIFICATION_PARENTED:
			_update_parent_node_4d();
			_global_transform_dirty = true;
			break;
		case NOTIFICATION_UNPARENTED:
			_parent_node_4d = nullptr;
			_global_transform_dirty = true;
			break;
		default:
			break;
	}
}

// ─── Transform ────────────────────────────────────────────────────────────────

void Node4D::set_transform_4d(const Ref<Transform4D> &p_transform) {
	_local_transform = p_transform;
	_propagate_transform_changed();
	if (_notify_local_transform) {
		notification(NOTIFICATION_TRANSFORM_4D_CHANGED);
	}
	emit_signal("transform_4d_changed");
}

Ref<Transform4D> Node4D::get_transform_4d() const {
	return _local_transform;
}

void Node4D::set_global_transform_4d(const Ref<Transform4D> &p_transform) {
	if (_parent_node_4d && !_top_level) {
		Ref<Transform4D> parent_inv = _parent_node_4d->get_global_transform_4d()->affine_inverse();
		_local_transform = parent_inv->multiplied(p_transform);
	} else {
		_local_transform = Transform4D::create(p_transform->get_basis(), p_transform->get_origin());
	}
	_global_transform_cache = Transform4D::create(p_transform->get_basis(), p_transform->get_origin());
	_global_transform_dirty = false;
	_propagate_transform_changed();
}

Ref<Transform4D> Node4D::get_global_transform_4d() const {
	_update_global_transform();
	return _global_transform_cache;
}

void Node4D::set_position_4d(const Vector4 &p_pos) {
	Ref<Vector4D> o = _local_transform->get_origin();
	o->x = p_pos.x; o->y = p_pos.y; o->z = p_pos.z; o->w = p_pos.w;
	_propagate_transform_changed();
	emit_signal("transform_4d_changed");
}

Vector4 Node4D::get_position_4d() const {
	Ref<Vector4D> o = _local_transform->get_origin();
	return Vector4(o->x, o->y, o->z, o->w);
}

void Node4D::set_global_position_4d(const Vector4 &p_pos) {
	Ref<Vector4D> v; v.instantiate();
	v->x = p_pos.x; v->y = p_pos.y; v->z = p_pos.z; v->w = p_pos.w;
	Ref<Transform4D> g = get_global_transform_4d();
	g->set_origin(v);
	set_global_transform_4d(g);
}

Vector4 Node4D::get_global_position_4d() const {
	Ref<Vector4D> o = get_global_transform_4d()->get_origin();
	return Vector4(o->x, o->y, o->z, o->w);
}

void Node4D::set_basis_4d(const Projection &p_basis) {
	Ref<Basis4D> b = _local_transform->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[c][r] = p_basis.columns[c][r];
	_propagate_transform_changed();
	emit_signal("transform_4d_changed");
}

Projection Node4D::get_basis_4d() const {
	Projection p;
	Ref<Basis4D> b = _local_transform->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			p.columns[c][r] = b->data[c][r];
	return p;
}

Projection Node4D::get_global_basis_4d() const {
	Projection p;
	Ref<Basis4D> b = get_global_transform_4d()->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			p.columns[c][r] = b->data[c][r];
	return p;
}

// ─── Transform property adapters ──────────────────────────────────────────────

PackedFloat64Array Node4D::_get_transform_prop() const {
	PackedFloat64Array arr;
	arr.resize(20);
	Ref<Basis4D> b = _local_transform->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			arr[c * 4 + r] = b->data[c][r];
	Ref<Vector4D> o = _local_transform->get_origin();
	arr[16] = o->x; arr[17] = o->y; arr[18] = o->z; arr[19] = o->w;
	return arr;
}

void Node4D::_set_transform_prop(const PackedFloat64Array &p_arr) {
	if (p_arr.size() < 20) return;
	Ref<Basis4D> b = _local_transform->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[c][r] = p_arr[c * 4 + r];
	Ref<Vector4D> o = _local_transform->get_origin();
	o->x = p_arr[16]; o->y = p_arr[17]; o->z = p_arr[18]; o->w = p_arr[19];
	_propagate_transform_changed();
	emit_signal("transform_4d_changed");
}

PackedFloat64Array Node4D::_get_global_transform_prop() const {
	PackedFloat64Array arr;
	arr.resize(20);
	Ref<Transform4D> g = get_global_transform_4d();
	Ref<Basis4D> b = g->get_basis();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			arr[c * 4 + r] = b->data[c][r];
	Ref<Vector4D> o = g->get_origin();
	arr[16] = o->x; arr[17] = o->y; arr[18] = o->z; arr[19] = o->w;
	return arr;
}

void Node4D::_set_global_transform_prop(const PackedFloat64Array &p_arr) {
	if (p_arr.size() < 20) return;
	Ref<Basis4D> b; b.instantiate();
	for (int c = 0; c < 4; c++)
		for (int r = 0; r < 4; r++)
			b->data[c][r] = p_arr[c * 4 + r];
	Ref<Vector4D> o; o.instantiate();
	o->x = p_arr[16]; o->y = p_arr[17]; o->z = p_arr[18]; o->w = p_arr[19];
	set_global_transform_4d(Transform4D::create(b, o));
}

// ─── Visibility ───────────────────────────────────────────────────────────────

void Node4D::set_visible(bool p_visible) {
	if (_visible == p_visible) return;
	_visible = p_visible;
	notification(NOTIFICATION_VISIBILITY_4D_CHANGED);
}
bool Node4D::is_visible() const { return _visible; }

bool Node4D::is_visible_in_tree() const {
	if (!_visible) return false;
	Node4D *p = _parent_node_4d;
	while (p) {
		if (!p->_visible) return false;
		p = p->_parent_node_4d;
	}
	return true;
}

void Node4D::show() { set_visible(true); }
void Node4D::hide() { set_visible(false); }

// ─── Top-level ────────────────────────────────────────────────────────────────

void Node4D::set_top_level(bool p_top_level) {
	_top_level = p_top_level;
	_global_transform_dirty = true;
}
bool Node4D::is_top_level() const { return _top_level; }

// ─── Notification opt-in ─────────────────────────────────────────────────────

void Node4D::set_notify_transform_4d(bool p_enable) { _notify_transform = p_enable; }
bool Node4D::is_transform_notification_enabled_4d() const { return _notify_transform; }
void Node4D::set_notify_local_transform_4d(bool p_enable) { _notify_local_transform = p_enable; }
bool Node4D::is_local_transform_notification_enabled_4d() const { return _notify_local_transform; }

// ─── Convenience methods ──────────────────────────────────────────────────────

void Node4D::rotate(int p_plane, real_t p_angle) {
	Ref<Basis4D> b = _local_transform->get_basis()->rotated(p_plane, p_angle);
	_local_transform->set_basis(b);
	_propagate_transform_changed();
}

void Node4D::rotate_local(int p_plane, real_t p_angle) {
	Ref<Basis4D> rot = Basis4D::from_rotation(p_plane, p_angle);
	Ref<Basis4D> b = _local_transform->get_basis()->multiplied(rot);
	_local_transform->set_basis(b);
	_propagate_transform_changed();
}

void Node4D::translate(const Ref<Vector4D> &p_offset) {
	Ref<Vector4D> new_origin = _local_transform->get_origin()->added(p_offset);
	_local_transform->set_origin(new_origin);
	_propagate_transform_changed();
}

void Node4D::translate_local(const Ref<Vector4D> &p_offset) {
	Ref<Vector4D> world_offset = _local_transform->get_basis()->xform(p_offset);
	Ref<Vector4D> new_origin = _local_transform->get_origin()->added(world_offset);
	_local_transform->set_origin(new_origin);
	_propagate_transform_changed();
}

void Node4D::global_rotate(int p_plane, real_t p_angle) {
	Ref<Transform4D> g = get_global_transform_4d();
	Ref<Basis4D> b = g->get_basis()->rotated(p_plane, p_angle);
	g->set_basis(b);
	set_global_transform_4d(g);
}

void Node4D::global_translate(const Ref<Vector4D> &p_offset) {
	Ref<Transform4D> g = get_global_transform_4d();
	Ref<Vector4D> new_origin = g->get_origin()->added(p_offset);
	g->set_origin(new_origin);
	set_global_transform_4d(g);
}

Ref<Vector4D> Node4D::to_local(const Ref<Vector4D> &p_global_point) const {
	return get_global_transform_4d()->xform_inv(p_global_point);
}

Ref<Vector4D> Node4D::to_global(const Ref<Vector4D> &p_local_point) const {
	return get_global_transform_4d()->xform(p_local_point);
}

Node4D *Node4D::get_parent_node_4d() const {
	return _parent_node_4d;
}

void Node4D::orthonormalize() {
	_local_transform->get_basis()->orthonormalize();
	_propagate_transform_changed();
}

void Node4D::set_identity() {
	_local_transform = Transform4D::create_identity();
	_propagate_transform_changed();
}

void Node4D::force_update_transform() {
	_global_transform_dirty = true;
	_update_global_transform();
	notification(NOTIFICATION_TRANSFORM_4D_CHANGED);
}

// ─── Bindings ─────────────────────────────────────────────────────────────────

void Node4D::_bind_methods() {
	// Transform (methods kept for GDScript; properties use adapters to avoid default-value warnings)
	ClassDB::bind_method(D_METHOD("set_transform_4d", "transform"), &Node4D::set_transform_4d);
	ClassDB::bind_method(D_METHOD("get_transform_4d"), &Node4D::get_transform_4d);
	ClassDB::bind_method(D_METHOD("set_global_transform_4d", "transform"), &Node4D::set_global_transform_4d);
	ClassDB::bind_method(D_METHOD("get_global_transform_4d"), &Node4D::get_global_transform_4d);
	ClassDB::bind_method(D_METHOD("_set_transform_prop", "data"), &Node4D::_set_transform_prop);
	ClassDB::bind_method(D_METHOD("_get_transform_prop"), &Node4D::_get_transform_prop);
	ClassDB::bind_method(D_METHOD("_set_global_transform_prop", "data"), &Node4D::_set_global_transform_prop);
	ClassDB::bind_method(D_METHOD("_get_global_transform_prop"), &Node4D::_get_global_transform_prop);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "transform_4d"),
		"_set_transform_prop", "_get_transform_prop");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_FLOAT64_ARRAY, "global_transform_4d",
		PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR),
		"_set_global_transform_prop", "_get_global_transform_prop");

	// Position
	ClassDB::bind_method(D_METHOD("set_position_4d", "position"), &Node4D::set_position_4d);
	ClassDB::bind_method(D_METHOD("get_position_4d"), &Node4D::get_position_4d);
	ClassDB::bind_method(D_METHOD("set_global_position_4d", "position"), &Node4D::set_global_position_4d);
	ClassDB::bind_method(D_METHOD("get_global_position_4d"), &Node4D::get_global_position_4d);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "position_4d"),
		"set_position_4d", "get_position_4d");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "global_position_4d",
		PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR),
		"set_global_position_4d", "get_global_position_4d");

	// Basis
	ClassDB::bind_method(D_METHOD("set_basis_4d", "basis"), &Node4D::set_basis_4d);
	ClassDB::bind_method(D_METHOD("get_basis_4d"), &Node4D::get_basis_4d);
	ClassDB::bind_method(D_METHOD("get_global_basis_4d"), &Node4D::get_global_basis_4d);
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "basis_4d"),
		"set_basis_4d", "get_basis_4d");
	ADD_PROPERTY(PropertyInfo(Variant::PROJECTION, "global_basis_4d",
		PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR | PROPERTY_USAGE_READ_ONLY),
		"set_basis_4d", "get_global_basis_4d");

	// Visibility
	ClassDB::bind_method(D_METHOD("set_visible", "visible"), &Node4D::set_visible);
	ClassDB::bind_method(D_METHOD("is_visible"), &Node4D::is_visible);
	ClassDB::bind_method(D_METHOD("is_visible_in_tree"), &Node4D::is_visible_in_tree);
	ClassDB::bind_method(D_METHOD("show"), &Node4D::show);
	ClassDB::bind_method(D_METHOD("hide"), &Node4D::hide);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");

	// Top level
	ClassDB::bind_method(D_METHOD("set_top_level", "enable"), &Node4D::set_top_level);
	ClassDB::bind_method(D_METHOD("is_top_level"), &Node4D::is_top_level);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "top_level"), "set_top_level", "is_top_level");

	// Notification opt-in
	ClassDB::bind_method(D_METHOD("set_notify_transform_4d", "enable"), &Node4D::set_notify_transform_4d);
	ClassDB::bind_method(D_METHOD("is_transform_notification_enabled_4d"), &Node4D::is_transform_notification_enabled_4d);
	ClassDB::bind_method(D_METHOD("set_notify_local_transform_4d", "enable"), &Node4D::set_notify_local_transform_4d);
	ClassDB::bind_method(D_METHOD("is_local_transform_notification_enabled_4d"), &Node4D::is_local_transform_notification_enabled_4d);

	// Convenience
	ClassDB::bind_method(D_METHOD("rotate", "plane", "angle"), &Node4D::rotate);
	ClassDB::bind_method(D_METHOD("rotate_local", "plane", "angle"), &Node4D::rotate_local);
	ClassDB::bind_method(D_METHOD("translate", "offset"), &Node4D::translate);
	ClassDB::bind_method(D_METHOD("translate_local", "offset"), &Node4D::translate_local);
	ClassDB::bind_method(D_METHOD("global_rotate", "plane", "angle"), &Node4D::global_rotate);
	ClassDB::bind_method(D_METHOD("global_translate", "offset"), &Node4D::global_translate);
	ClassDB::bind_method(D_METHOD("to_local", "global_point"), &Node4D::to_local);
	ClassDB::bind_method(D_METHOD("to_global", "local_point"), &Node4D::to_global);
	ClassDB::bind_method(D_METHOD("get_parent_node_4d"), &Node4D::get_parent_node_4d);
	ClassDB::bind_method(D_METHOD("orthonormalize"), &Node4D::orthonormalize);
	ClassDB::bind_method(D_METHOD("set_identity"), &Node4D::set_identity);
	ClassDB::bind_method(D_METHOD("force_update_transform"), &Node4D::force_update_transform);

	// Signals
	ADD_SIGNAL(MethodInfo("transform_4d_changed"));

	// Notification constants
	BIND_CONSTANT(NOTIFICATION_TRANSFORM_4D_CHANGED);
	BIND_CONSTANT(NOTIFICATION_VISIBILITY_4D_CHANGED);
}

} // namespace godot
