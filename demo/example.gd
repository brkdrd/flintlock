extends Node

# Rendering test: 4D sphere sliced by Camera4D's hyperplane.
#
# Setup:
#   - MeshInstance4D at 4D origin, HyperSphereShape4DResource radius 1.0
#   - Camera4D at (0, 0, 5, 0)
#       * Hyperplane normal = W-axis (identity 4D basis W-column)
#       * Hyperplane passes through W = 0  →  slices the sphere at its centre
#       * Cross-section = 3D sphere of radius 1.0
#       * 3D camera position = (0, 0, 5)  →  looks in -Z toward origin

func _ready() -> void:
	print("=== Flintlock 4D Rendering Test ===")

	# --- Camera4D -------------------------------------------------------
	# Must be added to the tree before make_current() can attach to the viewport.
	var camera := Camera4D.new()
	camera.name = "Camera4D"
	camera.fov  = 75.0
	camera.near = 0.05
	camera.far  = 1000.0
	add_child(camera)

	# Position: XYZ = (0,0,5) is the 3D camera location; W = 0 keeps the
	# hyperplane at W = 0 so it intersects the sphere at its equator.
	camera.set_position_4d(Vector4(0.0, 0.0, 5.0, 0.0))

	# Activate this camera for the viewport.
	camera.make_current()

	print("✓ Camera4D added at: ", camera.get_position_4d())
	print("  is_current: ", camera.is_current())

	# --- Lighting -------------------------------------------------------
	# A directional light so the slice mesh is shaded properly.
	var light := DirectionalLight3D.new()
	light.name = "Sun"
	light.rotation_degrees = Vector3(-45.0, 45.0, 0.0)
	add_child(light)
	print("✓ DirectionalLight3D added")

	# --- MeshInstance4D (sphere) ----------------------------------------
	var mesh_inst := MeshInstance4D.new()
	mesh_inst.name = "Sphere4D"
	add_child(mesh_inst)

	# 4D origin: the hyperplane at W = 0 cuts through the sphere's centre,
	# giving a 3D cross-section sphere of the same radius.
	mesh_inst.set_position_4d(Vector4(0.0, 0.0, 0.0, 0.0))

	var sphere_shape := HyperSphereShape4DResource.new()
	sphere_shape.radius = 1.0
	mesh_inst.shape = sphere_shape

	# Blue material so the slice is clearly visible.
	var mat := StandardMaterial3D.new()
	mat.albedo_color = Color(0.2, 0.5, 1.0)
	mesh_inst.material = mat

	print("✓ MeshInstance4D (sphere r=1) at: ", mesh_inst.get_position_4d())
	print("")
	print("Rendering loop active.")
	print("Expected: blue 3D sphere visible in the viewport")
	print("  (W=0 slice of a 4D hypersphere at origin with radius 1)")
