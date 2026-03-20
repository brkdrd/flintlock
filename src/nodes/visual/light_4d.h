#pragma once
#include "../node_4d.h"
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/rid.hpp>

using namespace godot;

// Light4D - abstract base for all 4D light types.
// Uses VisualServer4D light API instead of internal Light3D nodes.
// VS4D handles 4D→3D light projection during process_frame().
class Light4D : public Node4D {
	GDCLASS(Light4D, Node4D);

protected:
	// Properties
	Color _light_color = Color(1, 1, 1, 1);
	real_t _light_energy = 1.0f;
	real_t _light_indirect_energy = 1.0f;
	real_t _light_specular = 0.5f;
	bool _shadow_enabled = false;
	real_t _shadow_bias = 0.1f;
	real_t _shadow_normal_bias = 1.0f;
	real_t _shadow_opacity = 1.0f;
	real_t _shadow_blur = 1.0f;
	uint32_t _light_cull_mask = 0xFFFFFFFF;
	bool _light_negative = false;
	bool _editor_only = false;

	// VS4D light RID
	RID _vs_light;

	// Subclasses specify the light type (0=directional, 1=omni, 2=spot)
	virtual int _get_light_type() const = 0;

	// Push all property values to VS4D
	void _update_light_properties();

	// Push 4D transform to VS4D
	void _update_light_transform();

	// Override to propagate transform changes to VS4D
	virtual void _on_transform_4d_changed() override;

	static void _bind_methods();
	void _notification(int p_what);

public:
	// Color
	Color get_light_color() const { return _light_color; }
	void set_light_color(const Color &p_color);

	// Energy
	real_t get_light_energy() const { return _light_energy; }
	void set_light_energy(real_t p_energy);

	real_t get_light_indirect_energy() const { return _light_indirect_energy; }
	void set_light_indirect_energy(real_t p_energy);

	// Specular
	real_t get_light_specular() const { return _light_specular; }
	void set_light_specular(real_t p_specular);

	// Shadow
	bool get_shadow_enabled() const { return _shadow_enabled; }
	void set_shadow_enabled(bool p_enabled);

	real_t get_shadow_bias() const { return _shadow_bias; }
	void set_shadow_bias(real_t p_bias);

	real_t get_shadow_normal_bias() const { return _shadow_normal_bias; }
	void set_shadow_normal_bias(real_t p_bias);

	real_t get_shadow_opacity() const { return _shadow_opacity; }
	void set_shadow_opacity(real_t p_opacity);

	real_t get_shadow_blur() const { return _shadow_blur; }
	void set_shadow_blur(real_t p_blur);

	// Cull mask
	uint32_t get_light_cull_mask() const { return _light_cull_mask; }
	void set_light_cull_mask(uint32_t p_mask);

	// Flags
	bool get_light_negative() const { return _light_negative; }
	void set_light_negative(bool p_negative);

	bool get_editor_only() const { return _editor_only; }
	void set_editor_only(bool p_editor_only);
};
