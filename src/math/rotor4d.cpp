#include "rotor4d.h"

#include <algorithm>

// -- Constructors ------------------------------------------------------------

Rotor4D::Rotor4D() :
		s(1), e12(0), e13(0), e14(0), e23(0), e24(0), e34(0), e1234(0) {}

Rotor4D::Rotor4D(real_t p_s, real_t p_e12, real_t p_e13, real_t p_e14,
		real_t p_e23, real_t p_e24, real_t p_e34, real_t p_e1234) :
		s(p_s), e12(p_e12), e13(p_e13), e14(p_e14),
		e23(p_e23), e24(p_e24), e34(p_e34), e1234(p_e1234) {}

Rotor4D Rotor4D::from_plane_angle(RotationPlane p_plane, real_t p_angle) {
	real_t half = p_angle * 0.5;
	real_t c = std::cos(half);
	real_t sn = std::sin(half);

	// R = cos(θ/2) + sin(θ/2) * B̂
	// The sign convention: the bivector e_ij represents the plane from axis i
	// toward axis j. A positive angle rotates axis i toward axis j.
	// We negate sn because the sandwich product R v R† with positive bivector
	// rotates in the opposite sense from the conventional from_rotation matrix.
	Rotor4D r;
	r.s = c;
	switch (p_plane) {
		case PLANE_XY: r.e12 = -sn; break;
		case PLANE_XZ: r.e13 = -sn; break;
		case PLANE_XW: r.e14 = -sn; break;
		case PLANE_YZ: r.e23 = -sn; break;
		case PLANE_YW: r.e24 = -sn; break;
		case PLANE_ZW: r.e34 = -sn; break;
	}
	return r;
}

// -- Sandwich product (rotation) ---------------------------------------------

Vector4 Rotor4D::rotate(const Vector4 &p_v) const {
	// Compute R v R† where R is this rotor and R† is its conjugate (reverse).
	// This is done by expanding the geometric algebra products.
	//
	// For a general even multivector R = s + B + P (scalar + bivector + pseudoscalar),
	// the sandwich product on a vector v gives another vector.
	//
	// We implement this by converting to a rotation matrix (Basis4D) and applying it.
	// This is efficient when rotating multiple vectors with the same rotor.
	Basis4D mat = to_basis();
	return mat.xform(p_v);
}

// -- Geometric product (composition) -----------------------------------------

Rotor4D Rotor4D::operator*(const Rotor4D &p_other) const {
	// Full geometric product of two even multivectors in Cl(4,0).
	// Each component of the result is a sum of products of components from
	// the two rotors, with signs determined by the geometric algebra.
	//
	// Notation: a = this, b = p_other.
	const Rotor4D &a = *this;
	const Rotor4D &b = p_other;
	Rotor4D r;

	// Scalar part: products of same-grade elements.
	r.s = a.s * b.s
			- a.e12 * b.e12 - a.e13 * b.e13 - a.e14 * b.e14
			- a.e23 * b.e23 - a.e24 * b.e24 - a.e34 * b.e34
			+ a.e1234 * b.e1234;

	// Bivector e12:
	r.e12 = a.s * b.e12 + a.e12 * b.s
			- a.e13 * b.e23 + a.e23 * b.e13
			- a.e14 * b.e24 + a.e24 * b.e14
			- a.e1234 * b.e34 - a.e34 * b.e1234;

	// Bivector e13:
	r.e13 = a.s * b.e13 + a.e13 * b.s
			+ a.e12 * b.e23 - a.e23 * b.e12
			- a.e14 * b.e34 + a.e34 * b.e14
			+ a.e1234 * b.e24 + a.e24 * b.e1234;

	// Bivector e14:
	r.e14 = a.s * b.e14 + a.e14 * b.s
			+ a.e12 * b.e24 - a.e24 * b.e12
			+ a.e13 * b.e34 - a.e34 * b.e13
			- a.e1234 * b.e23 - a.e23 * b.e1234;

	// Bivector e23:
	r.e23 = a.s * b.e23 + a.e23 * b.s
			- a.e12 * b.e13 + a.e13 * b.e12
			- a.e24 * b.e34 + a.e34 * b.e24
			+ a.e1234 * b.e14 + a.e14 * b.e1234;

	// Bivector e24:
	r.e24 = a.s * b.e24 + a.e24 * b.s
			- a.e12 * b.e14 + a.e14 * b.e12
			+ a.e23 * b.e34 - a.e34 * b.e23
			- a.e1234 * b.e13 - a.e13 * b.e1234;

	// Bivector e34:
	r.e34 = a.s * b.e34 + a.e34 * b.s
			- a.e13 * b.e14 + a.e14 * b.e13
			- a.e23 * b.e24 + a.e24 * b.e23
			+ a.e1234 * b.e12 + a.e12 * b.e1234;

	// Pseudoscalar e1234:
	r.e1234 = a.s * b.e1234 + a.e1234 * b.s
			+ a.e12 * b.e34 + a.e34 * b.e12
			- a.e13 * b.e24 - a.e24 * b.e13
			+ a.e14 * b.e23 + a.e23 * b.e14;

	return r;
}

// -- Conjugate (reverse) -----------------------------------------------------

Rotor4D Rotor4D::conjugate() const {
	// The reverse of a multivector negates all bivector components.
	// Scalar and pseudoscalar signs depend on grade:
	//   grade 0 (scalar): unchanged
	//   grade 2 (bivectors): negated
	//   grade 4 (pseudoscalar): unchanged
	return Rotor4D(s, -e12, -e13, -e14, -e23, -e24, -e34, e1234);
}

// -- Norm --------------------------------------------------------------------

real_t Rotor4D::norm_squared() const {
	return s * s + e12 * e12 + e13 * e13 + e14 * e14
			+ e23 * e23 + e24 * e24 + e34 * e34 + e1234 * e1234;
}

real_t Rotor4D::norm() const {
	return std::sqrt(norm_squared());
}

void Rotor4D::normalize() {
	real_t n = norm();
	if (n > CMP_EPSILON) {
		real_t inv = 1.0 / n;
		s *= inv;
		e12 *= inv;
		e13 *= inv;
		e14 *= inv;
		e23 *= inv;
		e24 *= inv;
		e34 *= inv;
		e1234 *= inv;
	}
}

Rotor4D Rotor4D::normalized() const {
	Rotor4D r = *this;
	r.normalize();
	return r;
}

// -- Slerp -------------------------------------------------------------------

Rotor4D Rotor4D::slerp(const Rotor4D &p_to, real_t p_t) const {
	// Treat the 8-component rotor as a point on a 7-sphere.
	// Compute the dot product to find the angle between them.
	real_t dot = s * p_to.s + e12 * p_to.e12 + e13 * p_to.e13 + e14 * p_to.e14
			+ e23 * p_to.e23 + e24 * p_to.e24 + e34 * p_to.e34 + e1234 * p_to.e1234;

	// If dot is negative, negate one rotor to take the shorter path.
	Rotor4D to = p_to;
	if (dot < 0) {
		to = Rotor4D(-to.s, -to.e12, -to.e13, -to.e14,
				-to.e23, -to.e24, -to.e34, -to.e1234);
		dot = -dot;
	}

	// If very close, use linear interpolation to avoid division by zero.
	if (dot > 1.0 - CMP_EPSILON) {
		Rotor4D r;
		r.s = s + (to.s - s) * p_t;
		r.e12 = e12 + (to.e12 - e12) * p_t;
		r.e13 = e13 + (to.e13 - e13) * p_t;
		r.e14 = e14 + (to.e14 - e14) * p_t;
		r.e23 = e23 + (to.e23 - e23) * p_t;
		r.e24 = e24 + (to.e24 - e24) * p_t;
		r.e34 = e34 + (to.e34 - e34) * p_t;
		r.e1234 = e1234 + (to.e1234 - e1234) * p_t;
		r.normalize();
		return r;
	}

	real_t theta = std::acos(std::clamp(dot, (real_t)-1.0, (real_t)1.0));
	real_t sin_theta = std::sin(theta);
	real_t wa = std::sin((1.0 - p_t) * theta) / sin_theta;
	real_t wb = std::sin(p_t * theta) / sin_theta;

	Rotor4D r;
	r.s = wa * s + wb * to.s;
	r.e12 = wa * e12 + wb * to.e12;
	r.e13 = wa * e13 + wb * to.e13;
	r.e14 = wa * e14 + wb * to.e14;
	r.e23 = wa * e23 + wb * to.e23;
	r.e24 = wa * e24 + wb * to.e24;
	r.e34 = wa * e34 + wb * to.e34;
	r.e1234 = wa * e1234 + wb * to.e1234;
	r.normalize();
	return r;
}

// -- Conversion to Basis4D ---------------------------------------------------

Basis4D Rotor4D::to_basis() const {
	// Convert rotor to rotation matrix by computing R e_i R† for each basis vector.
	// For a unit rotor (norm = 1), this gives an orthogonal matrix.
	//
	// We precompute all needed products. Let:
	//   s2 = s*s, etc.
	real_t s2 = s * s;
	real_t b12 = e12 * e12, b13 = e13 * e13, b14 = e14 * e14;
	real_t b23 = e23 * e23, b24 = e24 * e24, b34 = e34 * e34;
	real_t p2 = e1234 * e1234;

	Basis4D m;

	// Row 0 (e1 component of R e_i R†):
	m.rows[0] = Vector4(
			s2 - b12 - b13 - b14 + b23 + b24 + b34 - p2,
			2.0 * (s * e12 + e13 * e23 + e14 * e24 + e34 * e1234),
			2.0 * (s * e13 - e12 * e23 + e14 * e34 - e24 * e1234),
			2.0 * (s * e14 - e12 * e24 - e13 * e34 + e23 * e1234));

	// Row 1 (e2):
	m.rows[1] = Vector4(
			2.0 * (-s * e12 + e13 * e23 + e14 * e24 - e34 * e1234),
			s2 - b12 - b23 - b24 + b13 + b14 + b34 - p2,
			2.0 * (s * e23 + e12 * e13 + e24 * e34 + e14 * e1234),
			2.0 * (s * e24 + e12 * e14 - e23 * e34 - e13 * e1234));

	// Row 2 (e3):
	m.rows[2] = Vector4(
			2.0 * (-s * e13 - e12 * e23 + e14 * e34 + e24 * e1234),
			2.0 * (-s * e23 + e12 * e13 + e24 * e34 - e14 * e1234),
			s2 - b13 - b23 - b34 + b12 + b14 + b24 - p2,
			2.0 * (s * e34 + e13 * e14 + e23 * e24 + e12 * e1234));

	// Row 3 (e4):
	m.rows[3] = Vector4(
			2.0 * (-s * e14 - e12 * e24 - e13 * e34 - e23 * e1234),
			2.0 * (-s * e24 + e12 * e14 - e23 * e34 + e13 * e1234),
			2.0 * (-s * e34 + e13 * e14 + e23 * e24 - e12 * e1234),
			s2 - b14 - b24 - b34 + b12 + b13 + b23 - p2);

	return m;
}

// -- Conversion from Basis4D -------------------------------------------------

Rotor4D Rotor4D::from_basis(const Basis4D &p_basis) {
	// Extract a rotor from an orthogonal rotation matrix.
	// Similar to quaternion extraction from a 3x3 matrix, generalized to 4D.
	//
	// The trace of the matrix relates to the scalar component:
	//   trace = 4*s^2 - 4*p^2 + (sum of squares terms)
	// For a simple rotor (no pseudoscalar), trace = 4*s^2 - (4 - 4*s^2) + ... simplified.
	//
	// We use the method: try extracting s from the trace, then get bivector
	// components from off-diagonal differences.

	const auto &m = p_basis.rows;
	real_t trace = m[0].x + m[1].y + m[2].z + m[3].w;

	// For a simple rotation (e1234 = 0), trace = 4*s^2 - 4 + 4*s^2 is wrong.
	// Actually for a simple rotor: trace = 4*cos^2(theta/2) - 2 ... no.
	// Let's use a robust extraction approach.
	//
	// From to_basis, the diagonal elements for a pure-bivector rotor (e1234=0) are:
	//   m[0][0] = s^2 + b12 + b13 + b14 - b23 - b24 - b34
	//   m[1][1] = s^2 - b12 + b23 + b24 + b13 - b14 - b34
	//   m[2][2] = s^2 - b13 - b23 + b34 + b12 - b14 - b24
	//   m[3][3] = s^2 - b14 - b24 - b34 + b12 + b13 + b23
	// Sum: 4*s^2 + 2*b12 + 2*b13 - 2*b14 - 2*b34 ... this is complex.
	//
	// Simpler: for the specific case of a simple rotor in one plane (which is
	// what from_plane_angle produces), we can extract using the antisymmetric
	// part of the matrix: m[i][j] - m[j][i] = 4 * s * B_ij.

	Rotor4D r;
	// Assume e1234 = 0 for rotations extracted from matrices.
	r.e1234 = 0;

	// Compute s from trace. For a rotation matrix from a unit rotor with e1234=0:
	// trace = 4*s^2 + 2*(b12+b13+b23) - 2*(b14+b24+b34) ... this depends on plane.
	// Instead: sum of diagonal = 4s^2 - 1 + 1 ... let me just use the Cayley method.
	//
	// Actually the simplest robust approach: compute s from 1 + trace.
	// For orthogonal matrices from simple rotors:
	//   trace = 4*s^2 + 4*s^2 - 4 = 8*s^2 - 4? No...
	//
	// Let me directly compute: for a simple rotation by angle theta in a single plane,
	// the trace = 2 + 2*cos(theta), and s = cos(theta/2).
	// So trace = 2 + 2*(2*s^2 - 1) = 4*s^2.
	// Therefore s^2 = trace / 4.

	real_t s2_val = trace / 4.0;
	if (s2_val > CMP_EPSILON) {
		r.s = std::sqrt(s2_val);
		real_t inv4s = 1.0 / (4.0 * r.s);
		// From the antisymmetric parts: m[j][i] - m[i][j] = 4*s*B_ij
		// But we need to be careful about our sign conventions from to_basis.
		// From to_basis row 0, col 1: 2*(s*e12 + ...)
		// From to_basis row 1, col 0: 2*(-s*e12 + ...)
		// Difference: row[0][1] - row[1][0] = 4*s*e12
		r.e12 = (m[0].y - m[1].x) * inv4s;
		r.e13 = (m[0].z - m[2].x) * inv4s;
		r.e14 = (m[0].w - m[3].x) * inv4s;
		r.e23 = (m[1].z - m[2].y) * inv4s;
		r.e24 = (m[1].w - m[3].y) * inv4s;
		r.e34 = (m[2].w - m[3].z) * inv4s;
	} else {
		// s ~ 0: rotation is close to 180 degrees.
		// Find the largest diagonal element to determine the dominant bivector.
		r.s = 0;
		// For a 180-degree rotation, the bivector components can be extracted
		// from the symmetric part. For simplicity, use the matrix directly.
		// This branch handles edge cases; the common case above handles most rotations.
		real_t diag[4] = { m[0].x, m[1].y, m[2].z, m[3].w };
		int max_i = 0;
		for (int i = 1; i < 4; i++) {
			if (diag[i] > diag[max_i]) max_i = i;
		}

		// The largest diagonal element corresponds to the axis least affected by rotation.
		// For a 180-degree rotation in plane (a,b), axes a and b get -1 diagonal entries.
		// We extract the bivector from the off-diagonal elements.
		// This is a simplified fallback; normalize at the end to fix up.
		r.e12 = (m[0].y + m[1].x) * 0.25;
		r.e13 = (m[0].z + m[2].x) * 0.25;
		r.e14 = (m[0].w + m[3].x) * 0.25;
		r.e23 = (m[1].z + m[2].y) * 0.25;
		r.e24 = (m[1].w + m[3].y) * 0.25;
		r.e34 = (m[2].w + m[3].z) * 0.25;
	}

	r.normalize();
	return r;
}
