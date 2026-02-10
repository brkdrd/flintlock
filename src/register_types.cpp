#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

// TODO: Will need Godot-native implementations of these classes
// For now, just register the basic structure

using namespace godot;

void initialize_flintlock_module(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Register PhysicsServer4D singleton
		// TODO: Create Godot-native PhysicsServer4D class
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Register scene node classes
		// TODO: Register RigidBody4D, StaticBody4D, Area4D, CollisionShape4D, etc.
	}
}

void uninitialize_flintlock_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Cleanup server singletons
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Cleanup scene resources
	}
}

extern "C"
{
	// Initialization - entry point must match .gdextension file
	GDExtensionBool GDE_EXPORT flintlock_library_init(
		GDExtensionInterfaceGetProcAddress p_get_proc_address,
		GDExtensionClassLibraryPtr p_library,
		GDExtensionInitialization *r_initialization)
	{
		GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
		init_obj.register_initializer(initialize_flintlock_module);
		init_obj.register_terminator(uninitialize_flintlock_module);
		init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);

		return init_obj.init();
	}
}
