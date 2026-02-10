#pragma once

#include "math/aabb4d.h"
#include <vector>
#include <utility>
#include <map>

/// Broadphase4D — Fast AABB-based collision pair generation.
///
/// Uses axis-aligned bounding boxes to quickly identify potentially
/// colliding pairs. Reduces the number of expensive narrowphase tests.
///
/// Implementation: Simple O(n²) brute-force check.
/// Future optimization: Sweep-and-prune or BVH4D.
class Broadphase4D {
public:
	Broadphase4D() = default;

	/// Insert an AABB and return its ID
	int insert(const AABB4D &p_aabb);

	/// Update an existing AABB
	void update(int p_id, const AABB4D &p_aabb);

	/// Remove an AABB by ID
	void remove(int p_id);

	/// Query all overlapping pairs
	/// Returns list of (id_a, id_b) where id_a < id_b
	std::vector<std::pair<int, int>> query_pairs() const;

	/// Query all AABBs overlapping the given AABB
	/// Returns list of IDs
	std::vector<int> query_aabb(const AABB4D &p_aabb) const;

private:
	struct Entry {
		int id;
		AABB4D aabb;
	};

	std::map<int, AABB4D> aabbs;
	int next_id = 1;
};
