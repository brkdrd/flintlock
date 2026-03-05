#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/environment.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

// Environment4D: analog of Environment resource for 4D scenes.
// Delegates to a standard Godot Environment for the 3D rendering of sliced geometry.
class Environment4D : public Resource {
	GDCLASS(Environment4D, Resource);

	Color _ambient_light_color = Color(0.2f, 0.2f, 0.2f, 1.0f);
	real_t _ambient_light_energy = 1.0f;
	bool _fog_enabled = false;
	real_t _fog_density = 0.01f;
	Color _fog_color = Color(0.5f, 0.6f, 0.7f, 1.0f);
	int _background_mode = 0; // 0 = SKY, 1 = COLOR, 2 = CANVAS

	mutable Ref<Environment> _cached_env;
	mutable bool _env_dirty = true;

protected:
	static void _bind_methods();

public:
	Color get_ambient_light_color() const { return _ambient_light_color; }
	void set_ambient_light_color(const Color &p_color);

	real_t get_ambient_light_energy() const { return _ambient_light_energy; }
	void set_ambient_light_energy(real_t p_energy);

	bool get_fog_enabled() const { return _fog_enabled; }
	void set_fog_enabled(bool p_enabled);

	real_t get_fog_density() const { return _fog_density; }
	void set_fog_density(real_t p_density);

	Color get_fog_color() const { return _fog_color; }
	void set_fog_color(const Color &p_color);

	int get_background_mode() const { return _background_mode; }
	void set_background_mode(int p_mode);

	// Get the underlying Environment for use by the 3D renderer
	Ref<Environment> get_environment() const;
};
