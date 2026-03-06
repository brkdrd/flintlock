#include "mesh_4d.h"
#include <godot_cpp/core/class_db.hpp>
#include <cmath>

// ============================================================
// Mesh4D
// ============================================================
void Mesh4D::_bind_methods() {
	BIND_ENUM_CONSTANT(ARRAY_VERTEX);
	BIND_ENUM_CONSTANT(ARRAY_NORMAL);
	BIND_ENUM_CONSTANT(ARRAY_TANGENT);
	BIND_ENUM_CONSTANT(ARRAY_TEX_UV);
	BIND_ENUM_CONSTANT(ARRAY_COLOR);
	BIND_ENUM_CONSTANT(ARRAY_INDEX);
	BIND_ENUM_CONSTANT(ARRAY_MAX);

	ClassDB::bind_method(D_METHOD("get_surface_count"), &Mesh4D::get_surface_count);
	ClassDB::bind_method(D_METHOD("get_surface_arrays", "surface"), &Mesh4D::get_surface_arrays);
	ClassDB::bind_method(D_METHOD("get_surface_material", "surface"), &Mesh4D::get_surface_material);
}

// ============================================================
// ArrayMesh4D
// ============================================================
void ArrayMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_surface", "arrays"), &ArrayMesh4D::add_surface);
	ClassDB::bind_method(D_METHOD("clear_surfaces"), &ArrayMesh4D::clear_surfaces);
	ClassDB::bind_method(D_METHOD("get_surface_count"), &ArrayMesh4D::get_surface_count);
	ClassDB::bind_method(D_METHOD("get_surface_arrays", "surface"), &ArrayMesh4D::get_surface_arrays);
	ClassDB::bind_method(D_METHOD("get_surface_material", "surface"), &ArrayMesh4D::get_surface_material);
	ClassDB::bind_method(D_METHOD("set_surface_material", "surface", "material"), &ArrayMesh4D::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_name", "surface"), &ArrayMesh4D::get_surface_name);
	ClassDB::bind_method(D_METHOD("set_surface_name", "surface", "name"), &ArrayMesh4D::set_surface_name);
}

void ArrayMesh4D::add_surface(const Array &p_arrays) {
	ERR_FAIL_COND(p_arrays.size() != ARRAY_MAX);
	SurfaceData sd;
	sd.arrays = p_arrays;
	_surfaces.push_back(sd);
	emit_changed();
}

void ArrayMesh4D::clear_surfaces() {
	_surfaces.clear();
	emit_changed();
}

int ArrayMesh4D::get_surface_count() const { return _surfaces.size(); }

Array ArrayMesh4D::get_surface_arrays(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), Array());
	return _surfaces[p_surface].arrays;
}

Ref<Material> ArrayMesh4D::get_surface_material(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), Ref<Material>());
	return _surfaces[p_surface].material;
}

void ArrayMesh4D::set_surface_material(int p_surface, const Ref<Material> &p_material) {
	ERR_FAIL_INDEX(p_surface, _surfaces.size());
	_surfaces.write[p_surface].material = p_material;
	emit_changed();
}

String ArrayMesh4D::get_surface_name(int p_surface) const {
	ERR_FAIL_INDEX_V(p_surface, _surfaces.size(), String());
	return _surfaces[p_surface].name;
}

void ArrayMesh4D::set_surface_name(int p_surface, const String &p_name) {
	ERR_FAIL_INDEX(p_surface, _surfaces.size());
	_surfaces.write[p_surface].name = p_name;
}

// ============================================================
// PrimitiveMesh4D
// ============================================================
void PrimitiveMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_surface_count"), &PrimitiveMesh4D::get_surface_count);
	ClassDB::bind_method(D_METHOD("get_surface_arrays", "surface"), &PrimitiveMesh4D::get_surface_arrays);
	ClassDB::bind_method(D_METHOD("get_surface_material", "surface"), &PrimitiveMesh4D::get_surface_material);
	ClassDB::bind_method(D_METHOD("get_material"), &PrimitiveMesh4D::get_material);
	ClassDB::bind_method(D_METHOD("set_surface_material", "material"), &PrimitiveMesh4D::set_surface_material);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "material", PROPERTY_HINT_RESOURCE_TYPE, "Material"), "set_surface_material", "get_material");
}

Array PrimitiveMesh4D::get_surface_arrays(int p_surface) const {
	if (p_surface != 0) return Array();
	if (_dirty) {
		_cached_arrays = _create_mesh_array();
		_dirty = false;
	}
	return _cached_arrays;
}

// ============================================================
// HyperBoxMesh4D - Tesseract mesh generation
// ============================================================
void HyperBoxMesh4D::set_size(const Vector4 &p_size) { _size = p_size; _request_update(); }

void HyperBoxMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_size"), &HyperBoxMesh4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &HyperBoxMesh4D::set_size);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR4, "size"), "set_size", "get_size");
}

// Tesseract: 16 vertices at (+/-hx, +/-hy, +/-hz, +/-hw).
// The boundary consists of 8 cubic cells (3-faces). Each cubic cell
// is decomposed into 6 tetrahedra using the Kuhn/Freudenthal method,
// giving 48 tetrahedra total.
Array HyperBoxMesh4D::_create_mesh_array() const {
	float hx = _size.x * 0.5f, hy = _size.y * 0.5f;
	float hz = _size.z * 0.5f, hw = _size.w * 0.5f;

	// 16 vertices of the tesseract
	PackedFloat32Array vertices;
	vertices.resize(16 * 4);

	int idx = 0;
	for (int w = 0; w < 2; w++) {
		for (int z = 0; z < 2; z++) {
			for (int y = 0; y < 2; y++) {
				for (int x = 0; x < 2; x++) {
					vertices[idx++] = x ? hx : -hx;
					vertices[idx++] = y ? hy : -hy;
					vertices[idx++] = z ? hz : -hz;
					vertices[idx++] = w ? hw : -hw;
				}
			}
		}
	}

	// Vertex index: v(x,y,z,w) = w*8 + z*4 + y*2 + x (x,y,z,w in {0,1})
	auto V = [](int x, int y, int z, int w) -> int {
		return w * 8 + z * 4 + y * 2 + x;
	};

	// Normals: 4D outward-pointing normals per vertex (accumulated from faces)
	PackedFloat32Array normals;
	normals.resize(16 * 4);
	for (int i = 0; i < 16 * 4; i++) normals[i] = 0.0f;

	// 6 permutations of 3 axes for Kuhn decomposition of each cube
	static const int perms3[6][3] = {
		{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
	};

	// The 8 cubic cells of a tesseract.
	// Each cell is obtained by fixing one axis to 0 or 1.
	// The 3 free axes index the 8 vertices of the cube.
	// face_axis: which of {x,y,z,w} is fixed. face_val: 0 or 1.
	// free_axes[3]: the 3 varying axes.
	PackedInt32Array indices;

	for (int face_axis = 0; face_axis < 4; face_axis++) {
		for (int face_val = 0; face_val < 2; face_val++) {
			// Determine the 3 free axes
			int free_axes[3];
			int fi = 0;
			for (int a = 0; a < 4; a++) {
				if (a != face_axis) free_axes[fi++] = a;
			}

			// Outward normal for this face
			float ndir = (face_val == 0) ? -1.0f : 1.0f;

			// Build the 8 vertices of this cube
			// coords[dx][dy][dz] where dx,dy,dz in {0,1} vary over free_axes
			int cube_v[2][2][2];
			for (int d0 = 0; d0 < 2; d0++) {
				for (int d1 = 0; d1 < 2; d1++) {
					for (int d2 = 0; d2 < 2; d2++) {
						int c[4] = {0,0,0,0};
						c[face_axis] = face_val;
						c[free_axes[0]] = d0;
						c[free_axes[1]] = d1;
						c[free_axes[2]] = d2;
						int vi = V(c[0], c[1], c[2], c[3]);
						cube_v[d0][d1][d2] = vi;

						// Add outward normal contribution
						normals[vi * 4 + face_axis] += ndir;
					}
				}
			}

			// Decompose cube into 6 tetrahedra using Kuhn method
			for (int t = 0; t < 6; t++) {
				int p[3] = {0, 0, 0};
				indices.push_back(cube_v[0][0][0]);
				for (int s = 0; s < 3; s++) {
					p[perms3[t][s]] = 1;
					indices.push_back(cube_v[p[0]][p[1]][p[2]]);
				}
			}
		}
	}

	// Normalize the accumulated normals
	for (int i = 0; i < 16; i++) {
		float nx = normals[i*4], ny = normals[i*4+1], nz = normals[i*4+2], nw = normals[i*4+3];
		float len = sqrtf(nx*nx + ny*ny + nz*nz + nw*nw);
		if (len > 1e-8f) {
			normals[i*4] /= len; normals[i*4+1] /= len;
			normals[i*4+2] /= len; normals[i*4+3] /= len;
		}
	}

	Array arrays;
	arrays.resize(Mesh4D::ARRAY_MAX);
	arrays[Mesh4D::ARRAY_VERTEX] = vertices;
	arrays[Mesh4D::ARRAY_NORMAL] = normals;
	arrays[Mesh4D::ARRAY_INDEX] = indices;
	return arrays;
}

// ============================================================
// HyperSphereMesh4D - simplified sphere (UV-sphere analog)
// ============================================================
void HyperSphereMesh4D::set_radius(real_t p_radius) { _radius = p_radius; _request_update(); }
void HyperSphereMesh4D::set_subdivisions(int p_subdivisions) { _subdivisions = p_subdivisions; _request_update(); }

void HyperSphereMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &HyperSphereMesh4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &HyperSphereMesh4D::set_radius);
	ClassDB::bind_method(D_METHOD("get_subdivisions"), &HyperSphereMesh4D::get_subdivisions);
	ClassDB::bind_method(D_METHOD("set_subdivisions", "subdivisions"), &HyperSphereMesh4D::set_subdivisions);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "subdivisions", PROPERTY_HINT_RANGE, "1,5,1"), "set_subdivisions", "get_subdivisions");
}

// Simplified 3-sphere mesh using parameterization with 3 angles.
Array HyperSphereMesh4D::_create_mesh_array() const {
	// Use parameterization:
	// x = r * sin(phi1) * sin(phi2) * cos(theta)
	// y = r * sin(phi1) * sin(phi2) * sin(theta)
	// z = r * sin(phi1) * cos(phi2)
	// w = r * cos(phi1)
	// phi1, phi2 in [0,pi], theta in [0,2pi]

	int rings1 = 4 * _subdivisions;
	int rings2 = 4 * _subdivisions;
	int segments = 8 * _subdivisions;

	PackedFloat32Array vertices;
	PackedFloat32Array normals;
	PackedInt32Array indices;

	// Generate vertices
	for (int i = 0; i <= rings1; i++) {
		float phi1 = (float)i / rings1 * Math_PI;
		for (int j = 0; j <= rings2; j++) {
			float phi2 = (float)j / rings2 * Math_PI;
			for (int k = 0; k <= segments; k++) {
				float theta = (float)k / segments * Math_TAU;
				float x = sinf(phi1) * sinf(phi2) * cosf(theta);
				float y = sinf(phi1) * sinf(phi2) * sinf(theta);
				float z = sinf(phi1) * cosf(phi2);
				float w = cosf(phi1);
				vertices.push_back(x * _radius);
				vertices.push_back(y * _radius);
				vertices.push_back(z * _radius);
				vertices.push_back(w * _radius);
				normals.push_back(x);
				normals.push_back(y);
				normals.push_back(z);
				normals.push_back(w);
			}
		}
	}

	// Generate indices (tetrahedra) using 3D Kuhn/Freudenthal decomposition.
	// Each cell in the (phi1, phi2, theta) parameter grid is a 3D cube
	// decomposed into 6 tetrahedra via the 6 permutations of {0,1,2}.
	int segs1 = segments + 1;
	int segs2 = rings2 + 1;

	// 6 permutations of {0,1,2} for the Kuhn decomposition
	static const int perms3[6][3] = {
		{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
	};

	for (int i = 0; i < rings1; i++) {
		for (int j = 0; j < rings2; j++) {
			for (int k = 0; k < segments; k++) {
				// 8 vertices of the cell: c[di][dj][dk]
				int c[2][2][2];
				for (int di = 0; di < 2; di++)
					for (int dj = 0; dj < 2; dj++)
						for (int dk = 0; dk < 2; dk++)
							c[di][dj][dk] = (i + di) * segs2 * segs1 + (j + dj) * segs1 + (k + dk);

				// 6 tetrahedra per cell
				for (int t = 0; t < 6; t++) {
					int p[3] = {0, 0, 0};
					indices.push_back(c[0][0][0]);
					for (int s = 0; s < 3; s++) {
						p[perms3[t][s]] = 1;
						indices.push_back(c[p[0]][p[1]][p[2]]);
					}
				}
			}
		}
	}

	Array arrays;
	arrays.resize(Mesh4D::ARRAY_MAX);
	arrays[Mesh4D::ARRAY_VERTEX] = vertices;
	arrays[Mesh4D::ARRAY_NORMAL] = normals;
	arrays[Mesh4D::ARRAY_INDEX] = indices;
	return arrays;
}

// ============================================================
// HyperCylinderMesh4D
// ============================================================
void HyperCylinderMesh4D::set_radius(real_t p_radius) { _radius = p_radius; _request_update(); }
void HyperCylinderMesh4D::set_height(real_t p_height) { _height = p_height; _request_update(); }
void HyperCylinderMesh4D::set_radial_segments(int p_segments) { _radial_segments = p_segments; _request_update(); }
void HyperCylinderMesh4D::set_rings(int p_rings) { _rings = p_rings; _request_update(); }

void HyperCylinderMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &HyperCylinderMesh4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &HyperCylinderMesh4D::set_radius);
	ClassDB::bind_method(D_METHOD("get_height"), &HyperCylinderMesh4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &HyperCylinderMesh4D::set_height);
	ClassDB::bind_method(D_METHOD("get_radial_segments"), &HyperCylinderMesh4D::get_radial_segments);
	ClassDB::bind_method(D_METHOD("set_radial_segments", "segments"), &HyperCylinderMesh4D::set_radial_segments);
	ClassDB::bind_method(D_METHOD("get_rings"), &HyperCylinderMesh4D::get_rings);
	ClassDB::bind_method(D_METHOD("set_rings", "rings"), &HyperCylinderMesh4D::set_rings);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments"), "set_radial_segments", "get_radial_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rings"), "set_rings", "get_rings");
}

// 4D cylinder: 3-sphere cross-section extruded along W axis
Array HyperCylinderMesh4D::_create_mesh_array() const {
	PackedFloat32Array vertices, normals;
	PackedInt32Array indices;

	float half_h = _height * 0.5f;
	int segs_phi = _radial_segments;
	int segs_theta = _radial_segments;
	int rings = _rings;

	// Generate vertices on each W ring
	for (int w = 0; w <= rings; w++) {
		float wv = -half_h + (float)w / rings * _height;
		for (int i = 0; i <= segs_phi; i++) {
			float phi = (float)i / segs_phi * Math_PI;
			for (int j = 0; j <= segs_theta; j++) {
				float theta = (float)j / segs_theta * Math_TAU;
				float x = sinf(phi) * cosf(theta);
				float y = sinf(phi) * sinf(theta);
				float z = cosf(phi);
				vertices.push_back(x * _radius);
				vertices.push_back(y * _radius);
				vertices.push_back(z * _radius);
				vertices.push_back(wv);
				normals.push_back(x);
				normals.push_back(y);
				normals.push_back(z);
				normals.push_back(0.0f);
			}
		}
	}

	static const int perms3[6][3] = {
		{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
	};

	int row_size = (segs_phi + 1) * (segs_theta + 1);
	for (int w = 0; w < rings; w++) {
		for (int i = 0; i < segs_phi; i++) {
			for (int j = 0; j < segs_theta; j++) {
				// 8 vertices of the cell: c[dw][di][dj]
				int c[2][2][2];
				for (int dw = 0; dw < 2; dw++)
					for (int di = 0; di < 2; di++)
						for (int dj = 0; dj < 2; dj++)
							c[dw][di][dj] = (w + dw) * row_size + (i + di) * (segs_theta + 1) + (j + dj);

				for (int t = 0; t < 6; t++) {
					int p[3] = {0, 0, 0};
					indices.push_back(c[0][0][0]);
					for (int s = 0; s < 3; s++) {
						p[perms3[t][s]] = 1;
						indices.push_back(c[p[0]][p[1]][p[2]]);
					}
				}
			}
		}
	}

	Array arrays;
	arrays.resize(Mesh4D::ARRAY_MAX);
	arrays[Mesh4D::ARRAY_VERTEX] = vertices;
	arrays[Mesh4D::ARRAY_NORMAL] = normals;
	arrays[Mesh4D::ARRAY_INDEX] = indices;
	return arrays;
}

// ============================================================
// HyperCapsuleMesh4D
// ============================================================
void HyperCapsuleMesh4D::set_radius(real_t p_radius) { _radius = p_radius; _request_update(); }
void HyperCapsuleMesh4D::set_height(real_t p_height) { _height = p_height; _request_update(); }
void HyperCapsuleMesh4D::set_radial_segments(int p_segments) { _radial_segments = p_segments; _request_update(); }
void HyperCapsuleMesh4D::set_rings(int p_rings) { _rings = p_rings; _request_update(); }

void HyperCapsuleMesh4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_radius"), &HyperCapsuleMesh4D::get_radius);
	ClassDB::bind_method(D_METHOD("set_radius", "radius"), &HyperCapsuleMesh4D::set_radius);
	ClassDB::bind_method(D_METHOD("get_height"), &HyperCapsuleMesh4D::get_height);
	ClassDB::bind_method(D_METHOD("set_height", "height"), &HyperCapsuleMesh4D::set_height);
	ClassDB::bind_method(D_METHOD("get_radial_segments"), &HyperCapsuleMesh4D::get_radial_segments);
	ClassDB::bind_method(D_METHOD("set_radial_segments", "segments"), &HyperCapsuleMesh4D::set_radial_segments);
	ClassDB::bind_method(D_METHOD("get_rings"), &HyperCapsuleMesh4D::get_rings);
	ClassDB::bind_method(D_METHOD("set_rings", "rings"), &HyperCapsuleMesh4D::set_rings);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "height"), "set_height", "get_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "radial_segments"), "set_radial_segments", "get_radial_segments");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rings"), "set_rings", "get_rings");
}

// Capsule = cylinder body + hemispherical caps
Array HyperCapsuleMesh4D::_create_mesh_array() const {
	// Reuse cylinder mesh but add hemispherical caps
	// For simplicity, use the cylinder mesh (caps can be added later)
	PackedFloat32Array vertices, normals;
	PackedInt32Array indices;

	float half_h = _height * 0.5f;
	int segs = _radial_segments;
	int rings = _rings;

	// Cylinder body
	for (int w = 0; w <= rings; w++) {
		float wv = -half_h + (float)w / rings * _height;
		for (int i = 0; i <= segs; i++) {
			float phi = (float)i / segs * Math_PI;
			for (int j = 0; j <= segs; j++) {
				float theta = (float)j / segs * Math_TAU;
				float x = sinf(phi) * cosf(theta);
				float y = sinf(phi) * sinf(theta);
				float z = cosf(phi);
				vertices.push_back(x * _radius);
				vertices.push_back(y * _radius);
				vertices.push_back(z * _radius);
				vertices.push_back(wv);
				normals.push_back(x); normals.push_back(y); normals.push_back(z); normals.push_back(0.0f);
			}
		}
	}

	static const int perms3[6][3] = {
		{0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
	};

	int row_size = (segs + 1) * (segs + 1);
	for (int w = 0; w < rings; w++) {
		for (int i = 0; i < segs; i++) {
			for (int j = 0; j < segs; j++) {
				int c[2][2][2];
				for (int dw = 0; dw < 2; dw++)
					for (int di = 0; di < 2; di++)
						for (int dj = 0; dj < 2; dj++)
							c[dw][di][dj] = (w + dw) * row_size + (i + di) * (segs + 1) + (j + dj);

				for (int t = 0; t < 6; t++) {
					int p[3] = {0, 0, 0};
					indices.push_back(c[0][0][0]);
					for (int s = 0; s < 3; s++) {
						p[perms3[t][s]] = 1;
						indices.push_back(c[p[0]][p[1]][p[2]]);
					}
				}
			}
		}
	}

	Array arrays;
	arrays.resize(Mesh4D::ARRAY_MAX);
	arrays[Mesh4D::ARRAY_VERTEX] = vertices;
	arrays[Mesh4D::ARRAY_NORMAL] = normals;
	arrays[Mesh4D::ARRAY_INDEX] = indices;
	return arrays;
}
