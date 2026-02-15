extends Node

# Demo script for Flintlock 4D Physics GDExtension

func _ready() -> void:
	print("=== Flintlock 4D Physics Demo ===")

	# Test that PhysicsServer4D singleton is available
	if PhysicsServer4D:
		print("✓ PhysicsServer4D singleton initialized")
	else:
		print("✗ PhysicsServer4D not found")
		return

	# Create a simple 4D rigid body
	var body := RigidBody4D.new()
	add_child(body)
	print("✓ Created RigidBody4D: ", body)

	# Create a collision shape
	var collision_shape := CollisionShape4D.new()
	body.add_child(collision_shape)
	print("✓ Created CollisionShape4D: ", collision_shape)

	# Create a hypersphere shape resource
	var shape := HyperSphereShape4DResource.new()
	shape.radius = 1.0
	collision_shape.shape = shape
	print("✓ Created HyperSphereShape4DResource with radius: ", shape.radius)

	print("=== Demo complete ===")
	print("Flintlock is working! The 4D physics classes are available.")
