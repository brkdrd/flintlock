#pragma once

#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_interface.hpp>

using namespace godot;

class Camera4D;

class Viewport4DPanel : public VBoxContainer {
	GDCLASS(Viewport4DPanel, VBoxContainer);

	// W-axis controls
	SpinBox *_w_spinbox = nullptr;
	HSlider *_w_slider = nullptr;
	Label *_w_label = nullptr;

	real_t _w_position = 0.0f;

	void _sync_controls(double p_value);
	void _apply_w_to_cameras();
	Camera4D *_find_camera_in(Node *p_node) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_w_spinbox_changed(double p_value);
	void _on_w_slider_changed(double p_value);

public:
	real_t get_w_position() const { return _w_position; }
	void set_w_position(real_t p_w);
};
