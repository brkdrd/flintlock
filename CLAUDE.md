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
2. **Everything else** (resources, nodes, servers, editor, slicer): `using namespace godot;` at top of header, class in global scope

### Registration (src/register_types.cpp)

All classes must be registered here at the correct initialization level:
- **SERVERS**: Math types (RefCounted wrappers), Resource subclasses (Shape4D, Mesh4D, Material4D, etc.), PhysicsServer4D, Slicer4D singletons
- **SCENE**: All Node subclasses (Node4D, physics bodies, visual nodes, camera, joints, collision helpers)
- **EDITOR**: EditorPlugin4D, inspector/gizmo plugins, Viewport4DPanel

Abstract classes use `register_abstract_class<T>()`. Registration order matters — base classes before derived.

### Core Rendering Pipeline

1. `VisualInstance4D` subclasses hold 4D tetrahedral mesh data (Mesh4D)
2. Each registers with the `Slicer4D` singleton
3. Every frame, `Camera4D` calls `Slicer4D::slice_all()` with the current hyperplane
4. Slicer intersects tetrahedra with the hyperplane → produces 3D triangles → updates RenderingServer mesh RIDs
5. Camera4D owns an internal `Camera3D` child (named `__InternalCamera3D__`) that renders the resulting 3D scene

### Key Singletons

- **PhysicsServer4D**: Stub 4D physics API (registered as engine singleton)
- **Slicer4D**: CPU-based 4D→3D hyperplane slicing (registered as engine singleton)

### Source Layout

- `src/math/` — 4D math types (Vector4D, Basis4D, Transform4D, AABB4D, Plane4D, Projection4D, Bivector4D). These are RefCounted wrappers exposing math to GDScript.
- `src/resources/` — Shape4D hierarchy, Mesh4D hierarchy (ArrayMesh4D, PrimitiveMesh4D with HyperBox/Sphere/Cylinder/Capsule), Material4D, Texture4D, SpriteFrames4D, Environment4D
- `src/nodes/` — Node4D (base spatial), physics body hierarchy, collision helpers, joints, camera
- `src/nodes/visual/` — VisualInstance4D → GeometryInstance4D → MeshInstance4D; SpriteBase4D → Sprite4D/AnimatedSprite4D; Light4D hierarchy
- `src/servers/physics/` — PhysicsServer4D, PhysicsDirectBodyState4D, PhysicsDirectSpaceState4D
- `src/slicer/` — Slicer4D singleton
- `src/editor/` — EditorPlugin4D, inspector plugin, gizmo plugin, Viewport4DPanel
- `thirdparty/eigen/` — Eigen (header-only) for internal matrix math
- `godot-cpp/` — git submodule

### Known godot-cpp Pitfalls

- **RID creation**: No `RID::from_uint64` in godot-cpp. Use `memcpy(rid._native_ptr(), &id, sizeof(uint64_t))`.
- **ObjectID**: Has no `GetTypeInfo` in godot-cpp — use `uint64_t` instead for bound methods.
- **Ref<T> destructors**: `editor_plugin_4d.h` must include full headers for `EditorInspectorPlugin4D` and `EditorGizmoPlugin4D` (not forward decls) because `Ref<T>` destructor needs complete type.
- **Forward declarations after includes**: Never add `class Foo;` forward declarations in a header that already fully defines `Foo` via includes — GCC treats it as incomplete for template arguments, breaking `ClassDB::register_class<Foo>()`.

### 4D-Specific Concepts

- **PlaneAxis enum** (math_4d.h): 6 rotation planes — XY, XZ, XW, YZ, YW, ZW
- **Mesh4D arrays**: Vertices are 4 floats (x,y,z,w), indices are groups of 4 (tetrahedra)
- **Node4D transform**: Stored as `Ref<Transform4D>` (Basis4D 4x4 + Vector4D origin). Position exposed as Godot `Vector4`, basis as `Projection` (4x4 matrix) to avoid inspector warnings.
- **Light4D**: Finds active Camera4D via `get_viewport()->get_camera_3d()->get_parent()`
