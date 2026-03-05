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

// Tesseract: 16 vertices at (+/-hx, +/-hy, +/-hz, +/-hw)
// Decomposed into 24 tetrahedra.
// Each of the 24 tetrahedra comes from one of the 8 cubic cells.
// Each cubic cell is divided into 6 tetrahedra.
// But a tesseract has 8 cells, 8*6 = 48 would be cubic division.
// Simpler: use the standard decomposition of a 4-cube into 24 simplices.
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

	// Vertex index layout:
	// v(x,y,z,w) = w*8 + z*4 + y*2 + x (where x,y,z,w in {0,1})
	// Decompose into tetrahedra using Kuhn decomposition of the 4-cube.
	// The standard decomposition of [0,1]^4 into 24 simplices uses
	// the ordering-based method: for each permutation of {x,y,z,w},
	// a simplex is defined by consecutive increments.
	// We'll use a simpler approach: decompose each of the 8 cubic cells.

	// Helper to get vertex index
	auto V = [](int x, int y, int z, int w) -> int {
		return w * 8 + z * 4 + y * 2 + x;
	};

	// Normals: 4D outward-pointing normals for each face
	PackedFloat32Array normals;
	normals.resize(16 * 4);
	// Initialize all normals to 0; we'll compute them per-tetrahedron later
	for (int i = 0; i < 16 * 4; i++) normals[i] = 0.0f;

	// Standard decomposition of unit 4-cube into 24 tetrahedra
	// Each tetrahedron is specified by 4 vertex indices
	// Using the Kuhn decomposition: for each permutation sigma of {0,1,2,3},
	// define vertices v_0=origin, v_k = v_{k-1} + e_{sigma(k)}.
	// There are 24 such permutations.
	static const int perms[24][4] = {
		{0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
		{1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
		{2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
		{3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}
	};

	PackedInt32Array indices;
	indices.resize(24 * 4);

	for (int t = 0; t < 24; t++) {
		// Build tetrahedron: start at (0,0,0,0), add unit vectors in permutation order
		int coords[4][4] = {{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};
		for (int step = 0; step < 4; step++) {
			// Copy previous
			for (int k = 0; k < 4; k++) coords[step+1 <= 3 ? step+1 : 3][k] = coords[step > 0 ? step : 0][k];
			if (step < 3) {
				for (int k = 0; k < 4; k++) coords[step+1][k] = coords[step][k];
				coords[step+1][perms[t][step]] += 1;
			}
		}
		// Actually build simplex as cumulative sums
		int xs[5] = {0,0,0,0,0}, ys[5] = {0,0,0,0,0}, zs[5] = {0,0,0,0,0}, ws[5] = {0,0,0,0,0};
		for (int step = 1; step <= 4; step++) {
			xs[step] = xs[step-1]; ys[step] = ys[step-1];
			zs[step] = zs[step-1]; ws[step] = ws[step-1];
			int axis = perms[t][step-1];
			if (axis == 0) xs[step]++;
			else if (axis == 1) ys[step]++;
			else if (axis == 2) zs[step]++;
			else ws[step]++;
		}
		for (int v = 0; v < 4; v++) {
			indices[t * 4 + v] = V(xs[v], ys[v], zs[v], ws[v]);
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

	// Generate indices (tetrahedra)
	int segs1 = segments + 1;
	int segs2 = rings2 + 1;
	for (int i = 0; i < rings1; i++) {
		for (int j = 0; j < rings2; j++) {
			for (int k = 0; k < segments; k++) {
				int v0 = i * segs2 * segs1 + j * segs1 + k;
				int v1 = v0 + 1;
				int v2 = v0 + segs1;
				int v3 = v2 + 1;
				int v4 = v0 + segs2 * segs1;
				// Two tetrahedra per quad in parameter space (simplified)
				indices.push_back(v0); indices.push_back(v1); indices.push_back(v2); indices.push_back(v4);
				indices.push_back(v1); indices.push_back(v2); indices.push_back(v3); indices.push_back(v4);
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

	int row_size = (segs_phi + 1) * (segs_theta + 1);
	for (int w = 0; w < rings; w++) {
		for (int i = 0; i < segs_phi; i++) {
			for (int j = 0; j < segs_theta; j++) {
				int v0 = w * row_size + i * (segs_theta + 1) + j;
				int v1 = v0 + 1;
				int v2 = v0 + (segs_theta + 1);
				int v3 = v2 + 1;
				int v4 = v0 + row_size;
				indices.push_back(v0); indices.push_back(v1); indices.push_back(v2); indices.push_back(v4);
				indices.push_back(v1); indices.push_back(v2); indices.push_back(v3); indices.push_back(v4);
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

	int row_size = (segs + 1) * (segs + 1);
	for (int w = 0; w < rings; w++) {
		for (int i = 0; i < segs; i++) {
			for (int j = 0; j < segs; j++) {
				int v0 = w * row_size + i * (segs + 1) + j;
				int v1 = v0 + 1;
				int v2 = v0 + (segs + 1);
				int v3 = v2 + 1;
				int v4 = v0 + row_size;
				indices.push_back(v0); indices.push_back(v1); indices.push_back(v2); indices.push_back(v4);
				indices.push_back(v1); indices.push_back(v2); indices.push_back(v3); indices.push_back(v4);
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
