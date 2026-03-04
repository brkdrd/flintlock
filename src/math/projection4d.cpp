#include "projection4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <cmath>
#include <cstring>

namespace godot {

Projection4D::Projection4D() {
	memset(data, 0, sizeof(data));
	for (int i = 0; i < 5; i++) data[i][i] = 1.0f;
}

Ref<Projection4D> Projection4D::create_identity() {
	Ref<Projection4D> p;
	p.instantiate();
	return p;
}

real_t Projection4D::get_element(int r, int c) const {
	if (r < 0 || r > 4 || c < 0 || c > 4) return 0.0f;
	return data[c][r];
}

void Projection4D::set_element(int r, int c, real_t v) {
	if (r < 0 || r > 4 || c < 0 || c > 4) return;
	data[c][r] = v;
}

bool Projection4D::is_equal_approx(const Ref<Projection4D> &p_b) const {
	for (int c = 0; c < 5; c++)
		for (int r = 0; r < 5; r++)
			if (std::abs(data[c][r] - p_b->data[c][r]) > 1e-5f) return false;
	return true;
}

String Projection4D::to_string() const {
	return String("[Projection4D]");
}

void Projection4D::_bind_methods() {
	ClassDB::bind_static_method("Projection4D", D_METHOD("create_identity"), &Projection4D::create_identity);
	ClassDB::bind_method(D_METHOD("get_element", "row", "col"), &Projection4D::get_element);
	ClassDB::bind_method(D_METHOD("set_element", "row", "col", "value"), &Projection4D::set_element);
	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &Projection4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("to_string"), &Projection4D::to_string);
}

} // namespace godot
