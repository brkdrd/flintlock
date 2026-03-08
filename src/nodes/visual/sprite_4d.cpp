#include "sprite_4d.h"
#include "../../slicer/slicer_4d.h"

using namespace godot;

void Sprite4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_hframes"), &Sprite4D::get_hframes);
	ClassDB::bind_method(D_METHOD("set_hframes", "hframes"), &Sprite4D::set_hframes);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_hframes", "get_hframes");

	ClassDB::bind_method(D_METHOD("get_vframes"), &Sprite4D::get_vframes);
	ClassDB::bind_method(D_METHOD("set_vframes", "vframes"), &Sprite4D::set_vframes);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "vframes", PROPERTY_HINT_RANGE, "1,16384,1"), "set_vframes", "get_vframes");

	ClassDB::bind_method(D_METHOD("get_frame"), &Sprite4D::get_frame);
	ClassDB::bind_method(D_METHOD("set_frame", "frame"), &Sprite4D::set_frame);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "frame", PROPERTY_HINT_RANGE, "0,16383,1"), "set_frame", "get_frame");

	ClassDB::bind_method(D_METHOD("get_region_enabled"), &Sprite4D::get_region_enabled);
	ClassDB::bind_method(D_METHOD("set_region_enabled", "enabled"), &Sprite4D::set_region_enabled);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "region_enabled"), "set_region_enabled", "get_region_enabled");

	ClassDB::bind_method(D_METHOD("get_region_rect"), &Sprite4D::get_region_rect);
	ClassDB::bind_method(D_METHOD("set_region_rect", "rect"), &Sprite4D::set_region_rect);
	ADD_PROPERTY(PropertyInfo(Variant::RECT2, "region_rect"), "set_region_rect", "get_region_rect");

	ClassDB::bind_method(D_METHOD("get_frame_count"), &Sprite4D::get_frame_count);
}

Rect2 Sprite4D::_get_texture_rect() const {
	if (_region_enabled) {
		return _region_rect;
	}

	Ref<Texture2D> tex = _texture;
	if (tex.is_null()) return Rect2(0, 0, 1, 1);

	int tw = tex->get_width();
	int th = tex->get_height();

	int frame_w = tw / MAX(_hframes, 1);
	int frame_h = th / MAX(_vframes, 1);

	int total_frames = _hframes * _vframes;
	int frame = CLAMP(_frame, 0, total_frames - 1);

	int col = frame % _hframes;
	int row = frame / _hframes;

	return Rect2(col * frame_w, row * frame_h, frame_w, frame_h);
}

void Sprite4D::set_hframes(int p_hframes) {
	_hframes = MAX(1, p_hframes);
	// Clamp frame index
	if (_frame >= _hframes * _vframes) {
		_frame = _hframes * _vframes - 1;
	}
	_mesh_dirty = true;
	upload_gpu_mesh();
}

void Sprite4D::set_vframes(int p_vframes) {
	_vframes = MAX(1, p_vframes);
	// Clamp frame index
	if (_frame >= _hframes * _vframes) {
		_frame = _hframes * _vframes - 1;
	}
	_mesh_dirty = true;
	upload_gpu_mesh();
}

void Sprite4D::set_frame(int p_frame) {
	_frame = CLAMP(p_frame, 0, MAX(_hframes * _vframes - 1, 0));
	_mesh_dirty = true;
	upload_gpu_mesh();
}

void Sprite4D::set_region_enabled(bool p_enabled) {
	_region_enabled = p_enabled;
	_mesh_dirty = true;
	upload_gpu_mesh();
}

void Sprite4D::set_region_rect(const Rect2 &p_rect) {
	_region_rect = p_rect;
	if (_region_enabled) {
		_mesh_dirty = true;
		upload_gpu_mesh();
	}
}
