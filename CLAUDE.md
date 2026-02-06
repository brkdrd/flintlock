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

The repository is scaffolded from the godot-cpp template. Only the placeholder
`ExampleClass` exists. Physics implementation has not started yet.

## Testing

No test framework is set up yet. When tests are added they should cover:

- Math4D operations (unit tests for Basis4D, Transform4D, rotations, slicing)
- Shape-hyperplane intersection correctness
- GJK/EPA convergence on known 4D shape pairs
- Gravity solver accuracy (Kepler orbits in 4D)
- Server API round-trips from GDScript
