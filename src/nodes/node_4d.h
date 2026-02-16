#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/vector4.hpp>
#include "math/transform4d.h"

using namespace godot;

/// Node4D — Base class for all 4D spatial nodes.
///
/// Similar to Node2D and Node3D, but operates in 4D space.
/// Inherits from Node as a standalone spatial node type for 4D.
/// Can optionally sync with a child Node3D for 3D visualization.
class Node4D : public Node {
	GDCLASS(Node4D, Node)

protected:
	Transform4D transform_4d;

public:
	Node4D();
	virtual ~Node4D();

	// -- Transform4D access --------------------------------------------------

	void set_transform_4d(const Transform4D &p_transform);
	Transform4D get_transform_4d() const;

	// -- Position (4D origin) ------------------------------------------------

	void set_position_4d(const Vector4 &p_position);
	Vector4 get_position_4d() const;

	// -- Individual coordinate access ----------------------------------------

	void set_position_4d_x(real_t p_x);
	void set_position_4d_y(real_t p_y);
	void set_position_4d_z(real_t p_z);
	void set_position_4d_w(real_t p_w);

	real_t get_position_4d_x() const;
	real_t get_position_4d_y() const;
	real_t get_position_4d_z() const;
	real_t get_position_4d_w() const;

	// -- Translation ---------------------------------------------------------

	void translate_4d(const Vector4 &p_offset);

	// -- Godot lifecycle -----------------------------------------------------

	void _notification(int p_what);

protected:
	static void _bind_methods();
};
