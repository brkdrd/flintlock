#pragma once
#include "geometry_instance_4d.h"
#include "../../resources/mesh_4d.h"
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/color.hpp>

using namespace godot;

// SpriteBase4D - base for 4D sprite nodes.
// Creates a thin slab mesh in 4D space that can be sliced by Slicer4D.
// The sprite lies flat in the hyperplane orthogonal to _axis.
// axis=0: YZW plane (sprite faces X)
// axis=1: XZW plane (sprite faces Y)
// axis=2: XYW plane (sprite faces Z)
// axis=3: XYZ plane (sprite faces W) - default
class SpriteBase4D : public GeometryInstance4D {
	GDCLASS(SpriteBase4D, GeometryInstance4D);

protected:
	Ref<Texture2D> _texture;
	Color _modulate = Color(1, 1, 1, 1);
	bool _centered = true;
	Vector2 _offset;
	real_t _pixel_size = 0.01f;
	int _axis = 3; // W axis - sprite faces the W direction
	bool _double_sided = true;

	// Cached generated mesh
	mutable Ref<ArrayMesh4D> _sprite_mesh;
	mutable bool _mesh_dirty = true;

	static void _bind_methods();
	void _notification(int p_what);

	// Build the 4D slab mesh based on the current texture/properties
	void _update_sprite_mesh();

	// Subclasses override to provide the actual texture rect to use
	virtual Rect2 _get_texture_rect() const;
	virtual Ref<Texture2D> _get_frame_texture() const { return _texture; }

public:
	// Texture
	Ref<Texture2D> get_texture() const { return _texture; }
	void set_texture(const Ref<Texture2D> &p_tex);

	// Modulate
	Color get_modulate() const { return _modulate; }
	void set_modulate(const Color &p_color);

	// Centered
	bool is_centered() const { return _centered; }
	void set_centered(bool p_centered);

	// Offset
	Vector2 get_offset() const { return _offset; }
	void set_offset(const Vector2 &p_offset);

	// Pixel size
	real_t get_pixel_size() const { return _pixel_size; }
	void set_pixel_size(real_t p_pixel_size);

	// Axis
	int get_axis() const { return _axis; }
	void set_axis(int p_axis);

	// Double sided
	bool get_double_sided() const { return _double_sided; }
	void set_double_sided(bool p_double_sided);

	// VisualInstance4D override
	Ref<Mesh4D> get_mesh_4d() const override;
};
