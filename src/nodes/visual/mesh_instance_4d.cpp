#include "mesh_instance_4d.h"

void MeshInstance4D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_mesh"), &MeshInstance4D::get_mesh);
	ClassDB::bind_method(D_METHOD("set_mesh", "mesh"), &MeshInstance4D::set_mesh);
	ClassDB::bind_method(D_METHOD("get_surface_material", "surface"), &MeshInstance4D::get_surface_material);
	ClassDB::bind_method(D_METHOD("set_surface_material", "surface", "material"), &MeshInstance4D::set_surface_material);
	ClassDB::bind_method(D_METHOD("get_surface_material_count"), &MeshInstance4D::get_surface_material_count);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "mesh", PROPERTY_HINT_RESOURCE_TYPE, "Mesh4D"), "set_mesh", "get_mesh");

	ClassDB::bind_method(D_METHOD("_on_mesh_changed"), &MeshInstance4D::_on_mesh_changed);
}

void MeshInstance4D::_notification(int p_what) {
	GeometryInstance4D::_notification(p_what);

	if (p_what == NOTIFICATION_ENTER_TREE) {
		apply_material_params();
	}
}

void MeshInstance4D::set_mesh(const Ref<Mesh4D> &p_mesh) {
	if (_mesh == p_mesh) return;
	if (_mesh.is_valid() && _mesh->is_connected("changed", Callable(this, "_on_mesh_changed"))) {
		_mesh->disconnect("changed", Callable(this, "_on_mesh_changed"));
	}
	_mesh = p_mesh;
	if (_mesh.is_valid()) {
		_mesh->connect("changed", Callable(this, "_on_mesh_changed"));
		_surface_materials.resize(_mesh->get_surface_count());
	}
	upload_mesh();
}

void MeshInstance4D::_on_mesh_changed() {
	upload_mesh();
}

Ref<Material4D> MeshInstance4D::get_surface_material(int p_surface) const {
	if (p_surface < 0 || p_surface >= _surface_materials.size()) return Ref<Material4D>();
	return _surface_materials[p_surface];
}

void MeshInstance4D::set_surface_material(int p_surface, const Ref<Material4D> &p_material) {
	if (p_surface < 0) return;
	if (p_surface >= _surface_materials.size()) _surface_materials.resize(p_surface + 1);
	_surface_materials[p_surface] = p_material;
	apply_material_params();
}

int MeshInstance4D::get_surface_material_count() const {
	return _surface_materials.size();
}
