# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What This Is

A Godot 4.3+ GDExtension (C++ via godot-cpp) that adds native 4D spatial dimension support: 4D math types, nodes, physics server, hyperplane slicing for rendering, and editor integration. The design generalizes Godot's existing 3D systems to 4 dimensions (X, Y, Z, W).

## Build

```bash
# First time setup
git submodule update --init --recursive

# Debug build (most common during development)
scons target=template_debug

# Release build
scons target=template_release
```

Output goes to `demo/addons/godot_4d/bin/`. The `demo/` directory is the Godot test project — open it with Godot 4.3+ to test.

There are no tests or linters configured.

## Architecture

### Namespace Convention (Mixed — Two Patterns)

The codebase uses two different namespace patterns. Follow whichever is already used in the file you're editing:

1. **Math types + Node4D**: Wrap class in `namespace godot { ... }`
2. **Everything else** (resources, nodes, servers, editor): `using namespace godot;` at top of header, class in global scope

### Registration (src/register_types.cpp)

All classes must be registered here at the correct initialization level:
- **SERVERS**: Math types (RefCounted wrappers), Resource subclasses (Shape4D, Mesh4D, Material4D, etc.), PhysicsServer4D, VisualServer4D singletons
- **SCENE**: All Node subclasses (Node4D, physics bodies, visual nodes, camera, joints, collision helpers)
- **EDITOR**: EditorPlugin4D, inspector/gizmo plugins, Viewport4DPanel

Abstract classes use `register_abstract_class<T>()`. Registration order matters — base classes before derived.

### Core Rendering Pipeline

```
4D Nodes → VisualServer4D (RID API) → RenderingServer → GPU
```

1. `VisualInstance4D` subclasses hold 4D tetrahedral mesh data (Mesh4D)
2. On ENTER_TREE, nodes create VS4D RIDs (instance + mesh) and upload mesh data
3. `VisualServer4D` packs tetrahedra into GPU vertex format and manages RS resources
4. Every frame, `Camera4D` calls `VisualServer4D::process_frame()` with the current hyperplane
5. VS4D updates shader uniforms (camera basis, slice plane), per-instance model transforms, and projects all 4D lights to 3D
6. The vertex shader (GPU) performs per-vertex slicing using a LUT-based algorithm
7. VS4D manages an internal `Camera3D` (attached to Camera4D node) for final 3D rendering

### Key Singletons

- **PhysicsServer4D**: Stub 4D physics API (registered as engine singleton)
- **VisualServer4D**: Centralized 4D rendering server — manages all RS resources, GPU slicing shader, light projection, camera (registered as engine singleton)

### Source Layout

- `src/math/` — 4D math types (Vector4D, Basis4D, Transform4D, AABB4D, Plane4D, Projection4D, Bivector4D). These are RefCounted wrappers exposing math to GDScript.
- `src/resources/` — Shape4D hierarchy, Mesh4D hierarchy (ArrayMesh4D, PrimitiveMesh4D with HyperBox/Sphere/Cylinder/Capsule), Material4D, Texture4D, SpriteFrames4D, Environment4D
- `src/nodes/` — Node4D (base spatial), physics body hierarchy, collision helpers, joints, camera
- `src/nodes/visual/` — VisualInstance4D → GeometryInstance4D → MeshInstance4D; SpriteBase4D → Sprite4D/AnimatedSprite4D; Light4D hierarchy
- `src/servers/physics/` — PhysicsServer4D, PhysicsDirectBodyState4D, PhysicsDirectSpaceState4D
- `src/servers/visual/` — VisualServer4D singleton (shader, LUT, mesh upload, light projection, camera)
- `src/editor/` — EditorPlugin4D, inspector plugin, gizmo plugin, Viewport4DPanel
- `thirdparty/eigen/` — Eigen (header-only) for internal matrix math
- `godot-cpp/` — git submodule

### Known godot-cpp Pitfalls

- **RID creation**: No `RID::from_uint64` in godot-cpp. Use `memcpy(rid._native_ptr(), &id, sizeof(uint64_t))`.
- **ObjectID**: Has no `GetTypeInfo` in godot-cpp — use `uint64_t` instead for bound methods.
- **Ref<T> destructors**: `editor_plugin_4d.h` must include full headers for `EditorInspectorPlugin4D` and `EditorGizmoPlugin4D` (not forward decls) because `Ref<T>` destructor needs complete type.
- **Forward declarations after includes**: Never add `class Foo;` forward declarations in a header that already fully defines `Foo` via includes — GCC treats it as incomplete for template arguments, breaking `ClassDB::register_class<Foo>()`. This is especially dangerous in `visual_server_4d.h` — do NOT forward-declare `Vector4D`, `Basis4D`, or `Transform4D` there.

### 4D-Specific Concepts

- **PlaneAxis enum** (math_4d.h): 6 rotation planes — XY, XZ, XW, YZ, YW, ZW
- **Mesh4D arrays**: Vertices are 4 floats (x,y,z,w), indices are groups of 4 (tetrahedra)
- **Node4D transform**: Stored as `Ref<Transform4D>` (Basis4D 4x4 + Vector4D origin). Position exposed as Godot `Vector4`, basis as `Projection` (4x4 matrix) to avoid inspector warnings.
- **Light4D**: No longer creates internal Light3D child nodes. Instead, creates VS4D light RIDs. VS4D creates RS lights directly and projects 4D→3D each frame during `process_frame()`.
- **Camera4D**: No longer manages internal Camera3D directly. VS4D owns the Camera3D lifecycle via `camera_attach_to_node()`/`camera_detach()`.
