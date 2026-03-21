#pragma once

#include "physics_types_4d.h"
#include <vector>
#include <functional>

// ============================================================================
// Dynamic BVH (Bounding Volume Hierarchy) for 4D broadphase collision detection.
// Uses AABB4 nodes with surface area heuristic for tree construction.
// Supports insert, remove, update, and pair query operations.
// ============================================================================

static constexpr int BVH_NULL_NODE = -1;
static constexpr float BVH_AABB_MARGIN = 0.1f; // Fat AABB margin

struct BVHNode4D {
	AABB4 aabb;          // Fat AABB (includes margin)
	int parent = BVH_NULL_NODE;
	int child_a = BVH_NULL_NODE;
	int child_b = BVH_NULL_NODE;
	int height = 0;
	int user_data = -1;  // External ID (body/area index). -1 for internal nodes.

	bool is_leaf() const { return child_a == BVH_NULL_NODE; }
};

class BroadPhase4D {
public:
	// Insert an AABB, returns the node index (proxy ID)
	int insert(const AABB4 &aabb, int user_data);

	// Remove a node by proxy ID
	void remove(int proxy_id);

	// Update an AABB. Returns true if the fat AABB needed to be enlarged.
	bool update(int proxy_id, const AABB4 &new_aabb);

	// Query all pairs of overlapping leaf nodes
	void query_pairs(std::vector<std::pair<int, int>> &pairs) const;

	// Query all nodes overlapping a given AABB
	void query_aabb(const AABB4 &aabb, std::vector<int> &results) const;

	// Query ray against the tree. Callback receives user_data and should return
	// true to continue, false to stop.
	void query_ray(const Vector4 &origin, const Vector4 &dir, float max_t,
				   std::function<bool(int user_data, float t_min, float t_max)> callback) const;

	// Get the AABB for a proxy
	const AABB4 &get_aabb(int proxy_id) const { return _nodes[proxy_id].aabb; }
	int get_user_data(int proxy_id) const { return _nodes[proxy_id].user_data; }

	// Clear all nodes
	void clear();

	int get_root() const { return _root; }

private:
	std::vector<BVHNode4D> _nodes;
	int _root = BVH_NULL_NODE;
	std::vector<int> _free_list;

	int _allocate_node();
	void _free_node(int node_id);
	void _insert_leaf(int leaf);
	void _remove_leaf(int leaf);
	void _balance(int node_id);
	int _pick_best_sibling(int leaf) const;
	void _refit_ancestors(int node_id);

	void _query_pairs_recursive(int node_a, int node_b,
								std::vector<std::pair<int, int>> &pairs) const;
	void _query_aabb_recursive(int node, const AABB4 &aabb, std::vector<int> &results) const;
};
