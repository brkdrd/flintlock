#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

// Material4D wraps rendering properties for 4D surfaces after slicing.
// Internally generates a StandardMaterial3D applied to the sliced 3D mesh.
class Material4D : public Resource {
	GDCLASS(Material4D, Resource);

	Color _albedo_color = Color(1, 1, 1, 1);
	Ref<Texture2D> _albedo_texture;
	real_t _metallic = 0.0f;
	real_t _roughness = 1.0f;
	Color _emission = Color(0, 0, 0, 0);
	real_t _emission_energy = 0.0f;
	StandardMaterial3D::Transparency _transparency = StandardMaterial3D::TRANSPARENCY_DISABLED;

	mutable Ref<StandardMaterial3D> _cached_material;
	mutable bool _material_dirty = true;

protected:
	static void _bind_methods();

public:
	Color get_albedo_color() const { return _albedo_color; }
	void set_albedo_color(const Color &p_color);

	Ref<Texture2D> get_albedo_texture() const { return _albedo_texture; }
	void set_albedo_texture(const Ref<Texture2D> &p_texture);

	real_t get_metallic() const { return _metallic; }
	void set_metallic(real_t p_metallic);

	real_t get_roughness() const { return _roughness; }
	void set_roughness(real_t p_roughness);

	Color get_emission() const { return _emission; }
	void set_emission(const Color &p_emission);

	real_t get_emission_energy() const { return _emission_energy; }
	void set_emission_energy(real_t p_energy);

	// Get or create the underlying StandardMaterial3D
	Ref<StandardMaterial3D> get_standard_material() const;
};
