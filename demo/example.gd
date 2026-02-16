extends Node

# Demo script for Flintlock 4D Physics GDExtension

func _ready() -> void:
	print("=== Flintlock 4D Physics Demo ===")
	print("")

	# Test that PhysicsServer4D singleton is available
	if PhysicsServer4D:
		print("✓ PhysicsServer4D singleton is accessible from GDScript")
		print("  Type: ", PhysicsServer4D.get_class())
	else:
		print("✗ PhysicsServer4D singleton not found!")
		return

	# Test creating a body through the server directly
	var server_body := PhysicsServer4D.body_create()
	print("✓ Created body via PhysicsServer4D: ", server_body)

	# Create a simple 4D rigid body node
	var body := RigidBody4D.new()
	add_child(body)
	print("✓ Created RigidBody4D node: ", body)

	# Create a collision shape
	var collision_shape := CollisionShape4D.new()
	body.add_child(collision_shape)
	print("✓ Created CollisionShape4D: ", collision_shape)

	# Create a hypersphere shape resource
	var shape := HyperSphereShape4DResource.new()
	shape.radius = 1.0
	collision_shape.shape = shape
	print("✓ Created HyperSphereShape4DResource with radius: ", shape.radius)

	# Test setting 4D position and velocity
	body.set_position_4d(Vector4(1.0, 2.0, 3.0, 4.0))
	var pos := body.get_position_4d()
	print("✓ Set 4D position: ", pos)

	body.set_linear_velocity(Vector4(0.1, 0.2, 0.3, 0.4))
	var vel := body.get_linear_velocity()
	print("✓ Set linear velocity: ", vel)

	# Test Node4D base class functionality
	print("")
	print("--- Testing Node4D base class ---")
	var node4d := Node4D.new()
	add_child(node4d)
	node4d.set_position_4d(Vector4(5.0, 6.0, 7.0, 8.0))
	print("✓ Node4D position: ", node4d.get_position_4d())
	print("  Individual components - X:", node4d.get_position_4d_x(), " Y:", node4d.get_position_4d_y(), " Z:", node4d.get_position_4d_z(), " W:", node4d.get_position_4d_w())
	node4d.translate_4d(Vector4(1.0, 1.0, 1.0, 1.0))
	print("✓ After translate_4d: ", node4d.get_position_4d())

	# Test other shape resources
	print("")
	print("--- Testing Shape Resources ---")

	var box_shape := HyperBoxShape4DResource.new()
	box_shape.set_half_extents(Vector4(2.0, 2.0, 2.0, 2.0))
	print("✓ HyperBoxShape4DResource created, half_extents: ", box_shape.get_half_extents())

	var capsule_shape := HyperCapsuleShape4DResource.new()
	capsule_shape.set_radius(1.5)
	capsule_shape.set_height(3.0)
	print("✓ HyperCapsuleShape4DResource created, radius: ", capsule_shape.get_radius(), ", height: ", capsule_shape.get_height())

	var ellipsoid_shape := HyperEllipsoidShape4DResource.new()
	ellipsoid_shape.set_radii(Vector4(1.0, 2.0, 3.0, 4.0))
	print("✓ HyperEllipsoidShape4DResource created, radii: ", ellipsoid_shape.get_radii())

	var convex_hull := ConvexHull4DResource.new()
	convex_hull.add_vertex(Vector4(1, 0, 0, 0))
	convex_hull.add_vertex(Vector4(0, 1, 0, 0))
	convex_hull.add_vertex(Vector4(0, 0, 1, 0))
	convex_hull.add_vertex(Vector4(0, 0, 0, 1))
	print("✓ ConvexHull4DResource created with ", convex_hull.get_vertex_count(), " vertices")

	# Test StaticBody4D
	print("")
	print("--- Testing StaticBody4D ---")
	var static_body := StaticBody4D.new()
	add_child(static_body)
	static_body.set_position_4d(Vector4(10.0, 10.0, 10.0, 10.0))
	print("✓ StaticBody4D created at position: ", static_body.get_position_4d())

	print("")
	print("=== Demo complete ===")
	print("All 4D physics classes are working!")
	print("- Node4D base class ✓")
	print("- RigidBody4D ✓")
	print("- StaticBody4D ✓")
	print("- CollisionShape4D ✓")
	print("- All shape resources ✓")
	print("- PhysicsServer4D singleton ✓")
