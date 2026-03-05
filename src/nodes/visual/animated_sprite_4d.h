#pragma once
#include "sprite_base_4d.h"
#include "../../resources/sprite_frames_4d.h"
#include <godot_cpp/variant/string_name.hpp>

using namespace godot;

// AnimatedSprite4D - 4D sprite with frame-based animation driven by SpriteFrames4D.
// Advances frames each _process() call based on the animation speed and speed_scale.
class AnimatedSprite4D : public SpriteBase4D {
	GDCLASS(AnimatedSprite4D, SpriteBase4D);

	Ref<SpriteFrames4D> _sprite_frames;
	StringName _animation = StringName("default");
	int _frame = 0;
	float _frame_progress = 0.0f;
	float _speed_scale = 1.0f;
	bool _playing = false;
	bool _loop = true;

	// Cached frame texture for current animation frame
	mutable Ref<Texture2D> _cached_frame_texture;
	mutable bool _frame_texture_dirty = true;

	void _advance_frame(double p_delta);
	void _update_frame_texture();

protected:
	Ref<Texture2D> _get_frame_texture() const override;
	Rect2 _get_texture_rect() const override;

	static void _bind_methods();
	void _notification(int p_what);

public:
	// SpriteFrames4D resource
	Ref<SpriteFrames4D> get_sprite_frames() const { return _sprite_frames; }
	void set_sprite_frames(const Ref<SpriteFrames4D> &p_frames);

	// Animation name
	StringName get_animation() const { return _animation; }
	void set_animation(const StringName &p_animation);

	// Current frame
	int get_frame() const { return _frame; }
	void set_frame(int p_frame);

	// Frame progress (0..1 within current frame)
	float get_frame_progress() const { return _frame_progress; }
	void set_frame_progress(float p_progress);

	// Speed scale
	float get_speed_scale() const { return _speed_scale; }
	void set_speed_scale(float p_scale);

	// Playback control
	bool is_playing() const { return _playing; }
	void play(const StringName &p_animation = StringName(), float p_custom_speed = 1.0f, bool p_from_end = false);
	void play_backwards(const StringName &p_animation = StringName());
	void pause();
	void stop();

	// Loop
	bool get_loop() const { return _loop; }
	void set_loop(bool p_loop);

	// Info
	int get_frame_count() const;
	double get_animation_speed() const;
};
