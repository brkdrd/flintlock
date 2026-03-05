#pragma once
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

// SpriteFrames4D: analog of SpriteFrames.
// Holds named animations, each being a sequence of Texture2D frames.
class SpriteFrames4D : public Resource {
	GDCLASS(SpriteFrames4D, Resource);

	struct AnimData {
		double speed = 5.0;
		bool loop = true;
		TypedArray<Texture2D> frames;
	};

	HashMap<StringName, AnimData> _animations;

protected:
	static void _bind_methods();

public:
	// Animation management
	void add_animation(const StringName &p_anim);
	bool has_animation(const StringName &p_anim) const;
	void remove_animation(const StringName &p_anim);
	void rename_animation(const StringName &p_anim, const StringName &p_newname);
	PackedStringArray get_animation_names() const;

	void set_animation_speed(const StringName &p_anim, double p_fps);
	double get_animation_speed(const StringName &p_anim) const;

	void set_animation_loop(const StringName &p_anim, bool p_loop);
	bool get_animation_loop(const StringName &p_anim) const;

	void add_frame(const StringName &p_anim, const Ref<Texture2D> &p_frame, float p_duration = 1.0f, int p_at_pos = -1);
	int get_frame_count(const StringName &p_anim) const;
	Ref<Texture2D> get_frame_texture(const StringName &p_anim, int p_idx) const;
	void set_frame_texture(const StringName &p_anim, int p_idx, const Ref<Texture2D> &p_texture);
	void remove_frame(const StringName &p_anim, int p_idx);
	void clear(const StringName &p_anim);
	void clear_all();
};
