#include "sprite_base_4d.h"
#include "../../resources/material_4d.h"
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;

void SpriteBase4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_texture"), &SpriteBase4D::get_texture);
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &SpriteBase4D::set_texture);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");

	ClassDB::bind_method(D_METHOD("get_modulate"), &SpriteBase4D::get_modulate);
	ClassDB::bind_method(D_METHOD("set_modulate", "color"), &SpriteBase4D::set_modulate);
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "modulate"), "set_modulate", "get_modulate");

	ClassDB::bind_method(D_METHOD("is_centered"), &SpriteBase4D::is_centered);
	ClassDB::bind_method(D_METHOD("set_centered", "centered"), &SpriteBase4D::set_centered);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "centered"), "set_centered", "is_centered");

	ClassDB::bind_method(D_METHOD("get_offset"), &SpriteBase4D::get_offset);
	ClassDB::bind_method(D_METHOD("set_offset", "offset"), &SpriteBase4D::set_offset);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "offset"), "set_offset", "get_offset");

	ClassDB::bind_method(D_METHOD("get_pixel_size"), &SpriteBase4D::get_pixel_size);
	ClassDB::bind_method(D_METHOD("set_pixel_size", "pixel_size"), &SpriteBase4D::set_pixel_size);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "pixel_size", PROPERTY_HINT_RANGE, "0.0001,128,0.0001"), "set_pixel_size", "get_pixel_size");

	ClassDB::bind_method(D_METHOD("get_axis"), &SpriteBase4D::get_axis);
	ClassDB::bind_method(D_METHOD("set_axis", "axis"), &SpriteBase4D::set_axis);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "axis", PROPERTY_HINT_ENUM, "X:0,Y:1,Z:2,W:3"), "set_axis", "get_axis");

	ClassDB::bind_method(D_METHOD("get_double_sided"), &SpriteBase4D::get_double_sided);
	ClassDB::bind_method(D_METHOD("set_double_sided", "double_sided"), &SpriteBase4D::set_double_sided);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "double_sided"), "set_double_sided", "get_double_sided");
}

void SpriteBase4D::_notification(int p_what) {
	GeometryInstance4D::_notification(p_what);

	if (p_what == NOTIFICATION_ENTER_TREE) {
		_mesh_dirty = true;
		upload_mesh();
	}
}

Rect2 SpriteBase4D::_get_texture_rect() const {
	Ref<Texture2D> tex = _get_frame_texture();
	if (tex.is_null()) return Rect2(0, 0, 1, 1);
	return Rect2(0, 0, tex->get_width(), tex->get_height());
}

void SpriteBase4D::_update_sprite_mesh() {
	_sprite_mesh = Ref<ArrayMesh4D>(memnew(ArrayMesh4D));

	Ref<Texture2D> tex = _get_frame_texture();
	Rect2 tex_rect = _get_texture_rect();

	// Determine sprite size in world units
	float w, h;
	if (tex.is_valid()) {
		w = tex_rect.size.x * _pixel_size;
		h = tex_rect.size.y * _pixel_size;
	} else {
		w = 1.0f;
		h = 1.0f;
	}

	// Apply offset
	float ox = _offset.x * _pixel_size;
	float oy = _offset.y * _pixel_size;

	// Half extents for centering
	float half_w = _centered ? w * 0.5f : 0.0f;
	float half_h = _centered ? h * 0.5f : 0.0f;

	float x0 = -half_w + ox;
	float x1 = x0 + w;
	float y0 = -half_h - oy;
	float y1 = y0 + h;

	// The sprite is a thin slab (very small extent) along the facing axis.
	// Thickness is half a pixel in 4D space to ensure it intersects the slice plane.
	float thickness = _pixel_size * 0.5f;

	// Build the 4D vertices based on axis orientation.
	// The sprite lies in the 2D plane (x,y) and has small thickness along _axis.
	// We build 8 vertices (2 parallel quads) and tesselate into tetrahedra.
	//
	// Axis mapping:
	//   axis=0 (X): sprite face is X, quad lives in YZ, extrude in X
	//   axis=1 (Y): sprite face is Y, quad lives in XZ, extrude in Y
	//   axis=2 (Z): sprite face is Z, quad lives in XY, extrude in Z
	//   axis=3 (W): sprite face is W, quad lives in XY, extrude in W (default)

	// 4 floats per vertex (x,y,z,w), 8 vertices
	PackedFloat32Array verts;
	verts.resize(8 * 4);

	// UV array: 2 floats per vertex
	PackedFloat32Array uvs;
	uvs.resize(8 * 2);

	// UVs for the texture region
	float uv_x0, uv_y0, uv_x1, uv_y1;
	if (tex.is_valid()) {
		int tw = tex->get_width();
		int th = tex->get_height();
		if (tw > 0 && th > 0) {
			uv_x0 = tex_rect.position.x / (float)tw;
			uv_y0 = tex_rect.position.y / (float)th;
			uv_x1 = (tex_rect.position.x + tex_rect.size.x) / (float)tw;
			uv_y1 = (tex_rect.position.y + tex_rect.size.y) / (float)th;
		} else {
			uv_x0 = 0; uv_y0 = 0; uv_x1 = 1; uv_y1 = 1;
		}
	} else {
		uv_x0 = 0; uv_y0 = 0; uv_x1 = 1; uv_y1 = 1;
	}

	// Helper to set vertex
	auto set_vert = [&](int idx, float a, float b, float c, float d) {
		verts[idx * 4 + 0] = a;
		verts[idx * 4 + 1] = b;
		verts[idx * 4 + 2] = c;
		verts[idx * 4 + 3] = d;
	};

	auto set_uv = [&](int idx, float u, float v) {
		uvs[idx * 2 + 0] = u;
		uvs[idx * 2 + 1] = v;
	};

	// Front face (at +thickness along axis), Back face (at -thickness along axis)
	// Front: vertices 0-3, Back: vertices 4-7
	// Vertex order: BL(0), BR(1), TR(2), TL(3)
	float t = thickness;

	if (_axis == 3) {
		// Sprite in XY plane, extrude along W
		set_vert(0, x0, y0, 0, -t);  set_uv(0, uv_x0, uv_y1);
		set_vert(1, x1, y0, 0, -t);  set_uv(1, uv_x1, uv_y1);
		set_vert(2, x1, y1, 0, -t);  set_uv(2, uv_x1, uv_y0);
		set_vert(3, x0, y1, 0, -t);  set_uv(3, uv_x0, uv_y0);
		set_vert(4, x0, y0, 0,  t);  set_uv(4, uv_x0, uv_y1);
		set_vert(5, x1, y0, 0,  t);  set_uv(5, uv_x1, uv_y1);
		set_vert(6, x1, y1, 0,  t);  set_uv(6, uv_x1, uv_y0);
		set_vert(7, x0, y1, 0,  t);  set_uv(7, uv_x0, uv_y0);
	} else if (_axis == 2) {
		// Sprite in XY plane, extrude along Z
		set_vert(0, x0, y0, -t, 0);  set_uv(0, uv_x0, uv_y1);
		set_vert(1, x1, y0, -t, 0);  set_uv(1, uv_x1, uv_y1);
		set_vert(2, x1, y1, -t, 0);  set_uv(2, uv_x1, uv_y0);
		set_vert(3, x0, y1, -t, 0);  set_uv(3, uv_x0, uv_y0);
		set_vert(4, x0, y0,  t, 0);  set_uv(4, uv_x0, uv_y1);
		set_vert(5, x1, y0,  t, 0);  set_uv(5, uv_x1, uv_y1);
		set_vert(6, x1, y1,  t, 0);  set_uv(6, uv_x1, uv_y0);
		set_vert(7, x0, y1,  t, 0);  set_uv(7, uv_x0, uv_y0);
	} else if (_axis == 1) {
		// Sprite in XZ plane, extrude along Y
		set_vert(0, x0, -t, y0, 0);  set_uv(0, uv_x0, uv_y1);
		set_vert(1, x1, -t, y0, 0);  set_uv(1, uv_x1, uv_y1);
		set_vert(2, x1, -t, y1, 0);  set_uv(2, uv_x1, uv_y0);
		set_vert(3, x0, -t, y1, 0);  set_uv(3, uv_x0, uv_y0);
		set_vert(4, x0,  t, y0, 0);  set_uv(4, uv_x0, uv_y1);
		set_vert(5, x1,  t, y0, 0);  set_uv(5, uv_x1, uv_y1);
		set_vert(6, x1,  t, y1, 0);  set_uv(6, uv_x1, uv_y0);
		set_vert(7, x0,  t, y1, 0);  set_uv(7, uv_x0, uv_y0);
	} else {
		// axis=0: Sprite in YZ plane, extrude along X
		set_vert(0, -t, x0, y0, 0);  set_uv(0, uv_x0, uv_y1);
		set_vert(1, -t, x1, y0, 0);  set_uv(1, uv_x1, uv_y1);
		set_vert(2, -t, x1, y1, 0);  set_uv(2, uv_x1, uv_y0);
		set_vert(3, -t, x0, y1, 0);  set_uv(3, uv_x0, uv_y0);
		set_vert(4,  t, x0, y0, 0);  set_uv(4, uv_x0, uv_y1);
		set_vert(5,  t, x1, y0, 0);  set_uv(5, uv_x1, uv_y1);
		set_vert(6,  t, x1, y1, 0);  set_uv(6, uv_x1, uv_y0);
		set_vert(7,  t, x0, y1, 0);  set_uv(7, uv_x0, uv_y0);
	}

	// Tetrahedra indices (groups of 4).
	// We need to tessellate the slab (two quads + 4 side quads) into tetrahedra.
	// A simple box tesselation uses 5 tetrahedra.
	// Vertices: 0-3 are "back" face, 4-7 are "front" face
	// BL=0/4, BR=1/5, TR=2/6, TL=3/7
	PackedInt32Array indices;

	// 5-tetrahedra decomposition of a box:
	// Tet 1: 0,1,3,4
	// Tet 2: 1,2,3,6
	// Tet 3: 4,5,6,1
	// Tet 4: 3,4,6,7
	// Tet 5: 1,3,4,6
	int tet_data[] = {
		0, 1, 3, 4,
		1, 2, 3, 6,
		4, 5, 6, 1,
		3, 4, 6, 7,
		1, 3, 4, 6,
	};
	indices.resize(20);
	for (int i = 0; i < 20; i++) {
		indices[i] = tet_data[i];
	}

	if (_double_sided) {
		// Mirror the mesh for the back face by duplicating with flipped normal order
		// Just add the same tets - the slab already has both sides since it has volume
		// (the slicer will produce a cross-section visible from both sides in 3D)
	}

	// Build surface arrays
	Array arrays;
	arrays.resize(Mesh4D::ARRAY_MAX);
	arrays[Mesh4D::ARRAY_VERTEX] = verts;
	arrays[Mesh4D::ARRAY_TEX_UV] = uvs;
	arrays[Mesh4D::ARRAY_INDEX] = indices;

	_sprite_mesh->add_surface(arrays);

	// Set material with texture
	if (tex.is_valid()) {
		Ref<StandardMaterial3D> mat = memnew(StandardMaterial3D);
		mat->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, tex);
		mat->set_albedo(_modulate);
		if (_double_sided) {
			mat->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
		}
		mat->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
		_sprite_mesh->set_surface_material(0, mat);
	}

	_mesh_dirty = false;
}

Ref<Mesh4D> SpriteBase4D::get_mesh_4d() const {
	if (_mesh_dirty || _sprite_mesh.is_null()) {
		const_cast<SpriteBase4D *>(this)->_update_sprite_mesh();
	}
	return _sprite_mesh;
}

// ─── Setters ──────────────────────────────────────────────────────────────────

void SpriteBase4D::set_texture(const Ref<Texture2D> &p_tex) {
	_texture = p_tex;
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_modulate(const Color &p_color) {
	_modulate = p_color;
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_centered(bool p_centered) {
	_centered = p_centered;
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_offset(const Vector2 &p_offset) {
	_offset = p_offset;
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_pixel_size(real_t p_pixel_size) {
	_pixel_size = p_pixel_size;
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_axis(int p_axis) {
	_axis = CLAMP(p_axis, 0, 3);
	_mesh_dirty = true;
	upload_mesh();
}

void SpriteBase4D::set_double_sided(bool p_double_sided) {
	_double_sided = p_double_sided;
	_mesh_dirty = true;
	upload_mesh();
}
