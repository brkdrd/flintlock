#include "register_types.h"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>

// Include our Godot-facing classes
#include "nodes/node_4d.h"
#include "nodes/rigid_body_4d.h"
#include "nodes/static_body_4d.h"
#include "nodes/collision_shape_4d.h"
#include "resources/hyper_sphere_shape_4d_resource.h"
#include "resources/hyper_box_shape_4d_resource.h"
#include "resources/hyper_capsule_shape_4d_resource.h"
#include "resources/hyper_ellipsoid_shape_4d_resource.h"
#include "resources/convex_hull_4d_resource.h"

// Include server for singleton initialization
#include "server/physics_server_4d.h"
#include "server/physics_server_4d_godot.h"

using namespace godot;

// Keep a reference to the singleton instance
static PhysicsServer4DGodot *physics_server_4d_singleton = nullptr;

void initialize_flintlock_module(ModuleInitializationLevel p_level)
{
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Initialize internal PhysicsServer4D C++ singleton
		PhysicsServer4D::initialize();

		// Register the Godot-facing wrapper class
		GDREGISTER_CLASS(PhysicsServer4DGodot);

		// TODO: Temporarily disabled singleton registration for debugging
		// Create and register the PhysicsServer4D singleton for GDScript
		// Note: Using memnew is correct for Godot objects
		// physics_server_4d_singleton = memnew(PhysicsServer4DGodot);
		// if (physics_server_4d_singleton && Engine::get_singleton()) {
		// 	Engine::get_singleton()->register_singleton("PhysicsServer4D", physics_server_4d_singleton);
		// }
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Register base node class
		GDREGISTER_CLASS(Node4D);

		// Register node classes
		GDREGISTER_CLASS(RigidBody4D);
		GDREGISTER_CLASS(StaticBody4D);
		GDREGISTER_CLASS(CollisionShape4D);

		// Register resource classes (shape resources)
		GDREGISTER_CLASS(HyperSphereShape4DResource);
		GDREGISTER_CLASS(HyperBoxShape4DResource);
		GDREGISTER_CLASS(HyperCapsuleShape4DResource);
		GDREGISTER_CLASS(HyperEllipsoidShape4DResource);
		GDREGISTER_CLASS(ConvexHull4DResource);
	}
}

void uninitialize_flintlock_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Unregister and cleanup the Godot singleton
		if (physics_server_4d_singleton) {
			Engine::get_singleton()->unregister_singleton("PhysicsServer4D");
			memdelete(physics_server_4d_singleton);
			physics_server_4d_singleton = nullptr;
		}

		// Cleanup internal PhysicsServer4D singleton
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
