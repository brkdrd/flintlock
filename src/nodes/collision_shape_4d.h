#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rid.hpp>
#include "node_4d.h"
#include "../resources/shape_4d.h"

using namespace godot;

class CollisionObject4D;

class CollisionShape4D : public Node4D {
	GDCLASS(CollisionShape4D, Node4D);

public:
	CollisionShape4D();
	~CollisionShape4D();

	void set_shape(const Ref<Shape4D> &p_shape);
	Ref<Shape4D> get_shape() const;

	void set_disabled(bool p_disabled);
	bool is_disabled() const;

	void set_debug_color(const Color &p_color);
	Color get_debug_color() const;

	void set_debug_fill(bool p_fill);
	bool get_debug_fill() const;

	void _notification(int p_what);

protected:
	static void _bind_methods();

private:
	Ref<Shape4D> _shape;
	bool _disabled = false;
	Color _debug_color = Color(0.0f, 0.6f, 0.7f, 0.42f);
	bool _debug_fill = true;

	// Cached reference to parent collision object and shape index
	CollisionObject4D *_parent_collision_object = nullptr;
	int _shape_index = -1;
	RID _shape_rid;

	void _update_in_server();
	void _register_with_parent();
	void _unregister_from_parent();
};
