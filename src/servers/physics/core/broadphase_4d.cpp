#include "broadphase_4d.h"
#include <algorithm>
#include <cmath>

int BroadPhase4D::_allocate_node() {
	if (!_free_list.empty()) {
		int id = _free_list.back();
		_free_list.pop_back();
		_nodes[id] = BVHNode4D();
		return id;
	}
	int id = (int)_nodes.size();
	_nodes.push_back(BVHNode4D());
	return id;
}

void BroadPhase4D::_free_node(int node_id) {
	_nodes[node_id] = BVHNode4D();
	_free_list.push_back(node_id);
}

int BroadPhase4D::insert(const AABB4 &aabb, int user_data) {
	int leaf = _allocate_node();
	_nodes[leaf].aabb = aabb.grown(BVH_AABB_MARGIN);
	_nodes[leaf].user_data = user_data;
	_nodes[leaf].height = 0;
	_insert_leaf(leaf);
	return leaf;
}

void BroadPhase4D::remove(int proxy_id) {
	if (proxy_id < 0 || proxy_id >= (int)_nodes.size()) return;
	_remove_leaf(proxy_id);
	_free_node(proxy_id);
}

bool BroadPhase4D::update(int proxy_id, const AABB4 &new_aabb) {
	if (proxy_id < 0 || proxy_id >= (int)_nodes.size()) return false;

	// Check if the new AABB is still inside the fat AABB
	if (_nodes[proxy_id].aabb.encloses(AABB4(new_aabb.min_bound, new_aabb.max_bound))) {
		return false; // No update needed
	}

	_remove_leaf(proxy_id);
	_nodes[proxy_id].aabb = new_aabb.grown(BVH_AABB_MARGIN);
	_insert_leaf(proxy_id);
	return true;
}

void BroadPhase4D::clear() {
	_nodes.clear();
	_free_list.clear();
	_root = BVH_NULL_NODE;
}

int BroadPhase4D::_pick_best_sibling(int leaf) const {
	// Use surface area heuristic to find best sibling
	const AABB4 &leaf_aabb = _nodes[leaf].aabb;

	int best = _root;
	float best_cost = leaf_aabb.merged(_nodes[_root].aabb).surface_area();

	// Simple greedy descent - walk down the tree choosing the child
	// that minimizes the combined surface area
	struct Entry {
		int node;
		float inherited_cost;
	};

	std::vector<Entry> stack;
	stack.push_back({_root, 0.0f});

	while (!stack.empty()) {
		Entry e = stack.back();
		stack.pop_back();

		int node = e.node;
		float inherited = e.inherited_cost;

		AABB4 combined = leaf_aabb.merged(_nodes[node].aabb);
		float direct_cost = combined.surface_area();
		float cost = direct_cost + inherited;

		if (cost < best_cost) {
			best_cost = cost;
			best = node;
		}

		if (!_nodes[node].is_leaf()) {
			float delta = direct_cost - _nodes[node].aabb.surface_area();
			float child_inherited = inherited + delta;

			// Lower bound on cost of inserting into subtree
			float lower_bound = leaf_aabb.surface_area() + child_inherited;
			if (lower_bound < best_cost) {
				stack.push_back({_nodes[node].child_a, child_inherited});
				stack.push_back({_nodes[node].child_b, child_inherited});
			}
		}
	}

	return best;
}

void BroadPhase4D::_insert_leaf(int leaf) {
	if (_root == BVH_NULL_NODE) {
		_root = leaf;
		_nodes[leaf].parent = BVH_NULL_NODE;
		return;
	}

	// Find the best sibling
	int sibling = _pick_best_sibling(leaf);

	// Create a new internal node
	int old_parent = _nodes[sibling].parent;
	int new_parent = _allocate_node();
	_nodes[new_parent].parent = old_parent;
	_nodes[new_parent].aabb = _nodes[leaf].aabb.merged(_nodes[sibling].aabb);
	_nodes[new_parent].height = _nodes[sibling].height + 1;

	if (old_parent != BVH_NULL_NODE) {
		if (_nodes[old_parent].child_a == sibling) {
			_nodes[old_parent].child_a = new_parent;
		} else {
			_nodes[old_parent].child_b = new_parent;
		}
	} else {
		_root = new_parent;
	}

	_nodes[new_parent].child_a = sibling;
	_nodes[new_parent].child_b = leaf;
	_nodes[sibling].parent = new_parent;
	_nodes[leaf].parent = new_parent;

	// Walk up and refit AABBs + rebalance
	_refit_ancestors(new_parent);
}

void BroadPhase4D::_remove_leaf(int leaf) {
	if (leaf == _root) {
		_root = BVH_NULL_NODE;
		return;
	}

	int parent = _nodes[leaf].parent;
	int grandparent = _nodes[parent].parent;
	int sibling = (_nodes[parent].child_a == leaf) ? _nodes[parent].child_b : _nodes[parent].child_a;

	if (grandparent != BVH_NULL_NODE) {
		if (_nodes[grandparent].child_a == parent) {
			_nodes[grandparent].child_a = sibling;
		} else {
			_nodes[grandparent].child_b = sibling;
		}
		_nodes[sibling].parent = grandparent;
		_free_node(parent);
		_refit_ancestors(grandparent);
	} else {
		_root = sibling;
		_nodes[sibling].parent = BVH_NULL_NODE;
		_free_node(parent);
	}
}

void BroadPhase4D::_refit_ancestors(int node_id) {
	int current = node_id;
	while (current != BVH_NULL_NODE) {
		_balance(current);

		int a = _nodes[current].child_a;
		int b = _nodes[current].child_b;
		if (a != BVH_NULL_NODE && b != BVH_NULL_NODE) {
			_nodes[current].aabb = _nodes[a].aabb.merged(_nodes[b].aabb);
			_nodes[current].height = 1 + std::max(_nodes[a].height, _nodes[b].height);
		}

		current = _nodes[current].parent;
	}
}

void BroadPhase4D::_balance(int node_id) {
	// AVL-style tree rotation for balance
	if (_nodes[node_id].is_leaf() || _nodes[node_id].height < 2) return;

	int a = _nodes[node_id].child_a;
	int b = _nodes[node_id].child_b;

	int balance = _nodes[b].height - _nodes[a].height;

	// Rotate B up
	if (balance > 1 && !_nodes[b].is_leaf()) {
		int b1 = _nodes[b].child_a;
		int b2 = _nodes[b].child_b;

		// Swap node_id's child B with one of B's children
		_nodes[b].child_a = node_id;
		_nodes[b].parent = _nodes[node_id].parent;
		_nodes[node_id].parent = b;

		if (_nodes[b].parent != BVH_NULL_NODE) {
			if (_nodes[_nodes[b].parent].child_a == node_id) {
				_nodes[_nodes[b].parent].child_a = b;
			} else {
				_nodes[_nodes[b].parent].child_b = b;
			}
		} else {
			_root = b;
		}

		if (_nodes[b1].height > _nodes[b2].height) {
			_nodes[b].child_b = b1;
			_nodes[node_id].child_b = b2;
			_nodes[b2].parent = node_id;
			_nodes[node_id].aabb = _nodes[a].aabb.merged(_nodes[b2].aabb);
			_nodes[b].aabb = _nodes[node_id].aabb.merged(_nodes[b1].aabb);
			_nodes[node_id].height = 1 + std::max(_nodes[a].height, _nodes[b2].height);
			_nodes[b].height = 1 + std::max(_nodes[node_id].height, _nodes[b1].height);
		} else {
			_nodes[b].child_b = b2;
			_nodes[node_id].child_b = b1;
			_nodes[b1].parent = node_id;
			_nodes[node_id].aabb = _nodes[a].aabb.merged(_nodes[b1].aabb);
			_nodes[b].aabb = _nodes[node_id].aabb.merged(_nodes[b2].aabb);
			_nodes[node_id].height = 1 + std::max(_nodes[a].height, _nodes[b1].height);
			_nodes[b].height = 1 + std::max(_nodes[node_id].height, _nodes[b2].height);
		}
		return;
	}

	// Rotate A up
	if (balance < -1 && !_nodes[a].is_leaf()) {
		int a1 = _nodes[a].child_a;
		int a2 = _nodes[a].child_b;

		_nodes[a].child_a = node_id;
		_nodes[a].parent = _nodes[node_id].parent;
		_nodes[node_id].parent = a;

		if (_nodes[a].parent != BVH_NULL_NODE) {
			if (_nodes[_nodes[a].parent].child_a == node_id) {
				_nodes[_nodes[a].parent].child_a = a;
			} else {
				_nodes[_nodes[a].parent].child_b = a;
			}
		} else {
			_root = a;
		}

		if (_nodes[a1].height > _nodes[a2].height) {
			_nodes[a].child_b = a1;
			_nodes[node_id].child_a = a2;
			_nodes[a2].parent = node_id;
			_nodes[node_id].aabb = _nodes[b].aabb.merged(_nodes[a2].aabb);
			_nodes[a].aabb = _nodes[node_id].aabb.merged(_nodes[a1].aabb);
			_nodes[node_id].height = 1 + std::max(_nodes[b].height, _nodes[a2].height);
			_nodes[a].height = 1 + std::max(_nodes[node_id].height, _nodes[a1].height);
		} else {
			_nodes[a].child_b = a2;
			_nodes[node_id].child_a = a1;
			_nodes[a1].parent = node_id;
			_nodes[node_id].aabb = _nodes[b].aabb.merged(_nodes[a1].aabb);
			_nodes[a].aabb = _nodes[node_id].aabb.merged(_nodes[a2].aabb);
			_nodes[node_id].height = 1 + std::max(_nodes[b].height, _nodes[a1].height);
			_nodes[a].height = 1 + std::max(_nodes[node_id].height, _nodes[a2].height);
		}
	}
}

void BroadPhase4D::query_pairs(std::vector<std::pair<int, int>> &pairs) const {
	if (_root == BVH_NULL_NODE) return;
	_query_pairs_recursive(_root, _root, pairs);
}

void BroadPhase4D::_query_pairs_recursive(int node_a, int node_b,
										   std::vector<std::pair<int, int>> &pairs) const {
	if (node_a == BVH_NULL_NODE || node_b == BVH_NULL_NODE) return;

	if (node_a == node_b) {
		if (_nodes[node_a].is_leaf()) return;
		_query_pairs_recursive(_nodes[node_a].child_a, _nodes[node_a].child_b, pairs);
		_query_pairs_recursive(_nodes[node_a].child_a, _nodes[node_a].child_a, pairs);
		_query_pairs_recursive(_nodes[node_a].child_b, _nodes[node_a].child_b, pairs);
		return;
	}

	if (!_nodes[node_a].aabb.intersects(_nodes[node_b].aabb)) return;

	if (_nodes[node_a].is_leaf() && _nodes[node_b].is_leaf()) {
		int ua = _nodes[node_a].user_data;
		int ub = _nodes[node_b].user_data;
		if (ua != ub) {
			pairs.push_back({std::min(ua, ub), std::max(ua, ub)});
		}
		return;
	}

	// Descend into the larger node
	if (_nodes[node_a].is_leaf() || (!_nodes[node_b].is_leaf() && _nodes[node_a].height < _nodes[node_b].height)) {
		_query_pairs_recursive(node_a, _nodes[node_b].child_a, pairs);
		_query_pairs_recursive(node_a, _nodes[node_b].child_b, pairs);
	} else {
		_query_pairs_recursive(_nodes[node_a].child_a, node_b, pairs);
		_query_pairs_recursive(_nodes[node_a].child_b, node_b, pairs);
	}
}

void BroadPhase4D::query_aabb(const AABB4 &aabb, std::vector<int> &results) const {
	if (_root == BVH_NULL_NODE) return;
	_query_aabb_recursive(_root, aabb, results);
}

void BroadPhase4D::_query_aabb_recursive(int node, const AABB4 &aabb, std::vector<int> &results) const {
	if (node == BVH_NULL_NODE) return;
	if (!_nodes[node].aabb.intersects(aabb)) return;

	if (_nodes[node].is_leaf()) {
		results.push_back(_nodes[node].user_data);
		return;
	}

	_query_aabb_recursive(_nodes[node].child_a, aabb, results);
	_query_aabb_recursive(_nodes[node].child_b, aabb, results);
}

void BroadPhase4D::query_ray(const Vector4 &origin, const Vector4 &dir, float max_t,
							  std::function<bool(int user_data, float t_min, float t_max)> callback) const {
	if (_root == BVH_NULL_NODE) return;

	struct Entry {
		int node;
	};

	std::vector<Entry> stack;
	stack.push_back({_root});

	while (!stack.empty()) {
		Entry e = stack.back();
		stack.pop_back();

		if (e.node == BVH_NULL_NODE) continue;

		float t_min, t_max;
		if (!_nodes[e.node].aabb.intersect_ray(origin, dir, t_min, t_max)) continue;
		if (t_min > max_t) continue;

		if (_nodes[e.node].is_leaf()) {
			if (!callback(_nodes[e.node].user_data, t_min, t_max)) return;
		} else {
			stack.push_back({_nodes[e.node].child_a});
			stack.push_back({_nodes[e.node].child_b});
		}
	}
}
