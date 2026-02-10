#include "broadphase_4d.h"

int Broadphase4D::insert(const AABB4D &p_aabb) {
	int id = next_id++;
	aabbs[id] = p_aabb;
	return id;
}

void Broadphase4D::update(int p_id, const AABB4D &p_aabb) {
	auto it = aabbs.find(p_id);
	if (it != aabbs.end()) {
		it->second = p_aabb;
	}
}

void Broadphase4D::remove(int p_id) {
	aabbs.erase(p_id);
}

std::vector<std::pair<int, int>> Broadphase4D::query_pairs() const {
	std::vector<std::pair<int, int>> pairs;

	// Brute-force O(n²) pair testing
	for (auto it1 = aabbs.begin(); it1 != aabbs.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != aabbs.end(); ++it2) {
			if (it1->second.intersects(it2->second)) {
				int id_a = it1->first;
				int id_b = it2->first;
				// Ensure id_a < id_b
				if (id_a > id_b) {
					std::swap(id_a, id_b);
				}
				pairs.push_back({id_a, id_b});
			}
		}
	}

	return pairs;
}

std::vector<int> Broadphase4D::query_aabb(const AABB4D &p_aabb) const {
	std::vector<int> results;

	for (const auto &entry : aabbs) {
		if (p_aabb.intersects(entry.second)) {
			results.push_back(entry.first);
		}
	}

	return results;
}
