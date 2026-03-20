#pragma once
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/rendering_server.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/rid.hpp>
#include <godot_cpp/variant/packed_float32_array.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <cstring>

using namespace godot;

class Mesh4D;
class VisualInstance4D;

// ============================================================
// VisualServer4D — Centralized 4D rendering server
//
// Mirrors Godot's RenderingServer pattern:
//   4D Nodes → VisualServer4D (RID API) → RenderingServer → GPU
//
// Manages all RS resources internally. Nodes communicate via RIDs.
// Handles GPU-based 4D→3D hyperplane slicing (vertex shader),
// light 4D→3D projection, and camera management.
// ============================================================
class VisualServer4D : public Object {
	GDCLASS(VisualServer4D, Object);

	static VisualServer4D *singleton;

	// ── RID Management ──────────────────────────────────────────
	uint64_t _rid_counter = 1;
	RID _make_rid() {
		uint64_t id = _rid_counter++;
		RID rid;
		memcpy(rid._native_ptr(), &id, sizeof(uint64_t));
		return rid;
	}

	// ── Internal Data Structures ────────────────────────────────
	enum BaseType {
		BASE_NONE,
		BASE_MESH,
		BASE_LIGHT,
	};

	struct Instance4D {
		BaseType base_type = BASE_NONE;
		RID base_rid;          // VS4D mesh or light RID
		RID scenario;          // RS scenario RID
		bool visible = true;
		uint32_t layers = 1;

		// Source node for reading live transforms each frame
		VisualInstance4D *source_node = nullptr;

		// Material params
		Color albedo = Color(1,1,1,1);
		float roughness = 1.0f;
		float metallic = 0.0f;

		// RS resources owned by this instance
		RID rs_instance;       // RenderingServer instance
		RID rs_mesh;           // RS mesh (for MESH base type)
		bool gpu_mesh_uploaded = false;
	};

	enum LightType {
		LIGHT_DIRECTIONAL,
		LIGHT_OMNI,
		LIGHT_SPOT,
	};

	struct Light4DData {
		LightType type;
		RID rs_light;          // RenderingServer light resource
		RID rs_instance;       // RenderingServer instance for the light
		RID scenario;

		Color color = Color(1,1,1,1);
		float energy = 1.0f;
		float indirect_energy = 1.0f;
		float specular = 0.5f;
		float range = 5.0f;
		float attenuation = 1.0f;
		float spot_angle = 45.0f;
		float spot_angle_attenuation = 1.0f;
		bool shadow = false;
		float shadow_bias = 0.1f;
		float shadow_normal_bias = 1.0f;
		float shadow_opacity = 1.0f;
		float shadow_blur = 1.0f;
		bool negative = false;
		uint32_t cull_mask = 0xFFFFFFFF;

		// Directional-specific
		float shadow_max_distance = 100.0f;
		float shadow_fade_start = 0.8f;
		float shadow_pancake_size = 20.0f;

		// 4D transform (for projection)
		float basis_col0[4] = {1,0,0,0};
		float basis_col1[4] = {0,1,0,0};
		float basis_col2[4] = {0,0,1,0};
		float basis_col3[4] = {0,0,0,1};
		float origin[4] = {0,0,0,0};
		bool visible = true;
	};

	struct Mesh4DData {
		Ref<Mesh4D> mesh;
	};

	// ── Storage ─────────────────────────────────────────────────
	HashMap<RID, Instance4D> _instances;
	HashMap<RID, Mesh4DData> _meshes;
	HashMap<RID, Light4DData> _lights;

	// ── GPU Resources (shader system from Slicer4D) ─────────────
	RID _shader_rid;
	RID _material_rid;
	Ref<ImageTexture> _lut_texture;
	bool _initialized = false;

	void _initialize();
	void _generate_lut();
	String _get_shader_code() const;

	// ── Camera State ────────────────────────────────────────────
	Camera3D *_internal_camera = nullptr;
	Node *_camera_parent = nullptr;

	// Cached slice state (set per frame)
	Vector4 _slice_normal = Vector4(0,0,0,1);
	float _slice_d = 0.0f;
	float _cam_basis[12] = {1,0,0,0, 0,1,0,0, 0,0,1,0};
	Vector4 _cam_origin = Vector4(0,0,0,0);

	// ── Internal Helpers ────────────────────────────────────────
	void _upload_instance_mesh(Instance4D &inst, const Ref<Mesh4D> &mesh);
	void _update_instance_shader_transforms(Instance4D &inst);
	void _update_instance_material_params(Instance4D &inst);
	void _project_light(Light4DData &light);

protected:
	static void _bind_methods();

public:
	static VisualServer4D *get_singleton();

	VisualServer4D();
	~VisualServer4D();

	void ensure_initialized();
	RID get_shader_rid();
	RID get_material_rid();

	// ── Instance API ────────────────────────────────────────────
	RID instance_create();
	void instance_set_base(const RID &p_instance, const RID &p_base);
	void instance_set_scenario(const RID &p_instance, const RID &p_scenario);
	void instance_set_source_node(const RID &p_instance, VisualInstance4D *p_node);
	void instance_set_transform_4d(const RID &p_instance,
		const PackedFloat32Array &p_basis_cols, const Vector4 &p_origin);
	void instance_set_visible(const RID &p_instance, bool p_visible);
	void instance_set_layer_mask(const RID &p_instance, uint32_t p_mask);
	void instance_set_material_params(const RID &p_instance,
		const Color &p_albedo, float p_roughness, float p_metallic);

	// ── Mesh API ────────────────────────────────────────────────
	RID mesh_create();
	void mesh_set_data(const RID &p_mesh, const Ref<Mesh4D> &p_mesh_data);
	void mesh_clear(const RID &p_mesh);

	// ── Light API ───────────────────────────────────────────────
	RID light_create(int p_type); // 0=directional, 1=omni, 2=spot
	void light_set_scenario(const RID &p_light, const RID &p_scenario);
	void light_set_color(const RID &p_light, const Color &p_color);
	void light_set_param(const RID &p_light, int p_param, float p_value);
	void light_set_shadow(const RID &p_light, bool p_enabled);
	void light_set_negative(const RID &p_light, bool p_negative);
	void light_set_cull_mask(const RID &p_light, uint32_t p_mask);
	void light_set_transform_4d(const RID &p_light,
		const PackedFloat32Array &p_basis_cols, const Vector4 &p_origin);
	void light_set_visible(const RID &p_light, bool p_visible);

	// ── Camera API ──────────────────────────────────────────────
	void camera_attach_to_node(Node *p_node);
	void camera_detach();
	void camera_set_perspective(float p_fov, float p_near, float p_far);
	void camera_set_orthogonal(float p_size, float p_near, float p_far);
	void camera_set_cull_mask(uint32_t p_mask);
	void camera_set_h_offset(float p_offset);
	void camera_set_v_offset(float p_offset);
	void camera_set_environment(const Ref<Environment> &p_env);
	void camera_set_attributes(const Ref<CameraAttributes> &p_attrs);
	void camera_make_current();
	void camera_clear_current(bool p_enable_next = true);
	Camera3D *camera_get_internal() const { return _internal_camera; }

	// ── Frame Processing ────────────────────────────────────────
	// Called by Camera4D each frame with the current slice state
	void process_frame(const Vector4 &p_plane_normal, float p_plane_d,
		const PackedFloat32Array &p_basis_cols, const Vector4 &p_camera_origin);

	// ── Cleanup ─────────────────────────────────────────────────
	void free_rid(const RID &p_rid);

	// ── Diagnostics ─────────────────────────────────────────────
	int get_instance_count() const { return _instances.size(); }
	int get_light_count() const { return _lights.size(); }
};
