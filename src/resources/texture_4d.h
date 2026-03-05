#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// Texture4D: wraps a Texture2D for now (simplest approach).
// A future implementation could use Texture3D for volumetric texturing.
class Texture4D : public Resource {
	GDCLASS(Texture4D, Resource);

	Ref<Texture2D> _texture;

protected:
	static void _bind_methods();

public:
	Ref<Texture2D> get_texture() const { return _texture; }
	void set_texture(const Ref<Texture2D> &p_texture);

	// Get the underlying Texture2D for rendering
	Ref<Texture2D> get_as_texture2d() const { return _texture; }
};
