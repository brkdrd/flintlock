#include "texture_4d.h"

void Texture4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_texture"), &Texture4D::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &Texture4D::set_texture);
	ClassDB::bind_method(D_METHOD("get_as_texture2d"), &Texture4D::get_as_texture2d);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
}

void Texture4D::set_texture(const Ref<Texture2D> &p_texture) {
	_texture = p_texture;
	emit_changed();
}
