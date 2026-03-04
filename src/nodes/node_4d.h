#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "../math/transform4d.h"
#include "../math/vector4d.h"
#include "../math/basis4d.h"

namespace godot {

class Node4D : public Node {
	GDCLASS(Node4D, Node);

public:
	enum {
		NOTIFICATION_TRANSFORM_4D_CHANGED  = 3000,
		NOTIFICATION_VISIBILITY_4D_CHANGED = 3001,
	};

	Node4D();
	~Node4D();

	// ─── Transform ────────────────────────────────────────────────────────────

	// Local transform (relative to parent Node4D)
	void set_transform_4d(const Ref<Transform4D> &p_transform);
	Ref<Transform4D> get_transform_4d() const;

	// Global (world-space) transform — computed lazily
	void set_global_transform_4d(const Ref<Transform4D> &p_transform);
	Ref<Transform4D> get_global_transform_4d() const;

	// Position shortcuts
	void set_position_4d(const Ref<Vector4D> &p_pos);
	Ref<Vector4D> get_position_4d() const;

	void set_global_position_4d(const Ref<Vector4D> &p_pos);
	Ref<Vector4D> get_global_position_4d() const;

	// Basis shortcuts
	void set_basis_4d(const Ref<Basis4D> &p_basis);
	Ref<Basis4D> get_basis_4d() const;

	Ref<Basis4D> get_global_basis_4d() const;

	// ─── Visibility ───────────────────────────────────────────────────────────
	void set_visible(bool p_visible);
	bool is_visible() const;
	bool is_visible_in_tree() const;
	void show();
	void hide();

	// ─── Top-level ────────────────────────────────────────────────────────────
	void set_top_level(bool p_top_level);
	bool is_top_level() const;

	// ─── Transform change notifications (opt-in) ──────────────────────────────
	void set_notify_transform_4d(bool p_enable);
	bool is_transform_notification_enabled_4d() const;
	void set_notify_local_transform_4d(bool p_enable);
	bool is_local_transform_notification_enabled_4d() const;

	// ─── Convenience methods ──────────────────────────────────────────────────
	void rotate(int p_plane, real_t p_angle);
	void rotate_local(int p_plane, real_t p_angle);
	void translate(const Ref<Vector4D> &p_offset);
	void translate_local(const Ref<Vector4D> &p_offset);
	void global_rotate(int p_plane, real_t p_angle);
	void global_translate(const Ref<Vector4D> &p_offset);
	Ref<Vector4D> to_local(const Ref<Vector4D> &p_global_point) const;
	Ref<Vector4D> to_global(const Ref<Vector4D> &p_local_point) const;
	Node4D *get_parent_node_4d() const;
	void orthonormalize();
	void set_identity();
	void force_update_transform();

	// ─── Notifications ────────────────────────────────────────────────────────
	void _notification(int p_what);

protected:
	static void _bind_methods();

private:
	// Internal transform storage (C++ structs, not Refs — avoids Ref overhead per frame)
	// We use Ref<> fields but manage them as value types.
	Ref<Transform4D> _local_transform;
	mutable Ref<Transform4D> _global_transform_cache;
	mutable bool _global_transform_dirty = true;

	bool _visible              = true;
	bool _top_level            = false;
	bool _notify_transform     = false;
	bool _notify_local_transform = false;

	Node4D *_parent_node_4d    = nullptr;

	void _propagate_transform_changed();
	void _update_global_transform() const;
	void _update_parent_node_4d();
};

} // namespace godot
