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
- ❌ `Hyperplane4D` — not yet implemented

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
    └─ VisualInstance4D (base for all visible 4D nodes)
        ├─ GeometryInstance4D (geometry-based rendering)
        │   └─ MeshInstance4D (renders 4D meshes, sliced to 3D)
        └─ Camera4D (defines hyperplane slice viewpoint)
```

**VisualInstance4D Requirements**:
- Inherit from `Node4D`
- Properties: `visible`, `layers` (render layer mask)
- Integrates with slicer to generate 3D cross-sections

**Camera4D Requirements**:
- Defines the **hyperplane slice** for rendering
- Properties:
  - `hyperplane_position` (Vector4) — point on hyperplane
  - `hyperplane_normal` (Vector4) — normal vector (default: W-axis)
  - `fov` (float) — 3D field of view after slicing
  - `near`, `far` (float) — clipping planes
  - `current` (bool) — active camera
- Methods:
  - `make_current()` — activate this camera
  - `get_hyperplane()` → Hyperplane4D

**MeshInstance4D Requirements**:
- Property: `mesh_4d` (Mesh4DResource) — 4D mesh data
- On `_process()`: request 3D slice from slicer, create 3D `MeshInstance3D` child
- Slicer generates 3D mesh by intersecting 4D polytope with camera's hyperplane

## Slicer Requirements

The **Hyperplane Slicer** must:

1. **Input**:
   - 4D shape/mesh (vertices, faces, cells)
   - Hyperplane definition (point + normal from `Camera4D`)

2. **Process**:
   - For each 4D cell (tetrahedron/polytope):
     - Classify vertices (above/below/on hyperplane)
     - Clip edges that cross the hyperplane
     - Triangulate the resulting 3D polygon
   - Generate 3D mesh (vertices, normals, UVs)

3. **Output**:
   - 3D `ArrayMesh` suitable for `MeshInstance3D`
   - Transform to position slice correctly in 3D space

4. **Integration**:
   - Called automatically for all `VisualInstance4D` nodes within camera frustum
   - Cached per frame (only regenerate when 4D transforms change)
   - Must handle:
     - `HyperSphereShape4D` → sphere/ellipse slice
     - `HyperBoxShape4D` → convex polygon slice
     - `ConvexHull4D` → general convex polygon slice
     - Custom 4D meshes → arbitrary 3D cross-sections

**Slicer API** (in `src/slicer/`):
```cpp
class Slicer4D {
public:
    // Slice a 4D shape against a hyperplane
    static Ref<ArrayMesh> slice_shape(
        const Shape4D* shape,
        const Hyperplane4D& plane,
        const Transform4D& transform
    );

    // Slice a 4D mesh
    static Ref<ArrayMesh> slice_mesh(
        const Mesh4DResource* mesh,
        const Hyperplane4D& plane,
        const Transform4D& transform
    );
};
```

## Implementation Priority

1. **Fix Rotor4D** — required for correct 4D rotations
2. **Implement Hyperplane4D** — needed by slicer and Camera4D
3. **Add CollisionObject4D** — fix hierarchy (PhysicsBody4D/Area4D inherit from it)
4. **Implement Camera4D** — defines viewing hyperplane
5. **Implement VisualInstance4D/MeshInstance4D** — rendering foundation
6. **Implement Slicer4D** — generate 3D meshes from 4D geometry
7. **Basic 4D → 3D rendering loop** — integrate slicer with scene tree

Without these, the 4D physics will run but produce no visible output.
