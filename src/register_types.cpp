#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

// Include our Godot-facing classes
#include "nodes/rigid_body_4d.h"
#include "nodes/static_body_4d.h"
#include "nodes/collision_shape_4d.h"
#include "resources/hyper_sphere_shape_4d_resource.h"

// Include server for singleton initialization
#include "server/physics_server_4d.h"

using namespace godot;

void initialize_flintlock_module(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Initialize PhysicsServer4D singleton
		PhysicsServer4D::initialize();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Register node classes
		GDREGISTER_CLASS(RigidBody4D);
		GDREGISTER_CLASS(StaticBody4D);
		GDREGISTER_CLASS(CollisionShape4D);

		// Register resource classes
		GDREGISTER_CLASS(HyperSphereShape4DResource);
	}
}

void uninitialize_flintlock_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Cleanup PhysicsServer4D singleton
		PhysicsServer4D::finalize();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Cleanup scene resources (if needed)
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
