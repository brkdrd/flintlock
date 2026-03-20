#include "visual_server_4d.h"
#include "../../nodes/visual/visual_instance_4d.h"
#include "../../resources/mesh_4d.h"
#include "../../math/transform4d.h"
#include "../../math/basis4d.h"
#include "../../math/vector4d.h"
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/classes/camera_attributes.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/node.hpp>
#include <cstring>

using namespace godot;

VisualServer4D *VisualServer4D::singleton = nullptr;

// ============================================================
// Pack a 4D normal into uint32, bitcast to float
// ============================================================
static float pack_normal_4d_as_float(float nx, float ny, float nz, float nw) {
	auto to_byte = [](float v) -> uint32_t {
		int b = (int)(v * 127.0f + 128.0f);
		return (uint32_t)(b < 1 ? 1 : (b > 255 ? 255 : b));
	};
	uint32_t packed = to_byte(nx) | (to_byte(ny) << 8) | (to_byte(nz) << 16) | (to_byte(nw) << 24);
	float result;
	memcpy(&result, &packed, sizeof(float));
	return result;
}

// ============================================================
// Binding
// ============================================================
void VisualServer4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instance_create"), &VisualServer4D::instance_create);
	ClassDB::bind_method(D_METHOD("instance_set_base", "instance", "base"), &VisualServer4D::instance_set_base);
	ClassDB::bind_method(D_METHOD("instance_set_scenario", "instance", "scenario"), &VisualServer4D::instance_set_scenario);
	ClassDB::bind_method(D_METHOD("instance_set_visible", "instance", "visible"), &VisualServer4D::instance_set_visible);
	ClassDB::bind_method(D_METHOD("instance_set_layer_mask", "instance", "mask"), &VisualServer4D::instance_set_layer_mask);
	ClassDB::bind_method(D_METHOD("mesh_create"), &VisualServer4D::mesh_create);
	ClassDB::bind_method(D_METHOD("light_create", "type"), &VisualServer4D::light_create);
	ClassDB::bind_method(D_METHOD("light_set_color", "light", "color"), &VisualServer4D::light_set_color);
	ClassDB::bind_method(D_METHOD("light_set_shadow", "light", "enabled"), &VisualServer4D::light_set_shadow);
	ClassDB::bind_method(D_METHOD("free_rid", "rid"), &VisualServer4D::free_rid);
	ClassDB::bind_method(D_METHOD("get_instance_count"), &VisualServer4D::get_instance_count);
	ClassDB::bind_method(D_METHOD("get_light_count"), &VisualServer4D::get_light_count);
}

VisualServer4D::VisualServer4D() {
	singleton = this;
}

VisualServer4D::~VisualServer4D() {
	// Free all RS resources
	RenderingServer *rs = RenderingServer::get_singleton();
	if (rs) {
		for (auto &pair : _instances) {
			if (pair.value.rs_instance.is_valid()) rs->free_rid(pair.value.rs_instance);
			if (pair.value.rs_mesh.is_valid()) rs->free_rid(pair.value.rs_mesh);
		}
		for (auto &pair : _lights) {
			if (pair.value.rs_instance.is_valid()) rs->free_rid(pair.value.rs_instance);
			if (pair.value.rs_light.is_valid()) rs->free_rid(pair.value.rs_light);
		}
		if (_material_rid.is_valid()) rs->free_rid(_material_rid);
		if (_shader_rid.is_valid()) rs->free_rid(_shader_rid);
	}

	// Camera is a child node — it will be freed by the scene tree
	_internal_camera = nullptr;
	singleton = nullptr;
}

VisualServer4D *VisualServer4D::get_singleton() {
	return singleton;
}

// ============================================================
// LUT Generation (from Slicer4D)
// ============================================================
void VisualServer4D::_generate_lut() {
	static const int edges[6][2] = {{0,1},{0,2},{0,3},{1,2},{1,3},{2,3}};

	PackedByteArray data;
	data.resize(8 * 16 * 4);
	for (int i = 0; i < data.size(); i++) data[i] = 255;

	for (int sign_bits = 0; sign_bits < 16; sign_bits++) {
		if (sign_bits == 0 || sign_bits == 15) continue;

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
			for (int v = 0; v < 3; v++) {
				int off = (sign_bits * 8 + v) * 4;
				data[off + 0] = crossing[v][0];
				data[off + 1] = crossing[v][1];
				data[off + 2] = 0;
				data[off + 3] = 255;
			}
		} else if (cross_count == 4) {
			int tmp0 = crossing[2][0], tmp1 = crossing[2][1];
			crossing[2][0] = crossing[3][0]; crossing[2][1] = crossing[3][1];
			crossing[3][0] = tmp0; crossing[3][1] = tmp1;

			int tri_verts[6][2] = {
				{crossing[0][0], crossing[0][1]},
				{crossing[1][0], crossing[1][1]},
				{crossing[2][0], crossing[2][1]},
				{crossing[0][0], crossing[0][1]},
				{crossing[2][0], crossing[2][1]},
				{crossing[3][0], crossing[3][1]},
			};
			for (int v = 0; v < 6; v++) {
				int off = (sign_bits * 8 + v) * 4;
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
// Shader Code (from Slicer4D)
// ============================================================
String VisualServer4D::_get_shader_code() const {
	return R"(
shader_type spatial;
render_mode cull_disabled;

// Global uniforms (set by VisualServer4D per frame)
uniform mat4 camera_basis_4d;
uniform vec4 camera_origin_4d;
uniform vec4 slice_normal;
uniform float slice_d;
uniform sampler2D lut_texture : filter_nearest, repeat_disable;

// Per-instance uniforms
instance uniform vec4 model_4d_col0 = vec4(1.0, 0.0, 0.0, 0.0);
instance uniform vec4 model_4d_col1 = vec4(0.0, 1.0, 0.0, 0.0);
instance uniform vec4 model_4d_col2 = vec4(0.0, 0.0, 1.0, 0.0);
instance uniform vec4 model_4d_col3 = vec4(0.0, 0.0, 0.0, 1.0);
instance uniform vec4 model_4d_origin = vec4(0.0);
instance uniform vec4 albedo_color : source_color = vec4(1.0);
instance uniform float roughness_value = 1.0;
instance uniform float metallic_value = 0.0;
instance uniform float instance_valid = 0.0;

varying float v_degenerate;
varying float v_has_normals;

void vertex() {
	vec4 va = vec4(VERTEX.x, VERTEX.y, VERTEX.z, CUSTOM0.x);
	vec4 vb = vec4(CUSTOM0.y, CUSTOM0.z, CUSTOM0.w, CUSTOM1.x);
	vec4 vc = vec4(CUSTOM1.y, CUSTOM1.z, CUSTOM1.w, CUSTOM2.x);
	vec4 vd = vec4(CUSTOM2.y, CUSTOM2.z, CUSTOM2.w, CUSTOM3.x);
	int vertex_id = int(CUSTOM3.y + 0.5);

	uint pna = floatBitsToUint(CUSTOM3.z);
	uint pnb = floatBitsToUint(CUSTOM3.w);
	uint pnc = floatBitsToUint(UV.x);
	uint pnd = floatBitsToUint(UV.y);

	mat4 model_4d_basis = mat4(model_4d_col0, model_4d_col1, model_4d_col2, model_4d_col3);

	vec4 wa = model_4d_basis * va + model_4d_origin;
	vec4 wb = model_4d_basis * vb + model_4d_origin;
	vec4 wc = model_4d_basis * vc + model_4d_origin;
	vec4 wd = model_4d_basis * vd + model_4d_origin;

	float da = dot(slice_normal, wa) - slice_d;
	float db = dot(slice_normal, wb) - slice_d;
	float dc = dot(slice_normal, wc) - slice_d;
	float dd = dot(slice_normal, wd) - slice_d;

	int sign_bits = (da > 0.0 ? 1 : 0) | (db > 0.0 ? 2 : 0) | (dc > 0.0 ? 4 : 0) | (dd > 0.0 ? 8 : 0);

	vec4 lut_val = texture(lut_texture, vec2((float(vertex_id) + 0.5) / 8.0, (float(sign_bits) + 0.5) / 16.0));
	int ep_a = int(lut_val.r * 255.0 + 0.5);
	int ep_b = int(lut_val.g * 255.0 + 0.5);

	bool degenerate = (ep_a > 3 || ep_b > 3) || (instance_valid < 0.5);
	v_degenerate = degenerate ? 1.0 : 0.0;

	int safe_a = clamp(ep_a, 0, 3);
	int safe_b = clamp(ep_b, 0, 3);
	int ia = degenerate ? 0 : safe_a;
	int ib = degenerate ? 0 : safe_b;

	float sa0 = float(ia == 0), sa1 = float(ia == 1), sa2 = float(ia == 2), sa3 = float(ia == 3);
	float sb0 = float(ib == 0), sb1 = float(ib == 1), sb2 = float(ib == 2), sb3 = float(ib == 3);

	vec4 p0 = wa * sa0 + wb * sa1 + wc * sa2 + wd * sa3;
	vec4 p1 = wa * sb0 + wb * sb1 + wc * sb2 + wd * sb3;
	float d0 = da * sa0 + db * sa1 + dc * sa2 + dd * sa3;
	float d1 = da * sb0 + db * sb1 + dc * sb2 + dd * sb3;

	vec4 na4d = vec4(uvec4(pna, pna >> 8u, pna >> 16u, pna >> 24u) & uvec4(0xFFu)) - 128.0;
	vec4 nb4d = vec4(uvec4(pnb, pnb >> 8u, pnb >> 16u, pnb >> 24u) & uvec4(0xFFu)) - 128.0;
	vec4 nc4d = vec4(uvec4(pnc, pnc >> 8u, pnc >> 16u, pnc >> 24u) & uvec4(0xFFu)) - 128.0;
	vec4 nd4d = vec4(uvec4(pnd, pnd >> 8u, pnd >> 16u, pnd >> 24u) & uvec4(0xFFu)) - 128.0;

	vec4 n0 = model_4d_basis * (na4d * sa0 + nb4d * sa1 + nc4d * sa2 + nd4d * sa3);
	vec4 n1 = model_4d_basis * (na4d * sb0 + nb4d * sb1 + nc4d * sb2 + nd4d * sb3);

	float denom = d0 - d1;
	float t = (abs(denom) > 1e-10) ? d0 / denom : 0.0;
	vec4 intersection_4d = degenerate ? vec4(0.0) : mix(p0, p1, t);

	vec4 interp_normal_4d = mix(n0, n1, t);

	VERTEX.x = dot(camera_basis_4d[0], intersection_4d);
	VERTEX.y = dot(camera_basis_4d[1], intersection_4d);
	VERTEX.z = dot(camera_basis_4d[2], intersection_4d);

	vec3 n_world;
	n_world.x = dot(camera_basis_4d[0], interp_normal_4d);
	n_world.y = dot(camera_basis_4d[1], interp_normal_4d);
	n_world.z = dot(camera_basis_4d[2], interp_normal_4d);

	float nlen = degenerate ? 0.0 : length(n_world);
	v_has_normals = (nlen > 0.01) ? 1.0 : 0.0;
	NORMAL = (nlen > 0.01) ? n_world / nlen : vec3(0.0, 1.0, 0.0);
}

void fragment() {
	if (v_degenerate > 0.5) discard;

	vec3 n;
	if (v_has_normals > 0.5) {
		n = normalize(NORMAL);
	} else {
		n = normalize(cross(dFdx(VERTEX), dFdy(VERTEX)));
	}

	if (n.z < 0.0) {
		n = -n;
	}

	NORMAL = n;
	ALBEDO = albedo_color.rgb;
	ROUGHNESS = roughness_value;
	METALLIC = metallic_value;
}
)";
}

// ============================================================
// Initialization
// ============================================================
void VisualServer4D::_initialize() {
	if (_initialized) return;

	_generate_lut();

	RenderingServer *rs = RenderingServer::get_singleton();
	_shader_rid = rs->shader_create();
	rs->shader_set_code(_shader_rid, _get_shader_code());

	_material_rid = rs->material_create();
	rs->material_set_shader(_material_rid, _shader_rid);
	rs->material_set_param(_material_rid, "lut_texture", _lut_texture->get_rid());

	Projection identity;
	rs->material_set_param(_material_rid, "camera_basis_4d", identity);
	rs->material_set_param(_material_rid, "camera_origin_4d", Vector4(0, 0, 0, 0));
	rs->material_set_param(_material_rid, "slice_normal", Vector4(0, 0, 0, 1));
	rs->material_set_param(_material_rid, "slice_d", 0.0f);

	_initialized = true;
}

void VisualServer4D::ensure_initialized() {
	_initialize();
}

RID VisualServer4D::get_shader_rid() {
	ensure_initialized();
	return _shader_rid;
}

RID VisualServer4D::get_material_rid() {
	ensure_initialized();
	return _material_rid;
}

// ============================================================
// Instance API
// ============================================================
RID VisualServer4D::instance_create() {
	ensure_initialized();

	RID rid = _make_rid();
	Instance4D inst;

	RenderingServer *rs = RenderingServer::get_singleton();
	inst.rs_mesh = rs->mesh_create();
	inst.rs_instance = rs->instance_create();
	rs->instance_set_base(inst.rs_instance, inst.rs_mesh);

	_instances[rid] = inst;
	return rid;
}

void VisualServer4D::instance_set_base(const RID &p_instance, const RID &p_base) {
	if (!_instances.has(p_instance)) return;
	Instance4D &inst = _instances[p_instance];
	inst.base_rid = p_base;

	if (_meshes.has(p_base)) {
		inst.base_type = BASE_MESH;
		// Upload mesh data to the instance's RS mesh
		_upload_instance_mesh(inst, _meshes[p_base].mesh);
	} else {
		inst.base_type = BASE_NONE;
	}
}

void VisualServer4D::instance_set_scenario(const RID &p_instance, const RID &p_scenario) {
	if (!_instances.has(p_instance)) return;
	Instance4D &inst = _instances[p_instance];
	inst.scenario = p_scenario;
	if (inst.rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(inst.rs_instance, p_scenario);
	}
}

void VisualServer4D::instance_set_source_node(const RID &p_instance, VisualInstance4D *p_node) {
	if (!_instances.has(p_instance)) return;
	_instances[p_instance].source_node = p_node;
}

void VisualServer4D::instance_set_transform_4d(const RID &p_instance,
	const PackedFloat32Array &p_basis_cols, const Vector4 &p_origin) {
	// Kept for API completeness / GDScript usage, but nodes with
	// source_node set will read transforms live during process_frame().
	(void)p_instance;
	(void)p_basis_cols;
	(void)p_origin;
}

void VisualServer4D::instance_set_visible(const RID &p_instance, bool p_visible) {
	if (!_instances.has(p_instance)) return;
	Instance4D &inst = _instances[p_instance];
	inst.visible = p_visible;
	if (inst.rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_visible(inst.rs_instance, p_visible);
	}
}

void VisualServer4D::instance_set_layer_mask(const RID &p_instance, uint32_t p_mask) {
	if (!_instances.has(p_instance)) return;
	Instance4D &inst = _instances[p_instance];
	inst.layers = p_mask;
	if (inst.rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_layer_mask(inst.rs_instance, p_mask);
	}
}

void VisualServer4D::instance_set_material_params(const RID &p_instance,
	const Color &p_albedo, float p_roughness, float p_metallic) {
	if (!_instances.has(p_instance)) return;
	Instance4D &inst = _instances[p_instance];
	inst.albedo = p_albedo;
	inst.roughness = p_roughness;
	inst.metallic = p_metallic;
	_update_instance_material_params(inst);
}

// ============================================================
// Mesh API
// ============================================================
RID VisualServer4D::mesh_create() {
	RID rid = _make_rid();
	_meshes[rid] = Mesh4DData();
	return rid;
}

void VisualServer4D::mesh_set_data(const RID &p_mesh, const Ref<Mesh4D> &p_mesh_data) {
	if (!_meshes.has(p_mesh)) return;
	_meshes[p_mesh].mesh = p_mesh_data;

	// Update all instances that reference this mesh
	for (auto &pair : _instances) {
		if (pair.value.base_type == BASE_MESH && pair.value.base_rid == p_mesh) {
			_upload_instance_mesh(pair.value, p_mesh_data);
		}
	}
}

void VisualServer4D::mesh_clear(const RID &p_mesh) {
	if (!_meshes.has(p_mesh)) return;
	_meshes[p_mesh].mesh = Ref<Mesh4D>();

	// Clear all instances referencing this mesh
	for (auto &pair : _instances) {
		if (pair.value.base_type == BASE_MESH && pair.value.base_rid == p_mesh) {
			if (pair.value.rs_mesh.is_valid()) {
				RenderingServer::get_singleton()->mesh_clear(pair.value.rs_mesh);
			}
			pair.value.gpu_mesh_uploaded = false;
		}
	}
}

// ============================================================
// Light API
// ============================================================
RID VisualServer4D::light_create(int p_type) {
	RID rid = _make_rid();
	Light4DData light;

	RenderingServer *rs = RenderingServer::get_singleton();

	switch (p_type) {
		case 0: // directional
			light.type = LIGHT_DIRECTIONAL;
			light.rs_light = rs->directional_light_create();
			break;
		case 1: // omni
			light.type = LIGHT_OMNI;
			light.rs_light = rs->omni_light_create();
			break;
		case 2: // spot
			light.type = LIGHT_SPOT;
			light.rs_light = rs->spot_light_create();
			break;
		default:
			light.type = LIGHT_OMNI;
			light.rs_light = rs->omni_light_create();
			break;
	}

	light.rs_instance = rs->instance_create();
	rs->instance_set_base(light.rs_instance, light.rs_light);

	_lights[rid] = light;
	return rid;
}

void VisualServer4D::light_set_scenario(const RID &p_light, const RID &p_scenario) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.scenario = p_scenario;
	if (light.rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(light.rs_instance, p_scenario);
	}
}

void VisualServer4D::light_set_color(const RID &p_light, const Color &p_color) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.color = p_color;
	RenderingServer::get_singleton()->light_set_color(light.rs_light, p_color);
}

void VisualServer4D::light_set_param(const RID &p_light, int p_param, float p_value) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	RenderingServer *rs = RenderingServer::get_singleton();

	// Map common params and store locally
	switch ((RenderingServer::LightParam)p_param) {
		case RenderingServer::LIGHT_PARAM_ENERGY:
			light.energy = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_INDIRECT_ENERGY:
			light.indirect_energy = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SPECULAR:
			light.specular = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_RANGE:
			light.range = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_ATTENUATION:
			light.attenuation = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SPOT_ANGLE:
			light.spot_angle = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SPOT_ATTENUATION:
			light.spot_angle_attenuation = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_BIAS:
			light.shadow_bias = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_NORMAL_BIAS:
			light.shadow_normal_bias = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_OPACITY:
			light.shadow_opacity = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_BLUR:
			light.shadow_blur = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_MAX_DISTANCE:
			light.shadow_max_distance = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_FADE_START:
			light.shadow_fade_start = p_value;
			break;
		case RenderingServer::LIGHT_PARAM_SHADOW_PANCAKE_SIZE:
			light.shadow_pancake_size = p_value;
			break;
		default:
			break;
	}

	rs->light_set_param(light.rs_light, (RenderingServer::LightParam)p_param, p_value);
}

void VisualServer4D::light_set_shadow(const RID &p_light, bool p_enabled) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.shadow = p_enabled;
	RenderingServer::get_singleton()->light_set_shadow(light.rs_light, p_enabled);
}

void VisualServer4D::light_set_negative(const RID &p_light, bool p_negative) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.negative = p_negative;
	RenderingServer::get_singleton()->light_set_negative(light.rs_light, p_negative);
}

void VisualServer4D::light_set_cull_mask(const RID &p_light, uint32_t p_mask) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.cull_mask = p_mask;
	RenderingServer::get_singleton()->light_set_cull_mask(light.rs_light, p_mask);
}

void VisualServer4D::light_set_transform_4d(const RID &p_light,
	const PackedFloat32Array &p_basis_cols, const Vector4 &p_origin) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];

	if (p_basis_cols.size() >= 16) {
		memcpy(light.basis_col0, &p_basis_cols[0], 4 * sizeof(float));
		memcpy(light.basis_col1, &p_basis_cols[4], 4 * sizeof(float));
		memcpy(light.basis_col2, &p_basis_cols[8], 4 * sizeof(float));
		memcpy(light.basis_col3, &p_basis_cols[12], 4 * sizeof(float));
	}
	light.origin[0] = p_origin.x;
	light.origin[1] = p_origin.y;
	light.origin[2] = p_origin.z;
	light.origin[3] = p_origin.w;
}

void VisualServer4D::light_set_visible(const RID &p_light, bool p_visible) {
	if (!_lights.has(p_light)) return;
	Light4DData &light = _lights[p_light];
	light.visible = p_visible;
	if (light.rs_instance.is_valid()) {
		RenderingServer::get_singleton()->instance_set_visible(light.rs_instance, p_visible);
	}
}

// ============================================================
// Camera API
// ============================================================
void VisualServer4D::camera_attach_to_node(Node *p_node) {
	if (!p_node) return;
	if (_internal_camera && _camera_parent) {
		camera_detach();
	}
	_camera_parent = p_node;
	_internal_camera = memnew(Camera3D);
	p_node->add_child(_internal_camera);
	_internal_camera->set_name("__InternalCamera3D__");
}

void VisualServer4D::camera_detach() {
	if (_internal_camera) {
		_internal_camera->queue_free();
		_internal_camera = nullptr;
	}
	_camera_parent = nullptr;
}

void VisualServer4D::camera_set_perspective(float p_fov, float p_near, float p_far) {
	if (_internal_camera) {
		_internal_camera->set_perspective(p_fov, p_near, p_far);
	}
}

void VisualServer4D::camera_set_orthogonal(float p_size, float p_near, float p_far) {
	if (_internal_camera) {
		_internal_camera->set_orthogonal(p_size, p_near, p_far);
	}
}

void VisualServer4D::camera_set_cull_mask(uint32_t p_mask) {
	if (_internal_camera) _internal_camera->set_cull_mask(p_mask);
}

void VisualServer4D::camera_set_h_offset(float p_offset) {
	if (_internal_camera) _internal_camera->set_h_offset(p_offset);
}

void VisualServer4D::camera_set_v_offset(float p_offset) {
	if (_internal_camera) _internal_camera->set_v_offset(p_offset);
}

void VisualServer4D::camera_set_environment(const Ref<Environment> &p_env) {
	if (_internal_camera) _internal_camera->set_environment(p_env);
}

void VisualServer4D::camera_set_attributes(const Ref<CameraAttributes> &p_attrs) {
	if (_internal_camera) _internal_camera->set_attributes(p_attrs);
}

void VisualServer4D::camera_make_current() {
	if (_internal_camera) _internal_camera->make_current();
}

void VisualServer4D::camera_clear_current(bool p_enable_next) {
	if (_internal_camera) _internal_camera->clear_current(p_enable_next);
}

// ============================================================
// Frame Processing
// ============================================================
void VisualServer4D::process_frame(const Vector4 &p_plane_normal, float p_plane_d,
	const PackedFloat32Array &p_basis_cols, const Vector4 &p_camera_origin) {

	if (!_initialized) _initialize();

	_slice_normal = p_plane_normal;
	_slice_d = p_plane_d;
	_cam_origin = p_camera_origin;
	if (p_basis_cols.size() >= 12) {
		memcpy(_cam_basis, p_basis_cols.ptr(), 12 * sizeof(float));
	}

	RenderingServer *rs = RenderingServer::get_singleton();

	// Pack camera basis as Projection (mat4)
	Projection cam_basis;
	cam_basis.columns[0] = Vector4(_cam_basis[0], _cam_basis[1], _cam_basis[2], _cam_basis[3]);
	cam_basis.columns[1] = Vector4(_cam_basis[4], _cam_basis[5], _cam_basis[6], _cam_basis[7]);
	cam_basis.columns[2] = Vector4(_cam_basis[8], _cam_basis[9], _cam_basis[10], _cam_basis[11]);
	cam_basis.columns[3] = p_plane_normal;

	// Update global uniforms on the shared material
	rs->material_set_param(_material_rid, "slice_normal", p_plane_normal);
	rs->material_set_param(_material_rid, "slice_d", p_plane_d);
	rs->material_set_param(_material_rid, "camera_origin_4d", p_camera_origin);
	rs->material_set_param(_material_rid, "camera_basis_4d", cam_basis);

	// Update per-instance shader transforms
	for (auto &pair : _instances) {
		_update_instance_shader_transforms(pair.value);
	}

	// Project all lights from 4D to 3D
	for (auto &pair : _lights) {
		_project_light(pair.value);
	}

	// Update Camera3D position
	if (_internal_camera) {
		Vector4 col0(_cam_basis[0], _cam_basis[1], _cam_basis[2], _cam_basis[3]);
		Vector4 col1(_cam_basis[4], _cam_basis[5], _cam_basis[6], _cam_basis[7]);
		Vector4 col2(_cam_basis[8], _cam_basis[9], _cam_basis[10], _cam_basis[11]);
		Transform3D cam_xform;
		cam_xform.origin = Vector3(col0.dot(p_camera_origin), col1.dot(p_camera_origin), col2.dot(p_camera_origin));
		_internal_camera->set_global_transform(cam_xform);
	}
}

// ============================================================
// Cleanup
// ============================================================
void VisualServer4D::free_rid(const RID &p_rid) {
	RenderingServer *rs = RenderingServer::get_singleton();

	if (_instances.has(p_rid)) {
		Instance4D &inst = _instances[p_rid];
		if (inst.rs_instance.is_valid()) rs->free_rid(inst.rs_instance);
		if (inst.rs_mesh.is_valid()) rs->free_rid(inst.rs_mesh);
		_instances.erase(p_rid);
		return;
	}

	if (_meshes.has(p_rid)) {
		_meshes.erase(p_rid);
		return;
	}

	if (_lights.has(p_rid)) {
		Light4DData &light = _lights[p_rid];
		if (light.rs_instance.is_valid()) rs->free_rid(light.rs_instance);
		if (light.rs_light.is_valid()) rs->free_rid(light.rs_light);
		_lights.erase(p_rid);
		return;
	}
}

// ============================================================
// Internal: Upload mesh data to GPU
// ============================================================
void VisualServer4D::_upload_instance_mesh(Instance4D &inst, const Ref<Mesh4D> &mesh) {
	if (!inst.rs_mesh.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();
	rs->mesh_clear(inst.rs_mesh);

	if (mesh.is_null() || mesh->get_surface_count() == 0) {
		inst.gpu_mesh_uploaded = false;
		return;
	}

	PackedVector3Array gpu_verts;
	PackedVector2Array gpu_uvs;
	PackedFloat32Array gpu_custom0, gpu_custom1, gpu_custom2, gpu_custom3;
	PackedInt32Array gpu_indices;

	int gpu_vert_offset = 0;

	for (int surf = 0; surf < mesh->get_surface_count(); surf++) {
		Array arrays = mesh->get_surface_arrays(surf);
		if (arrays.size() < Mesh4D::ARRAY_MAX) continue;

		PackedFloat32Array verts4d = arrays[Mesh4D::ARRAY_VERTEX];
		PackedInt32Array indices4d = arrays[Mesh4D::ARRAY_INDEX];

		if (verts4d.is_empty() || indices4d.is_empty()) continue;

		PackedFloat32Array normals4d;
		if (arrays[Mesh4D::ARRAY_NORMAL].get_type() == Variant::PACKED_FLOAT32_ARRAY) {
			normals4d = arrays[Mesh4D::ARRAY_NORMAL];
		}
		bool has_normals = (normals4d.size() >= verts4d.size());

		int tet_count = indices4d.size() / 4;
		int new_verts = tet_count * 6;
		int base = gpu_verts.size();
		gpu_verts.resize(base + new_verts);
		gpu_uvs.resize(base + new_verts);
		gpu_custom0.resize((base + new_verts) * 4);
		gpu_custom1.resize((base + new_verts) * 4);
		gpu_custom2.resize((base + new_verts) * 4);
		gpu_custom3.resize((base + new_verts) * 4);

		for (int t = 0; t < tet_count; t++) {
			int i0 = indices4d[t * 4 + 0];
			int i1 = indices4d[t * 4 + 1];
			int i2 = indices4d[t * 4 + 2];
			int i3 = indices4d[t * 4 + 3];

			float va[4], vb[4], vc[4], vd[4];
			for (int k = 0; k < 4; k++) {
				va[k] = verts4d[i0 * 4 + k];
				vb[k] = verts4d[i1 * 4 + k];
				vc[k] = verts4d[i2 * 4 + k];
				vd[k] = verts4d[i3 * 4 + k];
			}

			float na[4] = {0,0,0,0}, nb[4] = {0,0,0,0};
			float nc[4] = {0,0,0,0}, nd[4] = {0,0,0,0};
			if (has_normals) {
				for (int k = 0; k < 4; k++) {
					na[k] = normals4d[i0 * 4 + k];
					nb[k] = normals4d[i1 * 4 + k];
					nc[k] = normals4d[i2 * 4 + k];
					nd[k] = normals4d[i3 * 4 + k];
				}
			}
			float packed_na = pack_normal_4d_as_float(na[0], na[1], na[2], na[3]);
			float packed_nb = pack_normal_4d_as_float(nb[0], nb[1], nb[2], nb[3]);
			float packed_nc = pack_normal_4d_as_float(nc[0], nc[1], nc[2], nc[3]);
			float packed_nd = pack_normal_4d_as_float(nd[0], nd[1], nd[2], nd[3]);

			for (int v = 0; v < 6; v++) {
				int gi = base + t * 6 + v;
				int c0 = gi * 4;

				gpu_verts[gi] = Vector3(va[0], va[1], va[2]);

				gpu_custom0[c0 + 0] = va[3];
				gpu_custom0[c0 + 1] = vb[0];
				gpu_custom0[c0 + 2] = vb[1];
				gpu_custom0[c0 + 3] = vb[2];

				gpu_custom1[c0 + 0] = vb[3];
				gpu_custom1[c0 + 1] = vc[0];
				gpu_custom1[c0 + 2] = vc[1];
				gpu_custom1[c0 + 3] = vc[2];

				gpu_custom2[c0 + 0] = vc[3];
				gpu_custom2[c0 + 1] = vd[0];
				gpu_custom2[c0 + 2] = vd[1];
				gpu_custom2[c0 + 3] = vd[2];

				gpu_custom3[c0 + 0] = vd[3];
				gpu_custom3[c0 + 1] = (float)v;
				gpu_custom3[c0 + 2] = packed_na;
				gpu_custom3[c0 + 3] = packed_nb;

				gpu_uvs[gi] = Vector2(packed_nc, packed_nd);
			}

			int vi_base = gpu_vert_offset + t * 6;
			gpu_indices.push_back(vi_base + 0);
			gpu_indices.push_back(vi_base + 1);
			gpu_indices.push_back(vi_base + 2);
			gpu_indices.push_back(vi_base + 3);
			gpu_indices.push_back(vi_base + 4);
			gpu_indices.push_back(vi_base + 5);
		}

		gpu_vert_offset += new_verts;
	}

	if (gpu_verts.is_empty()) {
		inst.gpu_mesh_uploaded = false;
		return;
	}

	Array arrays;
	arrays.resize(RenderingServer::ARRAY_MAX);
	arrays[RenderingServer::ARRAY_VERTEX] = gpu_verts;
	arrays[RenderingServer::ARRAY_TEX_UV] = gpu_uvs;
	arrays[RenderingServer::ARRAY_CUSTOM0] = gpu_custom0;
	arrays[RenderingServer::ARRAY_CUSTOM1] = gpu_custom1;
	arrays[RenderingServer::ARRAY_CUSTOM2] = gpu_custom2;
	arrays[RenderingServer::ARRAY_CUSTOM3] = gpu_custom3;
	arrays[RenderingServer::ARRAY_INDEX] = gpu_indices;

	uint64_t fmt = (uint64_t)RenderingServer::ARRAY_FORMAT_VERTEX
		| (uint64_t)RenderingServer::ARRAY_FORMAT_TEX_UV
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM0
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM1
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM2
		| (uint64_t)RenderingServer::ARRAY_FORMAT_CUSTOM3
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM0_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM1_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM2_SHIFT)
		| ((uint64_t)RenderingServer::ARRAY_CUSTOM_RGBA_FLOAT << RenderingServer::ARRAY_FORMAT_CUSTOM3_SHIFT);

	rs->mesh_add_surface_from_arrays(inst.rs_mesh, RenderingServer::PRIMITIVE_TRIANGLES, arrays,
		Array(), Dictionary(), (BitField<RenderingServer::ArrayFormat>)fmt);

	rs->mesh_surface_set_material(inst.rs_mesh, 0, _material_rid);
	rs->mesh_set_custom_aabb(inst.rs_mesh, AABB(Vector3(-250, -250, -250), Vector3(500, 500, 500)));
	rs->instance_set_transform(inst.rs_instance, Transform3D());

	// Do NOT set instance_valid here. The shader default (0.0) keeps vertices
	// degenerate until process_frame() sets correct camera globals and then
	// sets instance_valid=1.0 via _update_instance_shader_transforms().
	// Explicitly setting it here can trigger per-instance uniform buffer
	// creation before camera globals are ready, producing a phantom mesh.

	inst.gpu_mesh_uploaded = true;
}

// ============================================================
// Internal: Update per-instance shader transforms
// ============================================================
void VisualServer4D::_update_instance_shader_transforms(Instance4D &inst) {
	if (!inst.rs_instance.is_valid() || !inst.gpu_mesh_uploaded) return;
	if (!inst.source_node) return;

	// Read the LIVE transform from the node each frame,
	// matching the reimplementation branch pattern that avoids phantoms.
	Ref<Transform4D> gt = inst.source_node->get_global_transform_4d();
	if (gt.is_null()) return;
	Ref<Basis4D> basis = gt->get_basis();
	Ref<Vector4D> origin = gt->get_origin();
	if (basis.is_null() || origin.is_null()) return;

	Vector4 col0(basis->data[0][0], basis->data[0][1], basis->data[0][2], basis->data[0][3]);
	Vector4 col1(basis->data[1][0], basis->data[1][1], basis->data[1][2], basis->data[1][3]);
	Vector4 col2(basis->data[2][0], basis->data[2][1], basis->data[2][2], basis->data[2][3]);
	Vector4 col3(basis->data[3][0], basis->data[3][1], basis->data[3][2], basis->data[3][3]);
	Vector4 model_origin(origin->x, origin->y, origin->z, origin->w);

	RenderingServer *rs = RenderingServer::get_singleton();
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "model_4d_col0", col0);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "model_4d_col1", col1);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "model_4d_col2", col2);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "model_4d_col3", col3);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "model_4d_origin", model_origin);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "instance_valid", 1.0f);
}

// ============================================================
// Internal: Update per-instance material params
// ============================================================
void VisualServer4D::_update_instance_material_params(Instance4D &inst) {
	if (!inst.rs_instance.is_valid()) return;

	RenderingServer *rs = RenderingServer::get_singleton();
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "albedo_color", inst.albedo);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "roughness_value", inst.roughness);
	rs->instance_geometry_set_shader_parameter(inst.rs_instance, "metallic_value", inst.metallic);
}

// ============================================================
// Internal: Project a 4D light to 3D
// ============================================================
void VisualServer4D::_project_light(Light4DData &light) {
	if (!light.rs_instance.is_valid()) return;
	if (!light.visible) return;

	RenderingServer *rs = RenderingServer::get_singleton();

	// Camera basis columns for projection
	Vector4 cb0(_cam_basis[0], _cam_basis[1], _cam_basis[2], _cam_basis[3]);
	Vector4 cb1(_cam_basis[4], _cam_basis[5], _cam_basis[6], _cam_basis[7]);
	Vector4 cb2(_cam_basis[8], _cam_basis[9], _cam_basis[10], _cam_basis[11]);

	Vector4 pos4(light.origin[0], light.origin[1], light.origin[2], light.origin[3]);

	if (light.type == LIGHT_DIRECTIONAL) {
		// Project direction (column 2 of light basis, negated)
		float dx = -light.basis_col2[0];
		float dy = -light.basis_col2[1];
		float dz = -light.basis_col2[2];
		float dw = -light.basis_col2[3];

		Vector4 dir4(dx, dy, dz, dw);
		float dir_x = cb0.dot(dir4);
		float dir_y = cb1.dot(dir4);
		float dir_z = cb2.dot(dir4);

		Vector3 dir3(dir_x, dir_y, dir_z);
		float len = dir3.length();
		if (len < 1e-6f) return;
		dir3 /= len;

		Vector3 light_pos = -dir3 * 1000.0f;

		Transform3D xform;
		// Build rotation from direction
		Vector3 up(0, 1, 0);
		if (Math::abs(dir3.dot(up)) > 0.999f) {
			up = Vector3(1, 0, 0);
		}
		Vector3 right = up.cross(-dir3).normalized();
		up = (-dir3).cross(right).normalized();
		xform.basis.set_column(0, right);
		xform.basis.set_column(1, up);
		xform.basis.set_column(2, -dir3);
		xform.origin = light_pos;

		rs->instance_set_transform(light.rs_instance, xform);

	} else {
		// Omni or Spot: project 4D position to 3D
		float x3 = cb0.dot(pos4);
		float y3 = cb1.dot(pos4);
		float z3 = cb2.dot(pos4);

		// W-distance for range attenuation
		float w_dist = Math::abs(_slice_normal.dot(pos4) - _slice_d);
		float range_sq = light.range * light.range - w_dist * w_dist;

		if (range_sq < 0.0f) {
			rs->instance_set_visible(light.rs_instance, false);
			return;
		}

		rs->instance_set_visible(light.rs_instance, true);
		float effective_range = Math::sqrt(range_sq);
		rs->light_set_param(light.rs_light, RenderingServer::LIGHT_PARAM_RANGE, effective_range);

		Transform3D xform;
		xform.origin = Vector3(x3, y3, z3);

		if (light.type == LIGHT_SPOT) {
			// Project spot direction
			float sdx = -light.basis_col2[0];
			float sdy = -light.basis_col2[1];
			float sdz = -light.basis_col2[2];
			float sdw = -light.basis_col2[3];
			Vector4 sdir4(sdx, sdy, sdz, sdw);
			float sdir_x = cb0.dot(sdir4);
			float sdir_y = cb1.dot(sdir4);
			float sdir_z = cb2.dot(sdir4);
			Vector3 sdir3(sdir_x, sdir_y, sdir_z);
			float slen = sdir3.length();
			if (slen > 1e-6f) {
				sdir3 /= slen;
				Vector3 up(0, 1, 0);
				if (Math::abs(sdir3.dot(up)) > 0.999f) {
					up = Vector3(1, 0, 0);
				}
				Vector3 right = up.cross(-sdir3).normalized();
				up = (-sdir3).cross(right).normalized();
				xform.basis.set_column(0, right);
				xform.basis.set_column(1, up);
				xform.basis.set_column(2, -sdir3);
			}
		}

		rs->instance_set_transform(light.rs_instance, xform);
	}
}
