#pragma once
#include "sprite_base_4d.h"
#include <godot_cpp/variant/rect2.hpp>

using namespace godot;

// Sprite4D - single-frame (or spritesheet) 4D sprite.
// Supports hframes/vframes for sprite sheets and frame selection.
// Also supports region clipping (region_enabled + region_rect).
class Sprite4D : public SpriteBase4D {
	GDCLASS(Sprite4D, SpriteBase4D);

	int _hframes = 1;
	int _vframes = 1;
	int _frame = 0;
	bool _region_enabled = false;
	Rect2 _region_rect;

protected:
	Rect2 _get_texture_rect() const override;
	Ref<Texture2D> _get_frame_texture() const override { return _texture; }

	static void _bind_methods();

public:
	int get_hframes() const { return _hframes; }
	void set_hframes(int p_hframes);

	int get_vframes() const { return _vframes; }
	void set_vframes(int p_vframes);

	int get_frame() const { return _frame; }
	void set_frame(int p_frame);

	bool get_region_enabled() const { return _region_enabled; }
	void set_region_enabled(bool p_enabled);

	Rect2 get_region_rect() const { return _region_rect; }
	void set_region_rect(const Rect2 &p_rect);

	int get_frame_count() const { return _hframes * _vframes; }
};
