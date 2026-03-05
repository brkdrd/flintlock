#include "viewport_4d_panel.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>

using namespace godot;

void Viewport4DPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_w_position"), &Viewport4DPanel::get_w_position);
	ClassDB::bind_method(D_METHOD("set_w_position", "w"), &Viewport4DPanel::set_w_position);

	ClassDB::bind_method(D_METHOD("_on_w_spinbox_changed", "value"), &Viewport4DPanel::_on_w_spinbox_changed);
	ClassDB::bind_method(D_METHOD("_on_w_slider_changed", "value"), &Viewport4DPanel::_on_w_slider_changed);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "w_position"), "set_w_position", "get_w_position");
}

void Viewport4DPanel::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY) {
		// Title label
		Label *title = memnew(Label);
		title->set_text("4D View Controls");
		add_child(title);

		// Row: W position label + spinbox + slider
		HBoxContainer *hbox = memnew(HBoxContainer);
		add_child(hbox);

		_w_label = memnew(Label);
		_w_label->set_text("W Position:");
		hbox->add_child(_w_label);

		_w_spinbox = memnew(SpinBox);
		_w_spinbox->set_min(-1000.0);
		_w_spinbox->set_max(1000.0);
		_w_spinbox->set_step(0.1);
		_w_spinbox->set_value(0.0);
		_w_spinbox->connect("value_changed", Callable(this, "_on_w_spinbox_changed"));
		hbox->add_child(_w_spinbox);

		_w_slider = memnew(HSlider);
		_w_slider->set_min(-100.0);
		_w_slider->set_max(100.0);
		_w_slider->set_step(0.01);
		_w_slider->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		_w_slider->connect("value_changed", Callable(this, "_on_w_slider_changed"));
		hbox->add_child(_w_slider);
	}
}

void Viewport4DPanel::_sync_controls(double p_value) {
	// Block re-entrant signal loops by setting values without emitting signals.
	if (_w_spinbox) {
		_w_spinbox->set_value_no_signal(p_value);
	}
	if (_w_slider) {
		_w_slider->set_value_no_signal(p_value);
	}
}

void Viewport4DPanel::_on_w_spinbox_changed(double p_value) {
	_w_position = static_cast<real_t>(p_value);
	if (_w_slider) {
		_w_slider->set_value_no_signal(p_value);
	}
}

void Viewport4DPanel::_on_w_slider_changed(double p_value) {
	_w_position = static_cast<real_t>(p_value);
	if (_w_spinbox) {
		_w_spinbox->set_value_no_signal(p_value);
	}
}

void Viewport4DPanel::set_w_position(real_t p_w) {
	_w_position = p_w;
	_sync_controls(static_cast<double>(p_w));
}
