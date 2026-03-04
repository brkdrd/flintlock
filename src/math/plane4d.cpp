#include "plane4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <cmath>
#include <algorithm>

namespace godot {

Plane4D::Plane4D() {
	normal.instantiate();
	normal->w = 1.0f; // default: W-normal (standard viewing hyperplane)
}

Ref<Plane4D> Plane4D::create(const Ref<Vector4D> &p_normal, real_t p_d) {
	Ref<Plane4D> p;
	p.instantiate();
	p->normal = p_normal;
	p->d = p_d;
	return p;
}

Ref<Plane4D> Plane4D::from_point_normal(const Ref<Vector4D> &p_point, const Ref<Vector4D> &p_normal) {
	return create(p_normal->normalized(), p_normal->normalized()->dot(p_point));
}

real_t Plane4D::distance_to(const Ref<Vector4D> &p_point) const {
	return normal->dot(p_point) - d;
}

bool Plane4D::is_point_over(const Ref<Vector4D> &p_point) const {
	return distance_to(p_point) > 0.0f;
}

Ref<Vector4D> Plane4D::project(const Ref<Vector4D> &p_point) const {
	real_t dist = distance_to(p_point);
	return p_point->subtracted(normal->multiplied_scalar(dist));
}

Ref<Vector4D> Plane4D::intersects_segment(const Ref<Vector4D> &p_a, const Ref<Vector4D> &p_b) const {
	real_t da = distance_to(p_a);
	real_t db = distance_to(p_b);
	// Check crossing
	if ((da > 0) == (db > 0)) return Ref<Vector4D>(); // same side, no intersection
	if (std::abs(da - db) < 1e-8f) return Ref<Vector4D>();
	real_t t = da / (da - db);
	return p_a->lerp(p_b, t);
}

void Plane4D::get_tangent_basis(Ref<Vector4D> &t1, Ref<Vector4D> &t2, Ref<Vector4D> &t3) const {
	// Gram-Schmidt with fixed priority axis order.
	// We pick the world axis least parallel to the normal as the seed.
	static const float axes[4][4] = {
		{1, 0, 0, 0}, // X
		{0, 1, 0, 0}, // Y
		{0, 0, 1, 0}, // Z
		{0, 0, 0, 1}, // W
	};

	// Find axis with smallest |dot| with normal
	int best = 0;
	real_t best_dot = std::abs(normal->x);
	real_t dy = std::abs(normal->y);
	real_t dz = std::abs(normal->z);
	real_t dw = std::abs(normal->w);
	if (dy < best_dot) { best = 1; best_dot = dy; }
	if (dz < best_dot) { best = 2; best_dot = dz; }
	if (dw < best_dot) { best = 3; }

	// Build 3 candidate vectors
	Ref<Vector4D> seed[3];
	int count = 0;
	for (int i = 0; i < 4 && count < 3; i++) {
		if (i == best) continue;
		seed[count++] = Vector4D::_make(axes[i][0], axes[i][1], axes[i][2], axes[i][3]);
	}

	// Gram-Schmidt: orthogonalize against normal, then against each other
	Ref<Vector4D> basis[4];
	basis[0] = normal; // the hyperplane normal (excluded from output)
	basis[1] = seed[0];
	basis[2] = seed[1];
	basis[3] = seed[2];

	// Orthonormalize
	for (int i = 1; i < 4; i++) {
		for (int j = 0; j < i; j++) {
			real_t proj = basis[i]->dot(basis[j]);
			basis[i] = basis[i]->subtracted(basis[j]->multiplied_scalar(proj));
		}
		basis[i] = basis[i]->normalized();
	}

	t1 = basis[1];
	t2 = basis[2];
	t3 = basis[3];
}

Array Plane4D::get_tangent_basis_array() const {
	Ref<Vector4D> t1, t2, t3;
	get_tangent_basis(t1, t2, t3);
	Array a;
	a.push_back(t1);
	a.push_back(t2);
	a.push_back(t3);
	return a;
}

bool Plane4D::is_equal_approx(const Ref<Plane4D> &p_b) const {
	return normal->is_equal_approx(p_b->normal) && std::abs(d - p_b->d) < 1e-5f;
}

String Plane4D::to_string() const {
	return String("[Plane4D]");
}

void Plane4D::_bind_methods() {
	ClassDB::bind_static_method("Plane4D", D_METHOD("create", "normal", "d"), &Plane4D::create);
	ClassDB::bind_static_method("Plane4D", D_METHOD("from_point_normal", "point", "normal"), &Plane4D::from_point_normal);

	ClassDB::bind_method(D_METHOD("get_normal"), &Plane4D::get_normal);
	ClassDB::bind_method(D_METHOD("set_normal", "n"), &Plane4D::set_normal);
	ClassDB::bind_method(D_METHOD("get_d"), &Plane4D::get_d);
	ClassDB::bind_method(D_METHOD("set_d", "v"), &Plane4D::set_d);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "normal", PROPERTY_HINT_RESOURCE_TYPE, "Vector4D"), "set_normal", "get_normal");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "d"), "set_d", "get_d");

	ClassDB::bind_method(D_METHOD("distance_to", "point"), &Plane4D::distance_to);
	ClassDB::bind_method(D_METHOD("is_point_over", "point"), &Plane4D::is_point_over);
	ClassDB::bind_method(D_METHOD("project", "point"), &Plane4D::project);
	ClassDB::bind_method(D_METHOD("intersects_segment", "a", "b"), &Plane4D::intersects_segment);
	ClassDB::bind_method(D_METHOD("get_tangent_basis_array"), &Plane4D::get_tangent_basis_array);
	ClassDB::bind_method(D_METHOD("is_equal_approx", "b"), &Plane4D::is_equal_approx);
	ClassDB::bind_method(D_METHOD("to_string"), &Plane4D::to_string);
}

} // namespace godot
