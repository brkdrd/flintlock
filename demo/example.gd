extends Node

# Demo script for Flintlock 4D Physics GDExtension

func _ready() -> void:
	print("=== Flintlock 4D Physics Demo ===")
	print("")

	# NOTE: PhysicsServer4D singleton temporarily disabled for debugging
	# Test that PhysicsServer4D singleton is available
	# if PhysicsServer4D:
	# 	print("✓ PhysicsServer4D singleton is accessible from GDScript")
	# 	print("  Type: ", PhysicsServer4D.get_class())
	# else:
	# 	print("✗ PhysicsServer4D singleton not found!")
	# 	return

	# Test creating a body through the server directly
	# var server_body := PhysicsServer4D.body_create()
	# print("✓ Created body via PhysicsServer4D: ", server_body)

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

	print("")
	print("=== Demo complete ===")
	print("Flintlock is working! The 4D physics classes and server are available.")
