# Flintlock — 4D Physics GDExtension for Godot

## Project Overview

Flintlock is a GDExtension that implements a complete 4D physics engine for Godot 4.3+.
The primary use case is **cosmological simulation in four spatial dimensions**.

The core deliverable is a `PhysicsServer4D` that mirrors the design of Godot's
`PhysicsServer3D`: it manages the full 4D simulation internally and exposes a **3D
cross-section** (hyperplane slice) of the 4D world to Godot's renderer and editor.
From GDScript, 4D objects should be usable with the same patterns as native 3D objects
(`RigidBody4D`, `CollisionShape4D`, `Area4D`, etc.).

## Architecture

### High-Level Data Flow

```
GDScript API  ──►  PhysicsServer4D  ──►  4D World State
                        │                     │
                        │    step / solve      │
                        ▼                     ▼
                   Hyperplane Slicer  ◄──  4D Bodies, Shapes, Spaces
                        │
                        ▼
                   3D Meshes + Transforms  ──►  Godot Renderer
```

### Core Subsystems

| Subsystem | Responsibility |
|---|---|
| **Math4D** | `Basis4D`, `Transform4D`, `AABB4D`, `Rotor4D`, hyperplane representation. Built on top of Godot's existing `Vector4`. |
| **Shapes4D** | Geometric primitives: `HyperSphereShape4D`, `HyperBoxShape4D` (tesseract), `HyperCapsuleShape4D`, `ConvexHull4D`, `HyperEllipsoidShape4D`. |
| **Collision** | Broadphase (4D BVH or sweep-and-prune) and narrowphase (GJK/EPA generalised to 4D). |
| **Dynamics** | Integration, 4D gravity (inverse-cube law), constraints, damping. N-body solver with Barnes-Hut or similar optimisation for large body counts. |
| **PhysicsServer4D** | Singleton server registered with Godot. Owns spaces, bodies, shapes, areas. Steps simulation. |
| **Slicer** | Computes the 3D cross-section of every 4D shape against an arbitrary oriented hyperplane. Outputs meshes and 3D transforms consumable by Godot's renderer. |
| **Scene Nodes** | `RigidBody4D`, `StaticBody4D`, `Area4D`, `CollisionShape4D`, `PhysicsBody4D` — GDScript-facing nodes that mirror Godot's 3D physics nodes. |

### Hyperplane Slicing

The slice is defined by a hyperplane: a point in 4D space plus a unit normal. This is
**not** restricted to axis-aligned W-cuts — the hyperplane can be arbitrarily rotated.
Typically the camera carries a 4D position and the hyperplane normal defaults to the W
axis, but rotation support exists for generality. The slicer intersects each 4D shape
with this hyperplane and produces 3D geometry.

### 4D Gravity Model

Since this targets cosmological simulation, gravity follows the **inverse-cube law**
(the natural generalisation of inverse-square to 4D). For large body counts, an
octree-like spatial partitioning (4D analogue: a 16-tree) or Barnes-Hut approximation
should be used.

## Naming Conventions

Follow Godot's existing patterns:

- **4D scene nodes**: `RigidBody4D`, `StaticBody4D`, `Area4D`, `CollisionShape4D`,
  `PhysicsBody4D`, `CharacterBody4D`
- **4D shapes**: `HyperSphereShape4D`, `HyperBoxShape4D`, `HyperCapsuleShape4D`
- **4D math types**: `Basis4D`, `Transform4D`, `AABB4D`, `Rotor4D`
- **Server**: `PhysicsServer4D`, `PhysicsDirectBodyState4D`, `PhysicsDirectSpaceState4D`

Source files use **snake_case** matching the class name: `rigid_body_4d.h`,
`hyper_sphere_shape_4d.cpp`, `physics_server_4d.h`.

## Source Layout

```
src/
├── math/               # 4D math primitives (Basis4D, Transform4D, AABB4D, Rotor4D)
├── shapes/             # Shape definitions and slicing logic per shape
├── collision/          # Broadphase + narrowphase
├── dynamics/           # Integrator, gravity solver, constraints
├── server/             # PhysicsServer4D implementation
├── slicer/             # Hyperplane slicer, mesh generation
├── nodes/              # GDScript-facing scene nodes (RigidBody4D, etc.)
├── register_types.cpp  # GDExtension entry point
└── register_types.h
```

## Build

Primary build system is **SCons** (matching godot-cpp conventions).

```bash
# Initialise submodule (first time)
git submodule update --init --recursive

# Debug build (current platform)
scons target=template_debug

# Release build
scons target=template_release

# Specific platform/arch
scons target=template_debug platform=linux arch=x86_64
```

CMake is available as an alternative (`CMakeLists.txt`).

### Requirements

- C++17 compiler
- Python 3.4+
- SCons (or CMake 3.13+)
- Godot 4.3+ (for the demo project)

## Code Style

- Follow `.clang-format` (LLVM-based, tabs for indentation per `.editorconfig`)
- `#pragma once` for header guards
- Use Godot's `ClassDB::bind_method` / `GDCLASS` macro patterns for all exposed classes
- Keep headers minimal — prefer forward declarations, implement in `.cpp`

## Key Design Decisions

1. **From-scratch physics** — no external physics library. This avoids overhead and
   allows full control over 4D-specific optimisations.
2. **Godot's `Vector4` as foundation** — avoids reimplementing what Godot already
   provides. Custom types (`Basis4D`, `Transform4D`) extend from there.
3. **Server pattern** — follows Godot's `PhysicsServer3D` architecture so the extension
   feels native. The server is a singleton, scene nodes are thin wrappers.
4. **Slicing over projection** — the 3D output is a true cross-section, not a
   projection. This gives physically meaningful geometry.
5. **Editor tools are deferred** — focus first on the runtime physics and GDScript API.
   Editor gizmos/inspectors will come later.

## Current State

### Completed: Foundation & Core Classes

**Math4D** (Stage 1):
- ✅ `Vector4D`, `Basis4D`, `Transform4D`, `AABB4D` — fully tested and working
- ⚠️ `Rotor4D` — `to_basis()` bug (6/9 tests failing, needs rederivation)
- ⚠️ `Hyperplane4D` — implemented but `get_tangent_basis()` stability unverified (blocker for Camera4D)

**PhysicsServer4D**:
- ✅ Singleton server inheriting from `godot::Object` (proper Godot pattern)
- ✅ Uses `godot::RID` throughout (no custom RID wrapper)
- ✅ Registered as Engine singleton, accessible from GDScript
- ✅ Fully bound with `_bind_methods()` for GDScript integration

**Node Hierarchy** (follows Godot's CollisionObject3D → PhysicsBody3D pattern):
```
Node
└─ Node4D (4D spatial base)
    ├─ CollisionObject4D ❌ NOT YET IMPLEMENTED
    │   ├─ PhysicsBody4D ✅ DONE
    │   │   ├─ StaticBody4D ✅ DONE
    │   │   ├─ CharacterBody4D ✅ DONE
    │   │   └─ RigidBody4D ✅ DONE
    │   └─ Area4D ✅ DONE (should inherit from CollisionObject4D)
    └─ CollisionShape4D ✅ DONE (should be child of CollisionObject4D)
```

**Shape Resources**:
- ✅ `HyperSphereShape4DResource`, `HyperBoxShape4DResource`
- ✅ `HyperCapsuleShape4DResource`, `HyperEllipsoidShape4DResource`
- ✅ `ConvexHull4DResource`

### Known Issues

1. **Missing CollisionObject4D**: Current hierarchy skips this base class. `PhysicsBody4D` and `Area4D` should both inherit from `CollisionObject4D` (see requirements below).
2. **Rotor4D bug**: `to_basis()` produces incorrect matrices (sign/coefficient errors).
3. **No visual nodes**: Missing `VisualInstance4D`, `GeometryInstance4D`, `MeshInstance4D` for rendering.
4. **No Camera4D**: No 4D camera to define hyperplane slicing viewpoint.
5. **Slicer incomplete**: Cannot generate 3D meshes from 4D shapes for rendering.

## Testing

Test framework: **doctest v2.4.11** (fetched via CMake FetchContent).

```bash
# Build and run tests (standalone, no godot-cpp needed)
cd tests
cmake -B build && cmake --build build
./build/flintlock_tests
```

The test compat shim (`tests/support/godot_compat.h`) provides minimal `Vector4`,
`Vector3`, and `real_t` definitions so tests compile without godot-cpp. Source headers
use `__has_include` in `math_defs_4d.h` to conditionally pick the right types.

Test files per stage are listed in `IMPLEMENTATION_PLAN.md`. Currently only Stage 1
math tests are enabled in `tests/CMakeLists.txt`.

## Required Class Hierarchy

To properly mirror Godot's 3D architecture, implement the following hierarchy:

### Physics Hierarchy (mirrors Godot 3D)

```
Node
└─ Node4D (base for all 4D spatial nodes)
    └─ CollisionObject4D (base for all collision-aware nodes)
        ├─ PhysicsBody4D (base for physics-enabled bodies)
        │   ├─ StaticBody4D (immovable, no forces)
        │   ├─ CharacterBody4D (kinematic, script-controlled)
        │   └─ RigidBody4D (dynamic, physics-driven)
        └─ Area4D (trigger volumes, spatial queries)
```

**CollisionObject4D Requirements**:
- Inherit from `Node4D`
- Properties: `collision_layer`, `collision_mask`, `collision_priority`
- Methods: `get_rid()`, shape owner management
- Signals: `body_entered`, `body_exited`, `area_entered`, `area_exited`
- Both `PhysicsBody4D` and `Area4D` must inherit from this

**CollisionShape4D**:
- Child node of `CollisionObject4D` (or subclasses)
- Property: `shape` (Shape4DResource)
- Property: `disabled` (bool)
- On `_ready()`: attach shape to parent's physics body via `PhysicsServer4D`

### Visual Hierarchy (for rendering/slicing)

```
Node
└─ Node4D
    ├─ Camera4D                    (defines hyperplane + drives render loop)
    └─ VisualInstance4D            (base for all visible 4D objects)
        └─ GeometryInstance4D      (adds material, shadow, LOD)
            └─ MeshInstance4D      (holds Shape4DResource, sliced each frame)
```

**VisualInstance4D Requirements**:
- Inherit from `Node4D`
- Properties: `visible` (bool), `layers` (uint32_t render layer mask)
- Owns a `RID instance_rid` (via `RS::instance_create()`) and `RID mesh_rid` (via `RS::mesh_create()`)
- **No `MeshInstance3D` child nodes** — uses RenderingServer RIDs directly, mirroring how Godot's own `VisualInstance3D` works
- `NOTIFICATION_ENTER_TREE`: `RS::instance_set_scenario(instance_rid, get_viewport()->find_world_3d()->get_scenario())`
- `NOTIFICATION_EXIT_TREE`: `RS::instance_set_scenario(instance_rid, RID())`
- Destructor: `RS::free(instance_rid)`, `RS::free(mesh_rid)`
- Internal method called by `Camera4D` each frame: `update_render_data(const Array &surface_arrays, const Transform3D &slice_transform)`
  - Calls `RS::mesh_clear(mesh_rid)`, `RS::mesh_add_surface_from_arrays(...)`, `RS::instance_set_base(instance_rid, mesh_rid)`, `RS::instance_set_transform(instance_rid, slice_transform)`
- Adds self to SceneTree group `"visual_4d"` on enter, removes on exit

**GeometryInstance4D Requirements**:
- Inherit from `VisualInstance4D`
- Property: `material` (`Ref<Material>`) — applied to the slice via `RS::instance_geometry_set_material_override(instance_rid, material->get_rid())`
- Material is set once on change, not every frame

**MeshInstance4D Requirements**:
- Inherit from `GeometryInstance4D`
- Property: `shape` (`Ref<Shape4DResource>`) — reuses existing physics shape resources
- No logic of its own — `Camera4D` reads `shape->get_shape()` and `get_transform_4d()` each frame

**Camera4D Requirements**:

Camera4D is a `Node4D` that owns a `RenderingServer` camera RID and drives the per-frame
slicing loop. It does **not** inherit from `Camera3D` and does **not** create child nodes.
It manages everything through RIDs, mirroring how `Camera3D` works internally.

**Hyperplane derivation** — fully derived from Camera4D's own `Transform4D`:
- `normal = transform_4d.basis.get_column(3).normalized()` — the W-basis column
- `point  = transform_4d.origin` — the 4D position

Moving `position_4d_w` shifts which depth is sliced. Rotating the 4D basis (e.g. via
`Rotor4D`) tilts the hyperplane. No separate `hyperplane_position`/`hyperplane_normal`
properties are needed — the transform is the single source of truth.

**Properties**:

| Property | Type    | Default | Description                          |
|----------|---------|---------|--------------------------------------|
| `fov`    | float   | 75.0    | 3D field of view (degrees)           |
| `near`   | float   | 0.05    | Near clipping plane                  |
| `far`    | float   | 4000.0  | Far clipping plane                   |
| `current`| bool    | false   | Whether this is the active camera    |

**Methods**:
- `make_current()` — calls `RS::viewport_attach_camera(get_viewport()->get_viewport_rid(), camera_rid)`
- `clear_current()` — calls `RS::viewport_attach_camera(..., RID())`
- `is_current() const` — returns `current` flag
- `get_hyperplane() const` → `Hyperplane4D` — computed from `transform_4d`
- `get_camera_rid() const` → `RID` — the RenderingServer camera handle

**Lifecycle**:
- Constructor: `camera_rid = RS::camera_create()`, `RS::camera_set_perspective(camera_rid, fov, near, far)`
- `NOTIFICATION_ENTER_TREE`: if `current`, call `make_current()`; add to group `"cameras_4d"`
- `NOTIFICATION_EXIT_TREE`: `clear_current()`, remove from group
- Destructor: `RS::free(camera_rid)`
- Property setters for `fov`/`near`/`far`: immediately call `RS::camera_set_perspective(camera_rid, ...)`

**Per-frame render loop** (`_process(double delta)`):
1. Compute `Hyperplane4D plane = get_hyperplane()`
2. Compute `Transform3D slice_transform` from tangent basis (see Coordinate Frame below)
3. `RS::camera_set_transform(camera_rid, derive_camera_3d_transform())`
4. For each node in group `"visual_4d"`:
   - Cast to `VisualInstance4D`; skip if not visible
   - `shape  = mesh_instance->get_shape()->get_shape()`
   - `xform  = instance->get_transform_4d()`
   - `result = Slicer4D::slice_shape(shape, xform, plane)`
   - `arrays = Slicer4D::build_surface_arrays(result)` — returns `Array`, not `Ref<ArrayMesh>`
   - `instance->update_render_data(arrays, slice_transform)`

**Camera 3D transform derivation**:
```
// xyz of the camera's 4D position becomes the 3D camera position
Vector3 origin_3d(transform_4d.origin.x, transform_4d.origin.y, transform_4d.origin.z);

// Upper-left 3×3 of the 4D basis becomes the 3D camera orientation
Basis basis_3d = extract_3x3_basis(transform_4d.basis);

RS::camera_set_transform(camera_rid, Transform3D(basis_3d, origin_3d));
```

**Coordinate Frame: Hyperplane → 3D World**:

The slicer outputs vertices in the hyperplane's local tangent frame (t1, t2, t3 from
`Hyperplane4D::get_tangent_basis()`). These map to Godot's 3D world axes via:

```cpp
Vector4 t1, t2, t3;
plane.get_tangent_basis(&t1, &t2, &t3);

// Project each tangent vector's XYZ components to get 3D world axes
Basis basis_3d(Vector3(t1.x, t1.y, t1.z),
               Vector3(t2.x, t2.y, t2.z),
               Vector3(t3.x, t3.y, t3.z));

Transform3D slice_transform(basis_3d, Vector3(0, 0, 0));
```

For the default case (normal = W-axis), the tangent basis is the standard XYZ axes so
`basis_3d` is identity — zero overhead.

**Tangent basis stability (blocker)**:
`Hyperplane4D::get_tangent_basis()` must use Gram-Schmidt orthogonalization with a fixed
priority axis order (choose the 4D world axis *least* parallel to the normal as the seed).
This prevents the basis from flipping between frames when the normal rotates. Must be
verified before Camera4D can produce stable output.

## Slicer Requirements

The **Hyperplane Slicer** must:

1. **Input**:
   - 4D shape (via `Shape4D*` from `Shape4DResource::get_shape()`)
   - Hyperplane definition (from `Camera4D::get_hyperplane()`)
   - Shape's world `Transform4D`

2. **Process**:
   - For each 4D cell (tetrahedron/polytope):
     - Classify vertices (above/below/on hyperplane)
     - Clip edges that cross the hyperplane
     - Triangulate the resulting 3D polygon
   - Generate 3D mesh (vertices, normals)

3. **Output**:
   - `SliceResult` (existing) — vertices and indices in hyperplane-local tangent frame
   - `Array build_surface_arrays(result)` — converts `SliceResult` to a Godot surface
     arrays `Array` (sized `Mesh::ARRAY_MAX`) ready for `RS::mesh_add_surface_from_arrays`.
     Returns a raw `Array`, **not** a `Ref<ArrayMesh>`, to avoid heap allocation per frame.

4. **Integration**:
   - Called by `Camera4D::_process()` for every visible `VisualInstance4D` each frame
   - Must handle:
     - `HyperSphereShape4D` → sphere/ellipse slice
     - `HyperBoxShape4D` → convex polygon slice
     - `ConvexHull4D` → general convex polygon slice

**Slicer API** (in `src/slicer/`):
```cpp
class Slicer4D {
public:
    // Slice a 4D shape — existing, unchanged
    static SliceResult slice_shape(
        const Shape4D *shape,
        const Transform4D &transform,
        const Hyperplane4D &plane
    );

    // NEW: Convert SliceResult to Godot surface arrays for RS::mesh_add_surface_from_arrays
    // Returns Array sized Mesh::ARRAY_MAX. Returns empty Array if result.is_empty().
    static Array build_surface_arrays(const SliceResult &result);
};
```

## Implementation Priority

1. **Fix Rotor4D** — required for correct 4D rotations
2. **Stabilise `Hyperplane4D::get_tangent_basis()`** — must use Gram-Schmidt with fixed
   priority axis order; blocker for Camera4D producing stable output
3. **Add `Slicer4D::build_surface_arrays()`** — converts `SliceResult` to Godot surface
   arrays `Array`; needed by Camera4D render loop
4. **Implement `VisualInstance4D` / `GeometryInstance4D` / `MeshInstance4D`** — RID-based,
   no `MeshInstance3D` children; registers to group `"visual_4d"`
5. **Implement `Camera4D`** — owns `RID camera_rid`, drives per-frame slicing loop,
   calls `RS::viewport_attach_camera` for `make_current()`
6. **Add CollisionObject4D** — fix physics hierarchy (PhysicsBody4D/Area4D inherit from it)
7. **Basic rendering integration test** — one `MeshInstance4D` + one `Camera4D`, confirm
   slice appears in the Godot viewport

Without steps 1–5, the 4D physics will run but produce no visible output.
