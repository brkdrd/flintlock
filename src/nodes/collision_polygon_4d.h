#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/rid.hpp>
#include "node_4d.h"
#include "../resources/shape_4d.h"

using namespace godot;

class CollisionObject4D;

class CollisionPolygon4D : public Node4D {
	GDCLASS(CollisionPolygon4D, Node4D);

public:
	CollisionPolygon4D();
	~CollisionPolygon4D();

	void set_polygon(const PackedVector3Array &p_polygon);
	PackedVector3Array get_polygon() const;

	void set_depth(float p_depth);
	float get_depth() const;

	void set_disabled(bool p_disabled);
	bool is_disabled() const;

	void set_margin(float p_margin);
	float get_margin() const;

	void _notification(int p_what);

protected:
	static void _bind_methods();

private:
	PackedVector3Array _polygon;
	float _depth = 1.0f;
	bool _disabled = false;
	float _margin = 0.04f;

	// Generated convex shape from polygon + depth extrusion
	Ref<ConvexPolygonShape4D> _generated_shape;
	CollisionObject4D *_parent_collision_object = nullptr;
	RID _shape_rid;

	void _build_shape();
	void _register_with_parent();
	void _unregister_from_parent();
	void _update_in_server();
};
