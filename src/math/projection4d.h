#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include "vector4d.h"

namespace godot {

// 5x5 projection matrix - analog of Godot's Projection (4x4) but for 4D->3D.
// Stored as 5 column vectors each of length 5.
// Kept for completeness; the main 4D->3D "projection" is hyperplane slicing.
class Projection4D : public RefCounted {
	GDCLASS(Projection4D, RefCounted);

public:
	real_t data[5][5];

	Projection4D();

	static Ref<Projection4D> create_identity();

	real_t get_element(int p_row, int p_col) const;
	void   set_element(int p_row, int p_col, real_t p_value);

	bool is_equal_approx(const Ref<Projection4D> &p_b) const;
	String to_string() const;

protected:
	static void _bind_methods();
};

} // namespace godot
