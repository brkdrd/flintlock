## test_4d_scene.gd
## Attach to a plain Node in a scene. On _ready it builds a 4D scene with:
##   - a HyperSphereMesh4D at the 4D origin
##   - an OmniLight4D offset to one side
##   - a Camera4D 5 units back along Z, slicing the W = 0 hyperplane

extends Node


func _ready() -> void:
	# ── Scene root ────────────────────────────────────────────────────────────
	var root := Node4D.new()
	root.name = "Scene4D"
	add_child(root)

	# ── Hypersphere ───────────────────────────────────────────────────────────
	var sphere := MeshInstance4D.new()
	sphere.name = "HyperSphere"

	var mesh := HyperSphereMesh4D.new()
	mesh.radius = 1.0
	mesh.subdivisions = 3
	sphere.mesh = mesh

	var mat := Material4D.new()
	mat.albedo_color = Color(0.2, 0.55, 1.0)
	mat.roughness = 0.4
	mat.metallic = 0.1
	sphere.material_override = mat

	root.add_child(sphere)

	# ── OmniLight4D ───────────────────────────────────────────────────────────
	# Positioned at (3, 4, 3, 0) so it is on the same W = 0 slice as the sphere.
	var light := OmniLight4D.new()
	light.name = "OmniLight"
	light.position_4d = Vector4(3.0, 4.0, 3.0, 0.0)
	light.light_color = Color(1.0, 0.95, 0.85)
	light.light_energy = 2.0
	light.omni_range = 20.0
	light.shadow_enabled = true
	root.add_child(light)

	# ── Camera4D ──────────────────────────────────────────────────────────────
	# The camera uses identity basis, so:
	#   - Slice normal = W column = (0, 0, 0, 1)  →  cuts the W = 0 hyperplane
	#   - Columns 0-2 = standard X/Y/Z axes inside the slice
	# Being 5 units back along Z means the sphere appears 5 units in front of
	# the internal Camera3D (which stays at identity and looks toward -Z).
	var camera := Camera4D.new()
	camera.name = "Camera4D"
	camera.position_4d = Vector4(0.0, 0.0, 5.0, 0.0)
	camera.fov = 60.0
	camera.near = 0.05
	camera.far = 100.0
	root.add_child(camera)

	# make_current() must be called after add_child so the internal Camera3D exists.
	camera.make_current()
