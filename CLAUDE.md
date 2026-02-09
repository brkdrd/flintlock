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

Stage 1 (Math4D) is **in progress**. The following classes are implemented and passing tests:

| Class | Header | Status | Tests |
|---|---|---|---|
| `Vector4D` | `src/math/vector4d.h/.cpp` | **Done** | 32 tests, 104 assertions passing |
| `Basis4D` | `src/math/basis4d.h/.cpp` | **Done** | 7 tests passing (identity, rotation, multiplication, inverse, determinant) |
| `Transform4D` | `src/math/transform4d.h/.cpp` | **Done** | 7 tests passing (identity, translation, composition, inverse) |
| `AABB4D` | `src/math/aabb4d.h/.cpp` | **Done** | 9 tests passing (containment, intersection, merge, grow, volume) |
| `Rotor4D` | `src/math/rotor4d.h/.cpp` | **WIP** | `to_basis()` conversion produces incorrect matrix; 6 of 9 tests failing. The geometric product, conjugate, norm, slerp, and from_plane_angle are implemented but the sandwich-product rotation (via `to_basis()`) has sign/coefficient errors that need debugging. |
| `Hyperplane4D` | `src/math/hyperplane4d.h/.cpp` | **Not started** | — |
| `math_defs_4d.h` | `src/math/math_defs_4d.h` | **Done** | Provides conditional includes for godot-cpp vs test compat shim |

### Known Issues

- **Rotor4D `to_basis()` bug**: The matrix produced by `to_basis()` for an XY-plane
  90-degree rotor (s=0.707, e12=-0.707) gives `[[1,-1,0,0],[1,0,0,0],...]` instead
  of the expected `[[0,-1,0,0],[1,0,0,0],...]`. The diagonal elements are wrong,
  suggesting the squared-term formula in `to_basis()` needs rederivation. The
  `rotate()` method delegates to `to_basis()`, so all rotation tests fail.

### What's Left for Stage 1

1. Fix `Rotor4D::to_basis()` — rederive the 4x4 matrix from the sandwich product
2. Implement `Hyperplane4D` (no pre-written test in `tests/math/`, but tested in `tests/slicer/test_hyperplane.cpp`)
3. All earlier-stage tests remain green (55 of 64 total tests pass; 9 are Rotor4D)

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
