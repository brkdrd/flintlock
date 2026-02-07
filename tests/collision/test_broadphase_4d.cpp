#include <doctest/doctest.h>
#include "godot_compat.h"
#include "collision/broadphase_4d.h"

TEST_SUITE("Broadphase4D") {

TEST_CASE("Two overlapping AABBs are reported as a pair") {
	Broadphase4D bp;
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	AABB4D b(Vector4(1, 1, 1, 1), Vector4(2, 2, 2, 2));

	int id_a = bp.insert(a);
	int id_b = bp.insert(b);

	auto pairs = bp.query_pairs();
	bool found = false;
	for (auto &pair : pairs) {
		if ((pair.first == id_a && pair.second == id_b) ||
			(pair.first == id_b && pair.second == id_a)) {
			found = true;
			break;
		}
	}
	CHECK(found);
}

TEST_CASE("Two separated AABBs are not reported") {
	Broadphase4D bp;
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	AABB4D b(Vector4(10, 10, 10, 10), Vector4(1, 1, 1, 1));

	bp.insert(a);
	bp.insert(b);

	auto pairs = bp.query_pairs();
	CHECK(pairs.empty());
}

TEST_CASE("Update moves an AABB and changes overlap status") {
	Broadphase4D bp;
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(1, 1, 1, 1));
	AABB4D b(Vector4(10, 10, 10, 10), Vector4(1, 1, 1, 1));

	int id_a = bp.insert(a);
	int id_b = bp.insert(b);

	// Initially separated.
	CHECK(bp.query_pairs().empty());

	// Move b to overlap with a.
	bp.update(id_b, AABB4D(Vector4(0.5, 0.5, 0.5, 0.5), Vector4(1, 1, 1, 1)));
	auto pairs = bp.query_pairs();
	CHECK_FALSE(pairs.empty());
}

TEST_CASE("Remove eliminates the AABB from future queries") {
	Broadphase4D bp;
	AABB4D a(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2));
	AABB4D b(Vector4(1, 1, 1, 1), Vector4(2, 2, 2, 2));

	int id_a = bp.insert(a);
	int id_b = bp.insert(b);

	CHECK_FALSE(bp.query_pairs().empty());

	bp.remove(id_a);
	CHECK(bp.query_pairs().empty());
}

TEST_CASE("query_aabb returns all overlapping ids") {
	Broadphase4D bp;
	int id1 = bp.insert(AABB4D(Vector4(0, 0, 0, 0), Vector4(2, 2, 2, 2)));
	int id2 = bp.insert(AABB4D(Vector4(1, 1, 1, 1), Vector4(2, 2, 2, 2)));
	int id3 = bp.insert(AABB4D(Vector4(100, 100, 100, 100), Vector4(1, 1, 1, 1)));

	AABB4D query(Vector4(0.5, 0.5, 0.5, 0.5), Vector4(1, 1, 1, 1));
	auto results = bp.query_aabb(query);

	bool has_id1 = false, has_id2 = false, has_id3 = false;
	for (int id : results) {
		if (id == id1) has_id1 = true;
		if (id == id2) has_id2 = true;
		if (id == id3) has_id3 = true;
	}

	CHECK(has_id1);
	CHECK(has_id2);
	CHECK_FALSE(has_id3);
}

} // TEST_SUITE
