#pragma once

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/variant/rid.hpp>
#include "node_4d.h"

using namespace godot;

class Joint4D : public Node4D {
	GDCLASS(Joint4D, Node4D);

public:
	Joint4D();
	virtual ~Joint4D();

	void set_node_a(const NodePath &p_node_a);
	NodePath get_node_a() const;

	void set_node_b(const NodePath &p_node_b);
	NodePath get_node_b() const;

	void set_solver_priority(int p_priority);
	int get_solver_priority() const;

	void set_exclude_nodes_from_collision(bool p_exclude);
	bool get_exclude_nodes_from_collision() const;

	RID get_joint_rid() const;

	void _notification(int p_what);

protected:
	static void _bind_methods();

	// Subclasses override to create the correct joint type on the physics server
	virtual void _configure_joint();

	RID _joint_rid;

private:
	NodePath _node_a;
	NodePath _node_b;
	int _solver_priority = 1;
	bool _exclude_nodes_from_collision = true;

	void _update_joint();
};
