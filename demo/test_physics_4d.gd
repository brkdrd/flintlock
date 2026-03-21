## test_physics_4d.gd
## 4D Physics test scene: a sphere falls under gravity onto a tilted box.
## The box is slightly rotated in the ZW plane so its cross-section appears
## as a slope, letting the sphere roll off when collision is implemented.
##
## NOTE: PhysicsServer4D currently has gravity integration but no collision
## detection. The sphere will fall under gravity and pass through the box.
## This scene is a testbed for developing the physics pipeline.

extends Node

var _sphere: RigidBody4D
var _box: StaticBody4D
var _camera: Camera4D
var _physics_space: RID

func _ready() -> void:
	# ── Physics space setup ──────────────────────────────────────────────────
	var ps := PhysicsServer4D
	_physics_space = ps.space_create()
	ps.space_set_active(_physics_space, true)

	# ── Scene root ───────────────────────────────────────────────────────────
	var root := Node4D.new()
	root.name = "Scene4D"
	add_child(root)

	# ── Falling Sphere (RigidBody4D) ─────────────────────────────────────────
	_sphere = RigidBody4D.new()
	_sphere.name = "Sphere"
	_sphere.mass = 1.0
	_sphere.gravity_scale = 1.0
	_sphere.position_4d = Vector4(0.0, 5.0, 0.0, 0.0)
	root.add_child(_sphere)

	# Visual mesh for the sphere
	var sphere_visual := MeshInstance4D.new()
	sphere_visual.name = "SphereVisual"
	var sphere_mesh := HyperSphereMesh4D.new()
	sphere_mesh.radius = 0.5
	sphere_mesh.subdivisions = 3
	sphere_visual.mesh = sphere_mesh
	var sphere_mat := Material4D.new()
	sphere_mat.albedo_color = Color(0.9, 0.3, 0.2)
	sphere_mat.roughness = 0.3
	sphere_mat.metallic = 0.1
	sphere_visual.material_override = sphere_mat
	_sphere.add_child(sphere_visual)

	# Collision shape for the sphere
	var sphere_col := CollisionShape4D.new()
	sphere_col.name = "SphereCollision"
	var sphere_shape := SphereShape4D.new()
	sphere_shape.radius = 0.5
	sphere_col.shape = sphere_shape
	_sphere.add_child(sphere_col)

	# Register sphere body with physics space
	ps.body_set_space(_sphere.get_rid(), _physics_space)

	# ── Floor Box (StaticBody4D) ─────────────────────────────────────────────
	_box = StaticBody4D.new()
	_box.name = "FloorBox"
	_box.position_4d = Vector4(0.0, -1.0, 0.0, 0.0)
	# Slight rotation in the ZW plane so the cross-section appears tilted
	_box.rotate(5, 0.15)  # PlaneAxis 5 = ZW, 0.15 radians ≈ 8.6°
	root.add_child(_box)

	# Visual mesh for the box
	var box_visual := MeshInstance4D.new()
	box_visual.name = "BoxVisual"
	var box_mesh := HyperBoxMesh4D.new()
	box_mesh.size = Vector4(6.0, 0.5, 6.0, 2.0)
	box_visual.mesh = box_mesh
	var box_mat := Material4D.new()
	box_mat.albedo_color = Color(0.3, 0.7, 0.4)
	box_mat.roughness = 0.8
	box_mat.metallic = 0.0
	box_visual.material_override = box_mat
	_box.add_child(box_visual)

	# Collision shape for the box
	var box_col := CollisionShape4D.new()
	box_col.name = "BoxCollision"
	var box_shape := BoxShape4D.new()
	box_shape.size = Vector4(6.0, 0.5, 6.0, 2.0)
	box_col.shape = box_shape
	_box.add_child(box_col)

	# Register box body with physics space
	ps.body_set_space(_box.get_rid(), _physics_space)

	# ── Lighting ─────────────────────────────────────────────────────────────
	var light := OmniLight4D.new()
	light.name = "OmniLight"
	light.position_4d = Vector4(3.0, 6.0, 3.0, 0.0)
	light.light_color = Color(1.0, 0.95, 0.85)
	light.light_energy = 2.5
	light.omni_range = 25.0
	light.shadow_enabled = true
	root.add_child(light)

	# Second light for fill
	var fill_light := OmniLight4D.new()
	fill_light.name = "FillLight"
	fill_light.position_4d = Vector4(-2.0, 3.0, -1.0, 0.0)
	fill_light.light_color = Color(0.7, 0.8, 1.0)
	fill_light.light_energy = 1.0
	fill_light.omni_range = 20.0
	root.add_child(fill_light)

	# ── Camera ───────────────────────────────────────────────────────────────
	_camera = Camera4D.new()
	_camera.name = "Camera4D"
	_camera.position_4d = Vector4(4.0, 4.0, 8.0, 0.0)
	_camera.fov = 60.0
	_camera.near = 0.05
	_camera.far = 100.0
	root.add_child(_camera)
	_camera.make_current()

	# ── UI ───────────────────────────────────────────────────────────────────
	var ui := CanvasLayer.new()
	add_child(ui)

	var vbox := VBoxContainer.new()
	vbox.offset_left = 10
	vbox.offset_top = 10
	vbox.offset_right = 400
	ui.add_child(vbox)

	var info_label := Label.new()
	info_label.name = "InfoLabel"
	info_label.text = "4D Physics Test — Sphere falls under gravity"
	vbox.add_child(info_label)

	var vel_label := Label.new()
	vel_label.name = "VelLabel"
	vel_label.text = "Velocity: (0, 0, 0, 0)"
	vbox.add_child(vel_label)

	var pos_label := Label.new()
	pos_label.name = "PosLabel"
	pos_label.text = "Position: (0, 5, 0, 0)"
	vbox.add_child(pos_label)

	# Reset button
	var reset_btn := Button.new()
	reset_btn.text = "Reset Sphere"
	reset_btn.pressed.connect(_reset_sphere)
	vbox.add_child(reset_btn)

func _physics_process(delta: float) -> void:
	# Step the 4D physics server
	PhysicsServer4D.step(delta)

	# Update UI labels
	var vel := _sphere.linear_velocity
	var pos := _sphere.position_4d
	var vel_label := get_node_or_null("CanvasLayer/VBoxContainer/VelLabel") as Label
	var pos_label := get_node_or_null("CanvasLayer/VBoxContainer/PosLabel") as Label
	if vel_label:
		vel_label.text = "Velocity: (%.2f, %.2f, %.2f, %.2f)" % [vel.x, vel.y, vel.z, vel.w]
	if pos_label:
		pos_label.text = "Position: (%.2f, %.2f, %.2f, %.2f)" % [pos.x, pos.y, pos.z, pos.w]

func _reset_sphere() -> void:
	_sphere.position_4d = Vector4(0.0, 5.0, 0.0, 0.0)
	_sphere.linear_velocity = Vector4.ZERO
