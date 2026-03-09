#include "camera_4d.h"
#include "../math/basis4d.h"
#include "../math/transform4d.h"
#include "../math/vector4d.h"
#include <godot_cpp/classes/engine.hpp>

void Camera4D::_bind_methods() {
	BIND_ENUM_CONSTANT(PROJECTION_PERSPECTIVE);
	BIND_ENUM_CONSTANT(PROJECTION_ORTHOGONAL);
	BIND_ENUM_CONSTANT(PROJECTION_FRUSTUM);
	BIND_ENUM_CONSTANT(KEEP_WIDTH);
	BIND_ENUM_CONSTANT(KEEP_HEIGHT);

	ClassDB::bind_method(D_METHOD("make_current"), &Camera4D::make_current);
	ClassDB::bind_method(D_METHOD("clear_current", "enable_next"), &Camera4D::clear_current, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_current"), &Camera4D::is_current);

	ClassDB::bind_method(D_METHOD("get_fov"), &Camera4D::get_fov);
	ClassDB::bind_method(D_METHOD("set_fov", "fov"), &Camera4D::set_fov);
	ClassDB::bind_method(D_METHOD("get_size"), &Camera4D::get_size);
	ClassDB::bind_method(D_METHOD("set_size", "size"), &Camera4D::set_size);
	ClassDB::bind_method(D_METHOD("get_near"), &Camera4D::get_near);
	ClassDB::bind_method(D_METHOD("set_near", "near"), &Camera4D::set_near);
	ClassDB::bind_method(D_METHOD("get_far"), &Camera4D::get_far);
	ClassDB::bind_method(D_METHOD("set_far", "far"), &Camera4D::set_far);
	ClassDB::bind_method(D_METHOD("get_projection"), &Camera4D::get_projection);
	ClassDB::bind_method(D_METHOD("set_projection", "projection"), &Camera4D::set_projection);
	ClassDB::bind_method(D_METHOD("get_keep_aspect"), &Camera4D::get_keep_aspect);
	ClassDB::bind_method(D_METHOD("set_keep_aspect", "keep_aspect"), &Camera4D::set_keep_aspect);
	ClassDB::bind_method(D_METHOD("get_cull_mask"), &Camera4D::get_cull_mask);
	ClassDB::bind_method(D_METHOD("set_cull_mask", "mask"), &Camera4D::set_cull_mask);
	ClassDB::bind_method(D_METHOD("get_h_offset"), &Camera4D::get_h_offset);
	ClassDB::bind_method(D_METHOD("set_h_offset", "offset"), &Camera4D::set_h_offset);
	ClassDB::bind_method(D_METHOD("get_v_offset"), &Camera4D::get_v_offset);
	ClassDB::bind_method(D_METHOD("set_v_offset", "offset"), &Camera4D::set_v_offset);
	ClassDB::bind_method(D_METHOD("get_slice_offset"), &Camera4D::get_slice_offset);
	ClassDB::bind_method(D_METHOD("set_slice_offset", "offset"), &Camera4D::set_slice_offset);
	ClassDB::bind_method(D_METHOD("get_environment"), &Camera4D::get_environment);
	ClassDB::bind_method(D_METHOD("set_environment", "env"), &Camera4D::set_environment);
	ClassDB::bind_method(D_METHOD("get_attributes"), &Camera4D::get_attributes);
	ClassDB::bind_method(D_METHOD("set_attributes", "attrs"), &Camera4D::set_attributes);
	ClassDB::bind_method(D_METHOD("get_slice_plane_normal"), &Camera4D::get_slice_plane_normal);
	ClassDB::bind_method(D_METHOD("get_slice_plane_distance"), &Camera4D::get_slice_plane_distance);

	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "current"), "", "is_current");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "projection", PROPERTY_HINT_ENUM, "Perspective,Orthogonal,Frustum"), "set_projection", "get_projection");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "fov", PROPERTY_HINT_RANGE, "1,179,0.1"), "set_fov", "get_fov");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "size", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_size", "get_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "near", PROPERTY_HINT_RANGE, "0.001,100,0.001"), "set_near", "get_near");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "far", PROPERTY_HINT_RANGE, "0.01,4000,0.01"), "set_far", "get_far");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "keep_aspect", PROPERTY_HINT_ENUM, "Keep Width,Keep Height"), "set_keep_aspect", "get_keep_aspect");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "cull_mask", PROPERTY_HINT_LAYERS_3D_RENDER), "set_cull_mask", "get_cull_mask");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "h_offset"), "set_h_offset", "get_h_offset");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "v_offset"), "set_v_offset", "get_v_offset");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "slice_offset"), "set_slice_offset", "get_slice_offset");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "environment", PROPERTY_HINT_RESOURCE_TYPE, "Environment"), "set_environment", "get_environment");
}

Camera4D::Camera4D() {}

void Camera4D::_notification(int p_what) {
	Node4D::_notification(p_what);

	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			// Create internal Camera3D as child
			_internal_camera = memnew(Camera3D);
			add_child(_internal_camera);
			_internal_camera->set_name("__InternalCamera3D__");
			// Prevent it from showing up easily in the editor
			_update_camera3d_properties();
			if (_current) {
				_internal_camera->make_current();
			}
			set_process(true);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			set_process(false);
			if (_internal_camera) {
				_internal_camera->queue_free();
				_internal_camera = nullptr;
			}
		} break;

		case NOTIFICATION_PROCESS: {
			_perform_slice();
		} break;

		case NOTIFICATION_TRANSFORM_4D_CHANGED: {
			// GPU slicer handles camera changes via uniform updates each frame.
		} break;
	}
}

void Camera4D::_update_camera3d_properties() {
	if (!_internal_camera) return;
	_internal_camera->set_fov(_fov);
	_internal_camera->set_near(_near);
	_internal_camera->set_far(_far);
	_internal_camera->set_cull_mask(_cull_mask);
	_internal_camera->set_h_offset(_h_offset);
	_internal_camera->set_v_offset(_v_offset);
	if (_environment.is_valid()) {
		_internal_camera->set_environment(_environment);
	}
	if (_attributes.is_valid()) {
		_internal_camera->set_attributes(_attributes);
	}
	// Set projection
	switch (_projection) {
		case PROJECTION_PERSPECTIVE:
			_internal_camera->set_perspective(_fov, _near, _far);
			break;
		case PROJECTION_ORTHOGONAL:
			_internal_camera->set_orthogonal(_size, _near, _far);
			break;
		default:
			break;
	}
}

void Camera4D::_perform_slice() {
	if (!Slicer4D::get_singleton()) return;

	// Get global 4D transform
	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return;

	Ref<Basis4D> basis = gt->get_basis();
	Ref<Vector4D> origin = gt->get_origin();
	if (basis.is_null() || origin.is_null()) return;

	// Slice plane: W column of basis is the normal, distance = normal.dot(origin) + slice_offset
	Ref<Vector4D> plane_normal_vec = basis->get_column(3);
	if (plane_normal_vec.is_null()) return;

	Vector4 plane_normal = Vector4(plane_normal_vec->x, plane_normal_vec->y, plane_normal_vec->z, plane_normal_vec->w);
	float plane_d = plane_normal.dot(Vector4(origin->x, origin->y, origin->z, origin->w)) + _slice_offset;

	// Basis columns 0,1,2 are the 3D axes within the slice
	PackedFloat32Array basis_cols;
	basis_cols.resize(12);
	for (int col = 0; col < 3; col++) {
		Ref<Vector4D> c = basis->get_column(col);
		if (c.is_null()) {
			basis_cols[col*4+0] = (col == 0) ? 1.0f : 0.0f;
			basis_cols[col*4+1] = (col == 1) ? 1.0f : 0.0f;
			basis_cols[col*4+2] = (col == 2) ? 1.0f : 0.0f;
			basis_cols[col*4+3] = 0.0f;
		} else {
			basis_cols[col*4+0] = c->x;
			basis_cols[col*4+1] = c->y;
			basis_cols[col*4+2] = c->z;
			basis_cols[col*4+3] = c->w;
		}
	}

	Vector4 camera_origin(origin->x, origin->y, origin->z, origin->w);
	Slicer4D::get_singleton()->update_frame(plane_normal, plane_d, basis_cols, camera_origin);

	// Camera3D stays at origin — geometry is projected relative to camera origin
	// in the vertex shader, so the 3D camera needs no offset.
	if (_internal_camera) {
		_internal_camera->set_global_transform(Transform3D());
	}
}

Vector4 Camera4D::get_slice_plane_normal() const {
	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return Vector4(0, 0, 0, 1);
	Ref<Basis4D> basis = gt->get_basis();
	if (basis.is_null()) return Vector4(0, 0, 0, 1);
	Ref<Vector4D> col3 = basis->get_column(3);
	if (col3.is_null()) return Vector4(0, 0, 0, 1);
	return Vector4(col3->x, col3->y, col3->z, col3->w);
}

float Camera4D::get_slice_plane_distance() const {
	Vector4 normal = get_slice_plane_normal();
	Ref<Transform4D> gt = get_global_transform_4d();
	if (gt.is_null()) return 0.0f;
	Ref<Vector4D> origin = gt->get_origin();
	if (origin.is_null()) return 0.0f;
	return normal.dot(Vector4(origin->x, origin->y, origin->z, origin->w)) + _slice_offset;
}

void Camera4D::make_current() {
	_current = true;
	if (_internal_camera) _internal_camera->make_current();
}

void Camera4D::clear_current(bool p_enable_next) {
	_current = false;
	if (_internal_camera) _internal_camera->clear_current(p_enable_next);
}

void Camera4D::set_fov(real_t p_fov) { _fov = p_fov; _update_camera3d_properties(); }
void Camera4D::set_size(real_t p_size) { _size = p_size; _update_camera3d_properties(); }
void Camera4D::set_near(real_t p_near) { _near = p_near; _update_camera3d_properties(); }
void Camera4D::set_far(real_t p_far) { _far = p_far; _update_camera3d_properties(); }
void Camera4D::set_projection(ProjectionType p_projection) { _projection = p_projection; _update_camera3d_properties(); }
void Camera4D::set_keep_aspect(KeepAspect p_keep) { _keep_aspect = p_keep; }
void Camera4D::set_cull_mask(uint32_t p_mask) { _cull_mask = p_mask; if (_internal_camera) _internal_camera->set_cull_mask(p_mask); }
void Camera4D::set_h_offset(real_t p_offset) { _h_offset = p_offset; if (_internal_camera) _internal_camera->set_h_offset(p_offset); }
void Camera4D::set_v_offset(real_t p_offset) { _v_offset = p_offset; if (_internal_camera) _internal_camera->set_v_offset(p_offset); }
void Camera4D::set_slice_offset(real_t p_offset) { _slice_offset = p_offset; }
void Camera4D::set_environment(const Ref<Environment> &p_env) { _environment = p_env; if (_internal_camera) _internal_camera->set_environment(p_env); }
void Camera4D::set_attributes(const Ref<CameraAttributes> &p_attrs) { _attributes = p_attrs; if (_internal_camera) _internal_camera->set_attributes(p_attrs); }
