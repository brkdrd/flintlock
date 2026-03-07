@tool
## test_4d_scene.gd
## Attach to a plain Node in a scene. On _ready it builds a 4D scene with:
##   - a HyperSphereMesh4D at the 4D origin
##   - an OmniLight4D offset to one side
##   - a Camera4D 5 units back along Z, slicing the W = 0 hyperplane
##   - Two sliders to control sphere W and camera W positions

extends Node

var _sphere: MeshInstance4D
var _camera: Camera4D

func _ready() -> void:
	# ── Scene root ────────────────────────────────────────────────────────────
	var root := Node4D.new()
	root.name = "Scene4D"
	add_child(root)

	# ── Hypersphere ───────────────────────────────────────────────────────────
	_sphere = MeshInstance4D.new()
	_sphere.name = "HyperSphere"

	var mesh := HyperSphereMesh4D.new()
	mesh.radius = 1.0
	mesh.subdivisions = 3
	_sphere.mesh = mesh

	var mat := Material4D.new()
	mat.albedo_color = Color(0.2, 0.55, 1.0)
	mat.roughness = 0.4
	mat.metallic = 0.1
	_sphere.material_override = mat

	root.add_child(_sphere)

	# ── OmniLight4D ───────────────────────────────────────────────────────────
	var light := OmniLight4D.new()
	light.name = "OmniLight"
	light.position_4d = Vector4(3.0, 4.0, 3.0, 0.0)
	light.light_color = Color(1.0, 0.95, 0.85)
	light.light_energy = 2.0
	light.omni_range = 20.0
	light.shadow_enabled = true
	root.add_child(light)

	# ── Camera4D ──────────────────────────────────────────────────────────────
	_camera = Camera4D.new()
	_camera.name = "Camera4D"
	_camera.position_4d = Vector4(0.0, 0.0, 5.0, 0.0)
	_camera.fov = 60.0
	_camera.near = 0.05
	_camera.far = 100.0
	root.add_child(_camera)
	_camera.make_current()

	# ── UI sliders ────────────────────────────────────────────────────────────
	var ui := CanvasLayer.new()
	add_child(ui)

	var vbox := VBoxContainer.new()
	vbox.offset_left = 10
	vbox.offset_top = 10
	vbox.offset_right = 310
	ui.add_child(vbox)

	# Sphere W slider
	var sphere_label := Label.new()
	sphere_label.text = "Sphere W: 0.00"
	vbox.add_child(sphere_label)

	var sphere_slider := HSlider.new()
	sphere_slider.min_value = -3.0
	sphere_slider.max_value = 3.0
	sphere_slider.step = 0.01
	sphere_slider.value = 0.0
	sphere_slider.custom_minimum_size = Vector2(280, 0)
	sphere_slider.value_changed.connect(func(val: float) -> void:
		var pos := _sphere.position_4d
		_sphere.position_4d = Vector4(pos.x, pos.y, pos.z, val)
		sphere_label.text = "Sphere W: %.2f" % val
	)
	vbox.add_child(sphere_slider)

	# Camera W slider
	var camera_label := Label.new()
	camera_label.text = "Camera W: 0.00"
	vbox.add_child(camera_label)

	var camera_slider := HSlider.new()
	camera_slider.min_value = -3.0
	camera_slider.max_value = 3.0
	camera_slider.step = 0.01
	camera_slider.value = 0.0
	camera_slider.custom_minimum_size = Vector2(280, 0)
	camera_slider.value_changed.connect(func(val: float) -> void:
		var pos := _camera.position_4d
		_camera.position_4d = Vector4(pos.x, pos.y, pos.z, val)
		camera_label.text = "Camera W: %.2f" % val
	)
	vbox.add_child(camera_slider)
