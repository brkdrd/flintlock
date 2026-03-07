#pragma once

#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/button.hpp>

using namespace godot;

class Viewport4DPanel : public VBoxContainer {
	GDCLASS(Viewport4DPanel, VBoxContainer);

	// W-axis controls
	SpinBox *_w_spinbox = nullptr;
	HSlider *_w_slider = nullptr;
	Label *_w_label = nullptr;

	real_t _w_position = 0.0f;

	// Internal helper: sync both controls to the given value without re-triggering signals.
	void _sync_controls(double p_value);

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_w_spinbox_changed(double p_value);
	void _on_w_slider_changed(double p_value);

public:
	real_t get_w_position() const { return _w_position; }
	void set_w_position(real_t p_w);
};
