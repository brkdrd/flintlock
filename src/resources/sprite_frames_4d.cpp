#include "sprite_frames_4d.h"

void SpriteFrames4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add_animation", "anim"), &SpriteFrames4D::add_animation);
	ClassDB::bind_method(D_METHOD("has_animation", "anim"), &SpriteFrames4D::has_animation);
	ClassDB::bind_method(D_METHOD("remove_animation", "anim"), &SpriteFrames4D::remove_animation);
	ClassDB::bind_method(D_METHOD("rename_animation", "anim", "newname"), &SpriteFrames4D::rename_animation);
	ClassDB::bind_method(D_METHOD("get_animation_names"), &SpriteFrames4D::get_animation_names);
	ClassDB::bind_method(D_METHOD("set_animation_speed", "anim", "fps"), &SpriteFrames4D::set_animation_speed);
	ClassDB::bind_method(D_METHOD("get_animation_speed", "anim"), &SpriteFrames4D::get_animation_speed);
	ClassDB::bind_method(D_METHOD("set_animation_loop", "anim", "loop"), &SpriteFrames4D::set_animation_loop);
	ClassDB::bind_method(D_METHOD("get_animation_loop", "anim"), &SpriteFrames4D::get_animation_loop);
	ClassDB::bind_method(D_METHOD("add_frame", "anim", "texture", "duration", "at_pos"), &SpriteFrames4D::add_frame, DEFVAL(1.0f), DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("get_frame_count", "anim"), &SpriteFrames4D::get_frame_count);
	ClassDB::bind_method(D_METHOD("get_frame_texture", "anim", "idx"), &SpriteFrames4D::get_frame_texture);
	ClassDB::bind_method(D_METHOD("set_frame_texture", "anim", "idx", "texture"), &SpriteFrames4D::set_frame_texture);
	ClassDB::bind_method(D_METHOD("remove_frame", "anim", "idx"), &SpriteFrames4D::remove_frame);
	ClassDB::bind_method(D_METHOD("clear", "anim"), &SpriteFrames4D::clear);
	ClassDB::bind_method(D_METHOD("clear_all"), &SpriteFrames4D::clear_all);
}

void SpriteFrames4D::add_animation(const StringName &p_anim) {
	ERR_FAIL_COND_MSG(_animations.has(p_anim), "Animation already exists: " + String(p_anim));
	_animations[p_anim] = AnimData();
	emit_changed();
}

bool SpriteFrames4D::has_animation(const StringName &p_anim) const {
	return _animations.has(p_anim);
}

void SpriteFrames4D::remove_animation(const StringName &p_anim) {
	_animations.erase(p_anim);
	emit_changed();
}

void SpriteFrames4D::rename_animation(const StringName &p_anim, const StringName &p_newname) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	AnimData data = _animations[p_anim];
	_animations.erase(p_anim);
	_animations[p_newname] = data;
	emit_changed();
}

PackedStringArray SpriteFrames4D::get_animation_names() const {
	PackedStringArray names;
	for (const KeyValue<StringName, AnimData> &kv : _animations) {
		names.push_back(kv.key);
	}
	names.sort();
	return names;
}

void SpriteFrames4D::set_animation_speed(const StringName &p_anim, double p_fps) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	_animations[p_anim].speed = p_fps;
	emit_changed();
}

double SpriteFrames4D::get_animation_speed(const StringName &p_anim) const {
	ERR_FAIL_COND_V(!_animations.has(p_anim), 0.0);
	return _animations[p_anim].speed;
}

void SpriteFrames4D::set_animation_loop(const StringName &p_anim, bool p_loop) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	_animations[p_anim].loop = p_loop;
	emit_changed();
}

bool SpriteFrames4D::get_animation_loop(const StringName &p_anim) const {
	ERR_FAIL_COND_V(!_animations.has(p_anim), false);
	return _animations[p_anim].loop;
}

void SpriteFrames4D::add_frame(const StringName &p_anim, const Ref<Texture2D> &p_frame, float p_duration, int p_at_pos) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	AnimData &ad = _animations[p_anim];
	if (p_at_pos < 0 || p_at_pos >= ad.frames.size()) {
		ad.frames.push_back(p_frame);
	} else {
		ad.frames.insert(p_at_pos, p_frame);
	}
	emit_changed();
}

int SpriteFrames4D::get_frame_count(const StringName &p_anim) const {
	ERR_FAIL_COND_V(!_animations.has(p_anim), 0);
	return _animations[p_anim].frames.size();
}

Ref<Texture2D> SpriteFrames4D::get_frame_texture(const StringName &p_anim, int p_idx) const {
	ERR_FAIL_COND_V(!_animations.has(p_anim), Ref<Texture2D>());
	const AnimData &ad = _animations[p_anim];
	ERR_FAIL_INDEX_V(p_idx, ad.frames.size(), Ref<Texture2D>());
	return ad.frames[p_idx];
}

void SpriteFrames4D::set_frame_texture(const StringName &p_anim, int p_idx, const Ref<Texture2D> &p_texture) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	AnimData &ad = _animations[p_anim];
	ERR_FAIL_INDEX(p_idx, ad.frames.size());
	ad.frames[p_idx] = p_texture;
	emit_changed();
}

void SpriteFrames4D::remove_frame(const StringName &p_anim, int p_idx) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	AnimData &ad = _animations[p_anim];
	ERR_FAIL_INDEX(p_idx, ad.frames.size());
	ad.frames.remove_at(p_idx);
	emit_changed();
}

void SpriteFrames4D::clear(const StringName &p_anim) {
	ERR_FAIL_COND(!_animations.has(p_anim));
	_animations[p_anim].frames.clear();
	emit_changed();
}

void SpriteFrames4D::clear_all() {
	_animations.clear();
	emit_changed();
}
