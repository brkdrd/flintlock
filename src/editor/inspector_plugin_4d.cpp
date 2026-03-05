#include "inspector_plugin_4d.h"

using namespace godot;

void EditorInspectorPlugin4D::_bind_methods() {}

bool EditorInspectorPlugin4D::_can_handle(Object *p_object) const {
	if (!p_object) {
		return false;
	}
	String class_name = p_object->get_class();
	return class_name.ends_with("4D") || class_name.contains("4D");
}

bool EditorInspectorPlugin4D::_parse_property(Object *p_object, Variant::Type p_type,
		const String &p_name, PropertyHint p_hint_type, const String &p_hint_string,
		BitField<PropertyUsageFlags> p_usage_flags, bool p_wide) {
	// Return false to use default property editors.
	// Full custom editors for Vector4D, Basis4D, etc. can be added later.
	return false;
}
