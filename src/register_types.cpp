#include "register_types.h"

// Math types
#include "math/vector4d.h"
#include "math/basis4d.h"
#include "math/transform4d.h"
#include "math/aabb4d.h"
#include "math/plane4d.h"
#include "math/projection4d.h"

// Nodes (stubs - Stage 2)
// #include "nodes/node_4d.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_godot_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Math types and resources
		ClassDB::register_class<Vector4D>();
		ClassDB::register_class<Basis4D>();
		ClassDB::register_class<Transform4D>();
		ClassDB::register_class<AABB4D>();
		ClassDB::register_class<Plane4D>();
		ClassDB::register_class<Projection4D>();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Node classes (Stage 2+)
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Editor plugins (Stage 9)
	}
}

void uninitialize_godot_4d_module(ModuleInitializationLevel p_level) {
	// Nothing to clean up for pure RefCounted types
}

extern "C" {
GDExtensionBool GDE_EXPORT godot_4d_init(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		const GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_godot_4d_module);
	init_obj.register_terminator(uninitialize_godot_4d_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);

	return init_obj.init();
}
}
