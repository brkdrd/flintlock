#include "register_types.h"

// Math types
#include "math/vector4d.h"
#include "math/basis4d.h"
#include "math/transform4d.h"
#include "math/aabb4d.h"
#include "math/plane4d.h"
#include "math/projection4d.h"

// Node hierarchy
#include "nodes/node_4d.h"
#include "nodes/collision_object_4d.h"
#include "nodes/physics_body_4d.h"
#include "nodes/static_body_4d.h"
#include "nodes/animatable_body_4d.h"
#include "nodes/character_body_4d.h"
#include "nodes/rigid_body_4d.h"
#include "nodes/area_4d.h"
#include "nodes/visual/visual_instance_4d.h"
#include "nodes/visual/geometry_instance_4d.h"
#include "nodes/visual/sprite_base_4d.h"
#include "nodes/visual/sprite_4d.h"
#include "nodes/visual/animated_sprite_4d.h"
#include "nodes/visual/mesh_instance_4d.h"
#include "nodes/visual/light_4d.h"
#include "nodes/visual/directional_light_4d.h"
#include "nodes/visual/omni_light_4d.h"
#include "nodes/visual/spot_light_4d.h"
#include "nodes/camera_4d.h"
#include "nodes/joint_4d.h"
#include "nodes/pin_joint_4d.h"
#include "nodes/hinge_joint_4d.h"
#include "nodes/slider_joint_4d.h"
#include "nodes/cone_twist_joint_4d.h"
#include "nodes/generic10dof_joint_4d.h"
#include "nodes/collision_shape_4d.h"
#include "nodes/collision_polygon_4d.h"
#include "nodes/ray_cast_4d.h"

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_godot_4d_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
		// Math types (RefCounted wrappers)
		ClassDB::register_class<Vector4D>();
		ClassDB::register_class<Basis4D>();
		ClassDB::register_class<Transform4D>();
		ClassDB::register_class<AABB4D>();
		ClassDB::register_class<Plane4D>();
		ClassDB::register_class<Projection4D>();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		// Base spatial node
		ClassDB::register_class<Node4D>();

		// Physics hierarchy — abstract bases first
		ClassDB::register_abstract_class<CollisionObject4D>();
		ClassDB::register_abstract_class<PhysicsBody4D>();
		ClassDB::register_class<StaticBody4D>();
		ClassDB::register_class<AnimatableBody4D>();
		ClassDB::register_class<CharacterBody4D>();
		ClassDB::register_class<RigidBody4D>();
		ClassDB::register_class<Area4D>();

		// Visual hierarchy
		ClassDB::register_abstract_class<VisualInstance4D>();
		ClassDB::register_abstract_class<GeometryInstance4D>();
		ClassDB::register_abstract_class<SpriteBase4D>();
		ClassDB::register_class<Sprite4D>();
		ClassDB::register_class<AnimatedSprite4D>();
		ClassDB::register_class<MeshInstance4D>();
		ClassDB::register_abstract_class<Light4D>();
		ClassDB::register_class<DirectionalLight4D>();
		ClassDB::register_class<OmniLight4D>();
		ClassDB::register_class<SpotLight4D>();

		// Camera
		ClassDB::register_class<Camera4D>();

		// Joints
		ClassDB::register_abstract_class<Joint4D>();
		ClassDB::register_class<PinJoint4D>();
		ClassDB::register_class<HingeJoint4D>();
		ClassDB::register_class<SliderJoint4D>();
		ClassDB::register_class<ConeTwistJoint4D>();
		ClassDB::register_class<Generic10DOFJoint4D>();

		// Collision helpers
		ClassDB::register_class<CollisionShape4D>();
		ClassDB::register_class<CollisionPolygon4D>();
		ClassDB::register_class<RayCast4D>();
	}

	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		// Editor plugins (Stage 9)
	}
}

void uninitialize_godot_4d_module(ModuleInitializationLevel p_level) {}

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
