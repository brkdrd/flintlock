#include "animated_sprite_4d.h"

using namespace godot;

void AnimatedSprite4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_sprite_frames"), &AnimatedSprite4D::get_sprite_frames);
	ClassDB::bind_method(D_METHOD("set_sprite_frames", "frames"), &AnimatedSprite4D::set_sprite_frames);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "sprite_frames", PROPERTY_HINT_RESOURCE_TYPE, "SpriteFrames4D"), "set_sprite_frames", "get_sprite_frames");

	ClassDB::bind_method(D_METHOD("get_animation"), &AnimatedSprite4D::get_animation);
	ClassDB::bind_method(D_METHOD("set_animation", "animation"), &AnimatedSprite4D::set_animation);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "animation"), "set_animation", "get_animation");

	ClassDB::bind_method(D_METHOD("get_frame"), &AnimatedSprite4D::get_frame);
	ClassDB::bind_method(D_METHOD("set_frame", "frame"), &AnimatedSprite4D::set_frame);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame"), "set_frame", "get_frame");

	ClassDB::bind_method(D_METHOD("get_frame_progress"), &AnimatedSprite4D::get_frame_progress);
	ClassDB::bind_method(D_METHOD("set_frame_progress", "progress"), &AnimatedSprite4D::set_frame_progress);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "frame_progress", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_frame_progress", "get_frame_progress");

	ClassDB::bind_method(D_METHOD("get_speed_scale"), &AnimatedSprite4D::get_speed_scale);
	ClassDB::bind_method(D_METHOD("set_speed_scale", "scale"), &AnimatedSprite4D::set_speed_scale);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed_scale", PROPERTY_HINT_RANGE, "-32,32,0.01"), "set_speed_scale", "get_speed_scale");

	ClassDB::bind_method(D_METHOD("get_loop"), &AnimatedSprite4D::get_loop);
	ClassDB::bind_method(D_METHOD("set_loop", "loop"), &AnimatedSprite4D::set_loop);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "loop"), "set_loop", "get_loop");

	ClassDB::bind_method(D_METHOD("is_playing"), &AnimatedSprite4D::is_playing);
	ClassDB::bind_method(D_METHOD("play", "animation", "custom_speed", "from_end"), &AnimatedSprite4D::play, DEFVAL(StringName()), DEFVAL(1.0f), DEFVAL(false));
	ClassDB::bind_method(D_METHOD("play_backwards", "animation"), &AnimatedSprite4D::play_backwards, DEFVAL(StringName()));
	ClassDB::bind_method(D_METHOD("pause"), &AnimatedSprite4D::pause);
	ClassDB::bind_method(D_METHOD("stop"), &AnimatedSprite4D::stop);

	ClassDB::bind_method(D_METHOD("get_frame_count"), &AnimatedSprite4D::get_frame_count);
	ClassDB::bind_method(D_METHOD("get_animation_speed"), &AnimatedSprite4D::get_animation_speed);

	ADD_SIGNAL(MethodInfo("animation_finished"));
	ADD_SIGNAL(MethodInfo("animation_looped"));
	ADD_SIGNAL(MethodInfo("frame_changed"));
}

void AnimatedSprite4D::_notification(int p_what) {
	SpriteBase4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			set_process(_playing);
		} break;

		case NOTIFICATION_PROCESS: {
			double delta = get_process_delta_time();
			if (_playing) {
				_advance_frame(delta);
			}
		} break;
	}
}

void AnimatedSprite4D::_advance_frame(double p_delta) {
	if (_sprite_frames.is_null()) return;
	if (!_sprite_frames->has_animation(_animation)) return;

	double fps = _sprite_frames->get_animation_speed(_animation) * (double)_speed_scale;
	if (Math::abs(fps) < 1e-6) return;

	int frame_count = _sprite_frames->get_frame_count(_animation);
	if (frame_count <= 0) return;

	bool anim_loop = _loop && _sprite_frames->get_animation_loop(_animation);

	_frame_progress += (float)(fps * p_delta);

	// Handle frame advancement
	while (_frame_progress >= 1.0f) {
		_frame_progress -= 1.0f;
		_frame++;
		if (_frame >= frame_count) {
			if (anim_loop) {
				_frame = 0;
				emit_signal("animation_looped");
			} else {
				_frame = frame_count - 1;
				_frame_progress = 1.0f;
				_playing = false;
				set_process(false);
				emit_signal("animation_finished");
				break;
			}
		}
		emit_signal("frame_changed");
		_frame_texture_dirty = true;
		_mesh_dirty = true;
	}

	// Handle negative speed (playing backwards)
	while (_frame_progress < 0.0f) {
		_frame_progress += 1.0f;
		_frame--;
		if (_frame < 0) {
			if (anim_loop) {
				_frame = frame_count - 1;
				emit_signal("animation_looped");
			} else {
				_frame = 0;
				_frame_progress = 0.0f;
				_playing = false;
				set_process(false);
				emit_signal("animation_finished");
				break;
			}
		}
		emit_signal("frame_changed");
		_frame_texture_dirty = true;
		_mesh_dirty = true;
	}

	if (_mesh_dirty) {
		upload_mesh();
	}
}

void AnimatedSprite4D::_update_frame_texture() {
	if (_sprite_frames.is_null() || !_sprite_frames->has_animation(_animation)) {
		_cached_frame_texture = Ref<Texture2D>();
	} else {
		int frame_count = _sprite_frames->get_frame_count(_animation);
		int safe_frame = CLAMP(_frame, 0, frame_count > 0 ? frame_count - 1 : 0);
		_cached_frame_texture = _sprite_frames->get_frame_texture(_animation, safe_frame);
	}
	_frame_texture_dirty = false;
}

Ref<Texture2D> AnimatedSprite4D::_get_frame_texture() const {
	if (_frame_texture_dirty) {
		const_cast<AnimatedSprite4D *>(this)->_update_frame_texture();
	}
	return _cached_frame_texture;
}

Rect2 AnimatedSprite4D::_get_texture_rect() const {
	Ref<Texture2D> tex = _get_frame_texture();
	if (tex.is_null()) return Rect2(0, 0, 1, 1);
	return Rect2(0, 0, tex->get_width(), tex->get_height());
}

// ─── Public API ───────────────────────────────────────────────────────────────

void AnimatedSprite4D::set_sprite_frames(const Ref<SpriteFrames4D> &p_frames) {
	_sprite_frames = p_frames;
	_frame = 0;
	_frame_progress = 0.0f;
	_frame_texture_dirty = true;
	_mesh_dirty = true;
	upload_mesh();
}

void AnimatedSprite4D::set_animation(const StringName &p_animation) {
	if (_animation == p_animation) return;
	_animation = p_animation;
	_frame = 0;
	_frame_progress = 0.0f;
	_frame_texture_dirty = true;
	_mesh_dirty = true;
	upload_mesh();
}

void AnimatedSprite4D::set_frame(int p_frame) {
	int fc = get_frame_count();
	_frame = CLAMP(p_frame, 0, fc > 0 ? fc - 1 : 0);
	_frame_progress = 0.0f;
	_frame_texture_dirty = true;
	_mesh_dirty = true;
	emit_signal("frame_changed");
	upload_mesh();
}

void AnimatedSprite4D::set_frame_progress(float p_progress) {
	_frame_progress = CLAMP(p_progress, 0.0f, 1.0f);
}

void AnimatedSprite4D::set_speed_scale(float p_scale) {
	_speed_scale = p_scale;
}

void AnimatedSprite4D::play(const StringName &p_animation, float p_custom_speed, bool p_from_end) {
	if (p_animation != StringName()) {
		set_animation(p_animation);
	}
	_speed_scale = (p_custom_speed != 1.0f) ? p_custom_speed : _speed_scale;
	if (p_from_end) {
		int fc = get_frame_count();
		_frame = fc > 0 ? fc - 1 : 0;
		_frame_progress = 0.0f;
	} else if (!_playing) {
		// Don't reset frame if already at a position
	}
	_playing = true;
	if (is_inside_tree()) set_process(true);
}

void AnimatedSprite4D::play_backwards(const StringName &p_animation) {
	play(p_animation, -Math::abs(_speed_scale), true);
}

void AnimatedSprite4D::pause() {
	_playing = false;
	if (is_inside_tree()) set_process(false);
}

void AnimatedSprite4D::stop() {
	_playing = false;
	_frame = 0;
	_frame_progress = 0.0f;
	_frame_texture_dirty = true;
	_mesh_dirty = true;
	if (is_inside_tree()) set_process(false);
	upload_mesh();
}

void AnimatedSprite4D::set_loop(bool p_loop) {
	_loop = p_loop;
}

int AnimatedSprite4D::get_frame_count() const {
	if (_sprite_frames.is_null() || !_sprite_frames->has_animation(_animation)) return 0;
	return _sprite_frames->get_frame_count(_animation);
}

double AnimatedSprite4D::get_animation_speed() const {
	if (_sprite_frames.is_null() || !_sprite_frames->has_animation(_animation)) return 0.0;
	return _sprite_frames->get_animation_speed(_animation);
}
