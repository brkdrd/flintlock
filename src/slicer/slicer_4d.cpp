#include "slicer_4d.h"
#include "../nodes/visual/visual_instance_4d.h"
#include "../resources/mesh_4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>

Slicer4D *Slicer4D::singleton = nullptr;

void Slicer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_instance", "instance"), &Slicer4D::register_instance);
	ClassDB::bind_method(D_METHOD("unregister_instance", "instance"), &Slicer4D::unregister_instance);
	ClassDB::bind_method(D_METHOD("get_instance_count"), &Slicer4D::get_instance_count);
}

Slicer4D::Slicer4D() {
	singleton = this;
}

Slicer4D::~Slicer4D() {
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		if (_material_rid.is_valid()) rs->free_rid(_material_rid);
		if (_shader_rid.is_valid()) rs->free_rid(_shader_rid);
	}
	singleton = nullptr;
}

Slicer4D *Slicer4D::get_singleton() {
	return singleton;
}

void Slicer4D::register_instance(VisualInstance4D *p_instance) {
	if (!_instances.has(p_instance)) {
		_instances.push_back(p_instance);
	}
}

void Slicer4D::unregister_instance(VisualInstance4D *p_instance) {
	_instances.erase(p_instance);
}

// ============================================================
// LUT Generation
//
// 8 columns x 16 rows, RGBA8.
// Row = sign_bits of 4 tet vertices (bit0=A, bit1=B, bit2=C, bit3=D)
// Column = gpu_vertex_id (0..5, columns 6-7 unused)
//
// Each tetrahedron produces 6 GPU vertices (2 triangles).
// For triangle intersections (3 crossing edges): verts 0-2 used, 3-5 degenerate.
// For quad intersections (4 crossing edges): all 6 used (2 tris from quad).
//
// R = endpoint_a index (0-3), G = endpoint_b index (0-3).
// Value 255 = degenerate (no intersection for this vertex).
// ============================================================
void Slicer4D::_generate_lut() {
	// Edges of a tetrahedron: (0,1),(0,2),(0,3),(1,2),(1,3),(2,3)
	static const int edges[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};

	// 8 wide x 16 tall
	PackedByteArray data;
	data.resize(8 * 16 * 4); // RGBA8
	for (int i = 0; i < data.size(); i++) data[i] = 255;

	for (int sign_bits = 0; sign_bits < 16; sign_bits++) {
		if (sign_bits == 0 || sign_bits == 15) continue; // all same side

		// Find crossing edges
		int crossing[6][2];
		int cross_count = 0;
		for (int e = 0; e < 6; e++) {
			int a = edges[e][0], b = edges[e][1];
			int sa = (sign_bits >> a) & 1;
			int sb = (sign_bits >> b) & 1;
			if (sa != sb) {
				crossing[cross_count][0] = a;
				crossing[cross_count][1] = b;
				cross_count++;
			}
		}

		if (cross_count == 3) {
			// Triangle: vertices 0,1,2 get the 3 edges; 3,4,5 degenerate
			for (int v = 0; v < 3; v++) {
				int px = v;
				int py = sign_bits;
				int off = (py * 8 + px) * 4;
				data[off + 0] = crossing[v][0];
				data[off + 1] = crossing[v][1];
				data[off + 2] = 0;
				data[off + 3] = 255;
			}
		} else if (cross_count == 4) {
			// Quad: need to order the 4 crossing edges into a proper polygon.
			// The edge enumeration order {(0,1),(0,2),(0,3),(1,2),(1,3),(2,3)}
			// produces crossings where indices 2 and 3 are swapped relative to
			// correct cyclic order. Swap them.
			int tmp0 = crossing[2][0], tmp1 = crossing[2][1];
			crossing[2][0] = crossing[3][0]; crossing[2][1] = crossing[3][1];
			crossing[3][0] = tmp0; crossing[3][1] = tmp1;

			// Two triangles: (0,1,2) and (0,2,3)
			int tri_verts[6][2] = {
				{crossing[0][0], crossing[0][1]},
				{crossing[1][0], crossing[1][1]},
				{crossing[2][0], crossing[2][1]},
				{crossing[0][0], crossing[0][1]},
				{crossing[2][0], crossing[2][1]},
				{crossing[3][0], crossing[3][1]},
			};
			for (int v = 0; v < 6; v++) {
				int px = v;
				int py = sign_bits;
				int off = (py * 8 + px) * 4;
				data[off + 0] = tri_verts[v][0];
				data[off + 1] = tri_verts[v][1];
				data[off + 2] = 0;
				data[off + 3] = 255;
			}
		}
	}

	Ref<Image> img = Image::create_from_data(8, 16, false, Image::FORMAT_RGBA8, data);
	_lut_texture = ImageTexture::create_from_image(img);
}

// ============================================================
// Shader Code
// ============================================================
String Slicer4D::_get_shader_code() const {
	return R"(
shader_type spatial;
render_mode cull_disabled, skip_vertex_transform;

// Global uniforms (set by Slicer4D per frame)
uniform mat4 camera_basis_4d;
uniform vec4 camera_origin_4d;
uniform vec4 slice_normal;
uniform float slice_d;
uniform sampler2D lut_texture : filter_nearest, repeat_disable;

// Per-instance uniforms (mat4 not supported as instance uniform, split into columns)
instance uniform vec4 model_4d_col0 = vec4(1.0, 0.0, 0.0, 0.0);
instance uniform vec4 model_4d_col1 = vec4(0.0, 1.0, 0.0, 0.0);
instance uniform vec4 model_4d_col2 = vec4(0.0, 0.0, 1.0, 0.0);
instance uniform vec4 model_4d_col3 = vec4(0.0, 0.0, 0.0, 1.0);
instance uniform vec4 model_4d_origin = vec4(0.0);
instance uniform vec4 albedo_color : source_color = vec4(1.0);
instance uniform float roughness_value = 1.0;
instance uniform float metallic_value = 0.0;

varying vec3 world_pos;

void vertex() {
	// Reconstruct 4 tetrahedron vertices from packed attributes
	vec4 va = vec4(VERTEX.x, VERTEX.y, VERTEX.z, CUSTOM0.x);
	vec4 vb = vec4(CUSTOM0.y, CUSTOM0.z, CUSTOM0.w, CUSTOM1.x);
	vec4 vc = vec4(CUSTOM1.y, CUSTOM1.z, CUSTOM1.w, CUSTOM2.x);
	vec4 vd = vec4(CUSTOM2.y, CUSTOM2.z, CUSTOM2.w, CUSTOM3.x);
	int vertex_id = int(CUSTOM3.y + 0.5);

	// Reconstruct 4x4 model basis from columns
	mat4 model_4d_basis = mat4(model_4d_col0, model_4d_col1, model_4d_col2, model_4d_col3);

	// Apply 4D model transform
	vec4 wa = model_4d_basis * va + model_4d_origin;
	vec4 wb = model_4d_basis * vb + model_4d_origin;
	vec4 wc = model_4d_basis * vc + model_4d_origin;
	vec4 wd = model_4d_basis * vd + model_4d_origin;

	// Signed distances to hyperplane
	float da = dot(slice_normal, wa) - slice_d;
	float db = dot(slice_normal, wb) - slice_d;
	float dc = dot(slice_normal, wc) - slice_d;
	float dd = dot(slice_normal, wd) - slice_d;

	// Build sign bits: bit0=A, bit1=B, bit2=C, bit3=D
	int sign_bits = (da > 0.0 ? 1 : 0) | (db > 0.0 ? 2 : 0) | (dc > 0.0 ? 4 : 0) | (dd > 0.0 ? 8 : 0);

	// LUT lookup
	vec4 lut_val = texture(lut_texture, vec2((float(vertex_id) + 0.5) / 8.0, (float(sign_bits) + 0.5) / 16.0));
	int ep_a = int(lut_val.r * 255.0 + 0.5);
	int ep_b = int(lut_val.g * 255.0 + 0.5);

	// Degenerate check — collapse to produce zero-area triangle
	if (ep_a > 3 || ep_b > 3) {
		VERTEX = vec3(0.0, 0.0, 0.0);
		POSITION = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	// Select the two 4D endpoints and their distances
	vec4 endpoints[4] = vec4[4](wa, wb, wc, wd);
	float dists[4] = float[4](da, db, dc, dd);

	vec4 p0 = endpoints[ep_a];
	vec4 p1 = endpoints[ep_b];
	float d0 = dists[ep_a];
	float d1 = dists[ep_b];

	// Interpolate to find hyperplane crossing (where distance = 0)
	float t = d0 / (d0 - d1);
	vec4 intersection_4d = mix(p0, p1, t);

	// Project 4D intersection to 3D using camera basis columns 0,1,2
	vec3 pos_3d;
	pos_3d.x = dot(camera_basis_4d[0], intersection_4d);
	pos_3d.y = dot(camera_basis_4d[1], intersection_4d);
	pos_3d.z = dot(camera_basis_4d[2], intersection_4d);

	world_pos = pos_3d;
	VERTEX = pos_3d;

	// Transform through Godot's view/projection pipeline
	POSITION = PROJECTION_MATRIX * VIEW_MATRIX * vec4(pos_3d, 1.0);

	// Placeholder normal (computed in fragment via derivatives)
	NORMAL = vec3(0.0, 1.0, 0.0);
}

void fragment() {
	// Compute flat normal from screen-space derivatives of world position
	vec3 dx = dFdx(world_pos);
	vec3 dy = dFdy(world_pos);
	vec3 n = normalize(cross(dx, dy));

	// Ensure normal faces the camera
	if (dot(n, VIEW_MATRIX[2].xyz) > 0.0) {
		n = -n;
	}

	NORMAL = (VIEW_MATRIX * vec4(n, 0.0)).xyz;
	ALBEDO = albedo_color.rgb;
	ROUGHNESS = roughness_value;
	METALLIC = metallic_value;
}
)";
}

// ============================================================
// Initialization
// ============================================================
void Slicer4D::_initialize() {
	if (_initialized) return;

	_generate_lut();

	RenderingServer *rs = RenderingServer::get_singleton();
	_shader_rid = rs->shader_create();
	rs->shader_set_code(_shader_rid, _get_shader_code());

	_material_rid = rs->material_create();
	rs->material_set_shader(_material_rid, _shader_rid);

	// Set LUT texture on the shared material (pass RID, not Object)
	rs->material_set_param(_material_rid, "lut_texture", _lut_texture->get_rid());

	// Set defaults for global uniforms
	Projection identity;
	rs->material_set_param(_material_rid, "camera_basis_4d", identity);
	rs->material_set_param(_material_rid, "camera_origin_4d", Vector4(0, 0, 0, 0));
	rs->material_set_param(_material_rid, "slice_normal", Vector4(0, 0, 0, 1));
	rs->material_set_param(_material_rid, "slice_d", 0.0f);

	_initialized = true;
}

void Slicer4D::ensure_initialized() {
	_initialize();
}

RID Slicer4D::get_material_rid() {
	ensure_initialized();
	return _material_rid;
}

// ============================================================
// Per-frame update — sets global shader uniforms only
// ============================================================
void Slicer4D::update_frame(const Vector4 &p_plane_normal, float p_plane_d,
	const PackedFloat32Array &p_basis_cols,
	const Vector4 &p_camera_origin) {

	if (!_initialized) _initialize();

	RenderingServer *rs = RenderingServer::get_singleton();

	// Update global uniforms on the shared material
	rs->material_set_param(_material_rid, "slice_normal", p_plane_normal);
	rs->material_set_param(_material_rid, "slice_d", p_plane_d);
	rs->material_set_param(_material_rid, "camera_origin_4d", p_camera_origin);

	// Pack camera basis as a Projection (mat4): columns 0-3 of the 4D camera basis
	// p_basis_cols has 12 floats (3 columns x 4 components).
	// Column 3 = slice_normal (already set separately).
	Projection cam_basis;
	// Column 0
	cam_basis.columns[0] = Vector4(p_basis_cols[0], p_basis_cols[1], p_basis_cols[2], p_basis_cols[3]);
	// Column 1
	cam_basis.columns[1] = Vector4(p_basis_cols[4], p_basis_cols[5], p_basis_cols[6], p_basis_cols[7]);
	// Column 2
	cam_basis.columns[2] = Vector4(p_basis_cols[8], p_basis_cols[9], p_basis_cols[10], p_basis_cols[11]);
	// Column 3 = slice normal
	cam_basis.columns[3] = p_plane_normal;

	rs->material_set_param(_material_rid, "camera_basis_4d", cam_basis);

	// Update per-instance 4D transforms
	for (VisualInstance4D *inst : _instances) {
		inst->update_shader_transforms();
	}
}
