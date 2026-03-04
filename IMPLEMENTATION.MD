# CLAUDE.MD - Godot 4D GDExtension Implementation Guide

## Project Overview

You are building a GDExtension for Godot 4.3+ that adds native 4D (four spatial dimensions: X, Y, Z, W) node types, a 4D physics server, 4D visual rendering via hyperplane slicing, and editor integration. The approach is NOT to invent new paradigms - you are taking what already works in Godot's 3D systems and the Jolt physics integration, and generalizing the math and data structures from 3 dimensions to 4.

**Language:** C++ via godot-cpp
**Build system:** SCons (required by GDExtension)
**Target:** Godot 4.3+ stable

---

## Stage 1: Project Structure, Build System, and 4D Math Foundations

### 1.1 Directory Structure

```
godot-4d/
  SConstruct
  src/
    register_types.h
    register_types.cpp
    math/
      vector4d.h          # 4D vector (NOT Godot's Vector4 - see note)
      basis4d.h           # 4x4 rotation/scale matrix for 4D
      transform4d.h       # Basis4D + Vector4D origin
      projection4d.h      # 5x5 projection matrix (4D analog of Projection)
      aabb4d.h            # 4D axis-aligned bounding box (hypercube)
      plane4d.h           # 4D hyperplane (normal is Vector4D + distance)
      math_4d.h           # Utility functions, constants, conversions
    nodes/
      node_4d.h
      node_4d.cpp
      visual/
        ...               # VisualInstance4D, MeshInstance4D, etc.
      physics/
        ...               # PhysicsBody4D, RigidBody4D, etc.
      collision/
        ...               # CollisionShape4D, CollisionPolygon4D, RayCast4D
    servers/
      physics/
        ...               # PhysicsServer4D and Jolt-based implementation
    resources/
      ...                 # Mesh4D, Shape4D, Material4D resources
    editor/
      ...                 # Editor plugins, gizmos, inspectors
    slicer/
      ...                 # GPU-accelerated 4D-to-3D slicing pipeline
  icons/
    ...                   # Orange-tinted versions of Godot 3D node icons
  demo/
    ...                   # Test project
```

### 1.2 Build System (SCons)

Use godot-cpp as the foundation. Follow the official GDExtension C++ guide exactly:
https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

**SConstruct setup:**

- Clone godot-cpp as a git submodule under `godot-cpp/`.
- Your SConstruct must invoke godot-cpp's own SConstruct first, then compile your sources.
- Study the SConstruct from godot-jolt for reference on how a large GDExtension structures its build: https://github.com/godotengine/godot/tree/master/modules/jolt_physics
- Do NOT use CMake. Do NOT use Meson. SCons only.

**Key SConstruct requirements:**

- Compile all `.cpp` files under `src/` recursively.
- Link against godot-cpp.
- Output the shared library to `demo/addons/godot_4d/bin/`.
- Generate the `.gdextension` file that points to the built library for each platform.
- Support at minimum: `linux.debug.x86_64`, `linux.release.x86_64`, `windows.debug.x86_64`, `windows.release.x86_64`, `macos.debug`, `macos.release`.

**WARNING:** Do NOT try to use `godot::ClassDB::register_class<>()` outside of the `register_types.cpp` initialization function. All class registrations happen in `initialize_4d_module()` and are called from the GDExtension entry point. Read the godot-cpp example carefully to understand the initialization levels (SCENE, EDITOR, SERVERS).

**Register types at the correct initialization levels:**

- Math types, resources, server classes: `MODULE_INITIALIZATION_LEVEL_SERVERS`
- Node classes: `MODULE_INITIALIZATION_LEVEL_SCENE`
- Editor plugins, gizmos: `MODULE_INITIALIZATION_LEVEL_EDITOR`

### 1.3 4D Math Foundations

This is the bedrock of the entire extension. Every other system depends on these types being correct.

#### 1.3.1 Which Math Library to Use

Use **Eigen** for internal heavy computation (matrix operations, decompositions, solving). Eigen is header-only, easy to integrate, and handles arbitrary-dimension math well.

However, all types exposed to Godot's API must be wrapped in your own classes that bind properly through godot-cpp. You cannot expose Eigen types directly to GDScript.

**Pattern:**
- Internal math uses Eigen for computation.
- Your `Vector4D`, `Basis4D`, `Transform4D` classes wrap the Eigen types.
- These classes register properties and methods with Godot's ClassDB so they work in GDScript and the inspector.

Add Eigen as a git submodule or download the header-only release into `thirdparty/eigen/`.

#### 1.3.2 Core Math Types

Study Godot's existing 3D math types as your template. The source is at:
https://github.com/godotengine/godot/tree/master/core/math

For each 3D type, you create a 4D analog:

| Godot 3D Type | Your 4D Type | Key Differences |
|---|---|---|
| `Vector3` | `Vector4D` | 4 components (x,y,z,w). Do NOT confuse with Godot's built-in `Vector4` which is used for homogeneous coordinates and color. Name yours distinctly, e.g. `Vector4D` or `Vec4D`. |
| `Basis` (3x3) | `Basis4D` (4x4) | 4x4 orthogonal matrix representing rotation and scale in 4D. Has 4 column vectors. Rotation in 4D is described by 6 independent rotation planes (XY, XZ, XW, YZ, YW, ZW) rather than 3 Euler angles. |
| `Transform3D` | `Transform4D` | `Basis4D` + `Vector4D` origin. This is the fundamental spatial transform for all 4D nodes. |
| `AABB` | `AABB4D` | Position (Vector4D) + size (Vector4D). A 4D axis-aligned bounding hyperbox. |
| `Plane` | `Plane4D` | Normal (Vector4D) + distance (real_t). Defines a 3D hyperplane in 4D space. Used for slicing. |
| `Projection` (4x4) | `Projection4D` (5x5) | For projecting from 4D to 3D. Rarely needed directly - the main "projection" from 4D is the hyperplane slice, not a perspective projection. Keep this for completeness. |

**CRITICAL - Rotations in 4D:**

4D rotations live in SO(4) and are NOT representable by a single quaternion. Do NOT try to use a single quaternion for 4D rotation. The options are:

1. **Pair of quaternions (double cover of SO(4))**: Every SO(4) rotation can be decomposed as a left-isoclinic and right-isoclinic rotation, each representable by a quaternion. This is the most compact representation. Study: https://en.wikipedia.org/wiki/Rotations_in_4-dimensional_Euclidean_space
2. **4x4 orthogonal matrix**: Simpler to implement and debug. Less compact, but straightforward. This is what `Basis4D` is.
3. **Bivector/Rotor (Geometric Algebra)**: Elegant but adds complexity. Skip this unless you're already familiar.

**Recommended approach:** Use the 4x4 matrix (`Basis4D`) as the primary representation, just as Godot uses `Basis` (3x3 matrix) as the primary rotation/scale representation for 3D. Provide utility functions to construct rotations from angle + plane-of-rotation (e.g., `Basis4D::rotated(PlaneAxis plane, real_t angle)` where `PlaneAxis` is an enum: XY, XZ, XW, YZ, YW, ZW).

DO NOT try to implement "4D Euler angles." There is no standard convention and it will create confusion. Provide rotation by specifying a rotation plane and angle, which is unambiguous.

#### 1.3.3 Vector4D

Look at Godot's `Vector3` implementation for the API surface. Your `Vector4D` must support:

- All arithmetic operators (+, -, *, /, unary -)
- `dot(Vector4D)`, `length()`, `length_squared()`, `normalized()`, `distance_to(Vector4D)`
- `lerp()`, `slerp()` (for unit vectors)
- `abs()`, `sign()`, `floor()`, `ceil()`, `round()`, `clamp()`
- `min_axis_index()`, `max_axis_index()`
- Component access by index: `operator[](int axis)`
- Conversion to/from Godot's `Vector4` for interop (but keep them conceptually distinct)

**IMPORTANT:** Godot already has a `Vector4` type. Yours must be named differently to avoid collision. Use `Vector4D`. When registering with ClassDB, the class name string should be `"Vector4D"`.

#### 1.3.4 Basis4D

Study Godot's `Basis` (3x3): https://github.com/godotengine/godot/blob/master/core/math/basis.h

Your `Basis4D` is a 4x4 matrix. It must support:

- Construction from 4 column vectors
- `rotate(PlaneAxis plane, real_t angle)` - rotation in one of the 6 fundamental planes
- `get_rotation_plane_angle(PlaneAxis plane)` - extract rotation angle for a given plane
- `scaled(Vector4D)` - apply scale
- `get_scale()` - extract scale (Vector4D)
- `orthonormalized()` - Gram-Schmidt
- `determinant()`
- `inverse()`, `transposed()`
- `operator*(Basis4D)`, `operator*(Vector4D)` (transform a vector)
- `IDENTITY` constant

Use Eigen internally for `inverse()` and `determinant()` to avoid bugs in hand-rolled 4x4 inversion.

#### 1.3.5 Transform4D

Direct analog of `Transform3D`:

```
struct Transform4D {
    Basis4D basis;
    Vector4D origin;
};
```

Must support:
- `affine_inverse()`
- `operator*(Transform4D)` - composition
- `xform(Vector4D)` - transform a point
- `xform_inv(Vector4D)` - inverse transform
- `interpolate_with(Transform4D, real_t)` - for animation
- `IDENTITY` constant

#### 1.3.6 AABB4D

Direct analog of `AABB`:
- `position` (Vector4D) + `size` (Vector4D)
- `intersects(AABB4D)`, `encloses(AABB4D)`, `merge(AABB4D)`
- `has_point(Vector4D)`
- `get_center()`, `get_volume()` (hypervolume)
- `grow(real_t)`, `expand(Vector4D)`

#### 1.3.7 Plane4D (Hyperplane)

A 3D hyperplane embedded in 4D space. Defined by:
- `normal` (Vector4D, unit length)
- `d` (real_t, signed distance from origin)

The hyperplane is the set of all points P where `normal.dot(P) = d`.

Must support:
- `distance_to(Vector4D point)` - signed distance
- `is_point_over(Vector4D point)`
- `intersects_segment(Vector4D a, Vector4D b)` - returns intersection point if segment crosses hyperplane
- `project(Vector4D point)` - project point onto hyperplane

This type is central to the slicing system.

#### 1.3.8 Exposing Math Types to GDScript

These math types need to be usable from GDScript. Since GDExtension cannot add new Variant types, you have two options:

**Option A (Recommended):** Register them as RefCounted-based wrapper classes. GDScript users will do:
```gdscript
var t = Transform4D.new()
t.origin = Vector4D.new(1, 2, 3, 4)
```

**Option B:** Use Godot's built-in types as transport (e.g., use `PackedFloat64Array` for serialization, `Vector4` for the vector part). Less ergonomic but avoids object overhead.

Go with Option A. Create RefCounted wrappers for Vector4D, Basis4D, Transform4D, AABB4D, Plane4D. The internal math can still use stack-allocated structs; the wrappers just hold an instance and expose methods to ClassDB.

**WARNING:** Do NOT try to register these as Variant types. GDExtension does not support custom Variant types. They must be Object-derived (RefCounted).

---

## Stage 2: Node4D Base Class and Scene Tree Integration

### 2.1 Study Material

Before implementing, study these Godot source files carefully:

- `Node3D` (formerly Spatial): https://github.com/godotengine/godot/blob/master/scene/3d/node_3d.h and `.cpp`
- How `Node3D` interacts with its parent for transform inheritance
- The notification system (`NOTIFICATION_ENTER_TREE`, `NOTIFICATION_TRANSFORM_CHANGED`, etc.)
- GDExtension class registration: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html

### 2.2 Node4D - The Base of Everything

`Node4D` is the 4D analog of `Node3D`. Every 4D node in this extension inherits from it. It inherits from Godot's `Node` (NOT `Node3D`).

**Why inherit from Node, not Node3D?**
`Node3D` carries a 3D transform, integrates with the 3D rendering server, and participates in the 3D transform hierarchy. None of that applies to your 4D nodes. You need your own parallel transform system. Inheriting from `Node3D` would create conflicts - two competing transforms, unwanted 3D visibility flags, etc. `Node` gives you scene tree participation (enter/exit tree, process callbacks, parenting) without the 3D baggage.

```
File: src/nodes/node_4d.h
File: src/nodes/node_4d.cpp
```

#### 2.2.1 Core Properties

Node4D must expose these properties to the inspector and GDScript:

| Property | Type | Description |
|---|---|---|
| `transform_4d` | `Transform4D` (Ref) | Local transform relative to parent |
| `global_transform_4d` | `Transform4D` (Ref) | World-space transform (computed) |
| `position_4d` | `Vector4D` (Ref) | Shortcut for `transform_4d.origin` |
| `global_position_4d` | `Vector4D` (Ref) | Shortcut for `global_transform_4d.origin` |
| `basis_4d` | `Basis4D` (Ref) | Shortcut for `transform_4d.basis` |
| `global_basis_4d` | `Basis4D` (Ref) | Shortcut for `global_transform_4d.basis` |
| `visible` | `bool` | Whether this node and children are visible in the 4D sense |
| `top_level` | `bool` | If true, transform is not affected by parent (like Node3D.top_level) |

**NOTE on rotation properties:** Node3D exposes `rotation` (Euler), `rotation_degrees`, `quaternion`, `rotation_edit_mode`, and `rotation_order` as convenience properties that decompose the basis. In 4D there are no Euler angles and no single quaternion. Node4D should instead expose `basis_4d` directly. For editor convenience, consider adding per-plane rotation angle properties or an array of 6 plane-angle pairs. This can be deferred to Stage 9 (editor integration).

**NOTE on scale:** Node3D has a `scale` property (Vector3). In 4D, the Basis4D can encode scale, but non-uniform scale combined with rotation is harder to decompose. For simplicity, you may omit a separate `scale_4d` property initially, or add one that operates on the diagonal of the basis. Warn that non-uniform scale is not well-supported by the physics system.

**NOTE on visibility:** Node3D has a `visibility_changed` signal and `NOTIFICATION_VISIBILITY_CHANGED = 43`. Node4D should replicate this with its own `visibility_4d_changed` signal and `NOTIFICATION_VISIBILITY_4D_CHANGED = 3001`. Also provide `is_visible_in_tree()` that walks the parent chain to check combined visibility. Note: Godot's editor scene tree eyeball icon is hardcoded for Node3D/CanvasItem. For GDExtension Node4D, the eyeball will not appear automatically. This is a known limitation (see https://github.com/godotengine/godot-proposals/issues/8812).

**NOTE on global properties:** Node3D has `global_position`, `global_rotation`, `global_rotation_degrees`, `global_scale`, `global_basis`, `global_transform` - all marked with `PROPERTY_USAGE_NO_EDITOR` (accessible from script but not shown in the inspector). Follow the same pattern: expose `global_position_4d`, `global_basis_4d`, `global_transform_4d` from script with `PROPERTY_USAGE_NO_EDITOR`.

**IMPORTANT - Property binding pattern:**

Since `Transform4D` and `Vector4D` are RefCounted objects (see Stage 1.3.8), properties must be `Ref<Transform4D>` and `Ref<Vector4D>`. When binding these to Godot's property system:

```cpp
ClassDB::bind_method(D_METHOD("get_transform_4d"), &Node4D::get_transform_4d);
ClassDB::bind_method(D_METHOD("set_transform_4d", "transform"), &Node4D::set_transform_4d);
ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "transform_4d", PROPERTY_HINT_RESOURCE_TYPE, "Transform4D"), "set_transform_4d", "get_transform_4d");
```

Study how Node3D binds `transform`, `position`, `rotation`, `scale` in `node_3d.cpp` and replicate the pattern for your 4D equivalents.

#### 2.2.2 Transform Hierarchy

This is the most important behavior to get right. Study how `Node3D` propagates transforms to children.

**How it works in 3D (replicate this for 4D):**

1. Each `Node3D` stores a local `Transform3D`.
2. When a node's transform changes, it marks itself and all child `Node3D` nodes as "dirty" (global transform needs recalculation).
3. When `get_global_transform()` is called, if dirty, it computes: `parent_global * local` and caches the result.
4. This lazy evaluation avoids recomputing on every frame if nothing reads the global transform.

**Your implementation must do the same thing:**

- Store `Transform4D local_transform` (the actual data, not the Ref wrapper).
- Store `Transform4D global_transform_cache`.
- Store `bool global_transform_dirty = true`.
- When `set_transform_4d()` or `set_position_4d()` is called, call `_propagate_transform_changed()`.
- `_propagate_transform_changed()` sets `global_transform_dirty = true` on self and recursively on all child `Node4D` nodes.
- `get_global_transform_4d()` checks dirty flag; if dirty, walks up the parent chain to compute the global transform, caches it, clears the flag.

**WARNING:** When walking children to propagate dirty flags, only visit children that are `Node4D`. Other node types (e.g., a regular `Node` or `Control` child) should be skipped. Use `Object::cast_to<Node4D>(child)` to check.

**WARNING:** Do NOT recompute the global transform eagerly on every local transform change. That is O(n) per change in a deep tree. The lazy/dirty pattern is O(1) per change and O(depth) per read, which is far better.

#### 2.2.3 Notifications

Override `_notification(int p_what)` to handle key lifecycle events. Study how `Node3D::_notification` works and replicate for 4D:

| Notification | Action |
|---|---|
| `NOTIFICATION_ENTER_TREE` | Find parent `Node4D` if any, register with it. Mark global transform dirty. If this node has visual/physics components, register with the respective 4D servers. |
| `NOTIFICATION_EXIT_TREE` | Unregister from parent `Node4D`. Unregister from 4D servers. |
| `NOTIFICATION_PARENTED` | Update parent `Node4D` reference. |
| `NOTIFICATION_UNPARENTED` | Clear parent `Node4D` reference. |

**NOTE:** You will also need a custom notification for transform changes. Define your own:
```cpp
enum {
    NOTIFICATION_TRANSFORM_4D_CHANGED = 3000,
    NOTIFICATION_VISIBILITY_4D_CHANGED = 3001,
};
```
**WARNING:** Do NOT use 2000 for your custom notification. Node3D already defines `NOTIFICATION_TRANSFORM_CHANGED = 2000`. Other values in the 2000-2014 range are also taken by Godot core (e.g., `RESET_PHYSICS_INTERPOLATION = 2001`, `OS_MEMORY_WARNING = 2009`, `TRANSLATION_CHANGED = 2010`, `CRASH = 2012`). Use 3000+ to stay safely out of range. The 1000-1099 range is also taken by window manager notifications.

Also avoid values 41-44 which Node3D uses for `ENTER_WORLD`, `EXIT_WORLD`, `VISIBILITY_CHANGED`, and `LOCAL_TRANSFORM_CHANGED`. Since Node4D inherits from Node (not Node3D), these won't conflict at the class level, but reusing the same integer values could cause confusion if a developer checks notification numbers manually.

When `_propagate_transform_changed()` fires, send `NOTIFICATION_TRANSFORM_4D_CHANGED` to self so subclasses can react (e.g., physics bodies update their server-side position).

#### 2.2.4 Convenience Methods

Following Node3D's API, provide:

| Method | Description |
|---|---|
| `rotate(PlaneAxis plane, real_t angle)` | Rotate local transform in the given plane |
| `rotate_local(PlaneAxis plane, real_t angle)` | Rotate in local space |
| `translate(Vector4D offset)` | Move in parent space |
| `translate_local(Vector4D offset)` | Move in local space |
| `global_rotate(PlaneAxis plane, real_t angle)` | Rotate in global space |
| `global_translate(Vector4D offset)` | Move in global space |
| `look_at_4d(Vector4D target, ...)` | Orient to face a target. This is more complex in 4D - you need to specify which "forward" axis and which "up" plane. For now, implement a basic version that aligns one axis toward the target. |
| `to_local(Vector4D global_point)` | Convert global to local coordinates |
| `to_global(Vector4D local_point)` | Convert local to global coordinates |
| `get_parent_node_4d()` | Returns parent Node4D or null |
| `orthonormalize()` | Gram-Schmidt orthonormalization on basis (like Node3D.orthonormalize) |
| `set_identity()` | Reset transform to identity |
| `force_update_transform()` | Force recompute global transform (like Node3D.force_update_transform) |
| `set_notify_transform_4d(bool)` | Opt-in for NOTIFICATION_TRANSFORM_4D_CHANGED |
| `is_transform_notification_enabled_4d()` | Query opt-in state |
| `set_notify_local_transform_4d(bool)` | Opt-in for local transform change notification |
| `is_local_transform_notification_enabled_4d()` | Query opt-in state |
| `show()` | Set visible = true |
| `hide()` | Set visible = false |
| `is_visible_in_tree()` | Check if this and all ancestors are visible |

**IMPORTANT - opt-in notification pattern:** In Node3D, `NOTIFICATION_TRANSFORM_CHANGED` is NOT sent by default. Users must call `set_notify_transform(true)` to opt in. This is a performance optimization - most nodes don't need to react to every parent transform change. Node4D must replicate this pattern. The `_propagate_transform_changed()` method should check `is_transform_notification_enabled_4d()` before sending the notification.

#### 2.2.5 Signal

Define at minimum:
```cpp
ADD_SIGNAL(MethodInfo("transform_4d_changed"));
```

Emitted when the local transform changes. Subclasses and user scripts can connect to this for reactive behavior.

#### 2.2.6 The Full Inheritance Tree

Here is the complete class hierarchy you are building. Every class listed here inherits from `Node4D` (which inherits from `Node`). The tree mirrors Godot's 3D hierarchy:

```
Node (Godot built-in)
  +-- Node4D (YOUR base class)
        |
        +-- CollisionObject4D (base for physics-interactive objects)
        |     +-- PhysicsBody4D (base for bodies)
        |     |     +-- StaticBody4D
        |     |     +-- AnimatableBody4D
        |     |     +-- CharacterBody4D
        |     |     +-- RigidBody4D
        |     +-- Area4D
        |
        +-- VisualInstance4D (base for renderable 4D objects)
        |     +-- GeometryInstance4D (base for geometry)
        |     |     +-- SpriteBase4D
        |     |     |     +-- Sprite4D
        |     |     |     +-- AnimatedSprite4D
        |     |     +-- MeshInstance4D
        |     +-- Light4D (intermediate! do NOT skip)
        |           +-- DirectionalLight4D
        |           +-- OmniLight4D
        |           +-- SpotLight4D
        |
        +-- Camera4D
        +-- Joint4D
        |     +-- PinJoint4D
        |     +-- HingeJoint4D
        |     +-- SliderJoint4D
        |     +-- ConeTwistJoint4D
        |     +-- Generic10DOFJoint4D
        +-- CollisionShape4D
        +-- CollisionPolygon4D
        +-- RayCast4D
```

**CRITICAL:** This tree must match the 3D analog exactly. If you are unsure about where a class sits in the 3D hierarchy, look it up:
https://docs.godotengine.org/en/stable/classes/index.html

Verify the exact Godot 3D inheritance chain. For example, `DirectionalLight3D` inherits from `Light3D`, which inherits from `VisualInstance3D`. Do NOT skip intermediate classes. The `Light4D` intermediate between `VisualInstance4D` and the concrete light types is required.

The Godot 3D tree for reference:

```
Node3D
  +-- CollisionObject3D
  |     +-- PhysicsBody3D
  |     |     +-- StaticBody3D
  |     |     +-- AnimatableBody3D
  |     |     +-- CharacterBody3D
  |     |     +-- RigidBody3D
  |     +-- Area3D
  +-- VisualInstance3D
  |     +-- GeometryInstance3D
  |     |     +-- SpriteBase3D
  |     |     |     +-- Sprite3D
  |     |     |     +-- AnimatedSprite3D
  |     |     +-- MeshInstance3D
  |     +-- Light3D (intermediate!)
  |           +-- DirectionalLight3D
  |           +-- OmniLight3D
  |           +-- SpotLight3D
  +-- Camera3D
  +-- CollisionShape3D
  +-- CollisionPolygon3D
  +-- RayCast3D
```

So the final 4D hierarchy must be:

```
Node4D
  +-- CollisionObject4D
  |     +-- PhysicsBody4D
  |     |     +-- StaticBody4D
  |     |     +-- AnimatableBody4D
  |     |     +-- CharacterBody4D
  |     |     +-- RigidBody4D
  |     +-- Area4D
  +-- VisualInstance4D
  |     +-- GeometryInstance4D
  |     |     +-- SpriteBase4D
  |     |     |     +-- Sprite4D
  |     |     |     +-- AnimatedSprite4D
  |     |     +-- MeshInstance4D
  |     +-- Light4D
  |           +-- DirectionalLight4D
  |           +-- OmniLight4D
  |           +-- SpotLight4D
  +-- Camera4D
  +-- Joint4D
  |     +-- PinJoint4D
  |     +-- HingeJoint4D
  |     +-- SliderJoint4D
  |     +-- ConeTwistJoint4D
  |     +-- Generic10DOFJoint4D
  +-- CollisionShape4D
  +-- CollisionPolygon4D
  +-- RayCast4D
```

#### 2.2.7 Class Registration

Every class must be registered in `register_types.cpp`. The pattern:

```cpp
#include <godot_cpp/core/class_db.hpp>
#include "nodes/node_4d.h"

void initialize_4d_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        ClassDB::register_class<Node4D>();
        ClassDB::register_class<CollisionObject4D>();
        ClassDB::register_class<PhysicsBody4D>();
        ClassDB::register_class<StaticBody4D>();
        // ... all node classes
    }
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
        // Math wrappers, resources, PhysicsServer4D
    }
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        // Editor plugins, gizmos
    }
}
```

**WARNING:** Abstract base classes (like `CollisionObject4D`, `PhysicsBody4D`, `VisualInstance4D`) should be registered with `ClassDB::register_abstract_class<>()` so users cannot instantiate them directly. Only concrete classes (like `StaticBody4D`, `RigidBody4D`, `MeshInstance4D`) use `register_class<>()`.

#### 2.2.8 Stub Implementation Strategy

For Stage 2, implement `Node4D` fully (transform hierarchy, notifications, signals, convenience methods). For all other classes in the tree, create **stubs**: the header and source files with correct inheritance, empty `_bind_methods()`, and registration in `register_types.cpp`. They don't need functionality yet - that comes in later stages. But the inheritance tree and class registration must be correct from the start so you can test compilation and basic scene tree behavior.

Example stub:

```cpp
// collision_object_4d.h
#pragma once
#include "node_4d.h"

class CollisionObject4D : public Node4D {
    GDCLASS(CollisionObject4D, Node4D);
protected:
    static void _bind_methods();
};

// collision_object_4d.cpp
#include "collision_object_4d.h"
void CollisionObject4D::_bind_methods() {}
```

Do this for ALL classes in the tree. Verify the project compiles and you can add a `Node4D` to a scene in the Godot editor.

#### 2.2.9 GDExtension Notification Dispatch (Godot 4.3+)

In Godot 4.2, GDExtension class hierarchies had a bug where `_notification` was not automatically dispatched up the inheritance chain. In Godot 4.3+, this is fixed: if both `CollisionObject4D` and `Node4D` override `_notification`, both will be called automatically by the engine. Do NOT manually call the parent `_notification` from subclasses - Godot handles the dispatch. See https://github.com/godotengine/godot-cpp/issues/1498 for details.

---

## Stage 3: Resource Types

Resources are the data containers that nodes reference. Just as `MeshInstance3D` holds a `Ref<Mesh>`, your `MeshInstance4D` will hold a `Ref<Mesh4D>`. All resources inherit from Godot's `Resource` class, which gives you serialization, inspector editing, and sharing between nodes for free.

### 3.1 Study Material

- Godot `Resource` class: https://docs.godotengine.org/en/stable/classes/class_resource.html
- How Godot's 3D resources work in source: https://github.com/godotengine/godot/tree/master/scene/resources
- Specifically study: `mesh.h`, `shape_3d.h`, `primitive_meshes.h`, `environment.h`
- Custom resources in GDExtension: https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html (the resource section)

### 3.2 Resource Hierarchy

Mirror the 3D resource hierarchy. Here is the full set of resources you need:

```
Resource (Godot built-in)
  +-- Mesh4D (base for all 4D mesh data)
  |     +-- ArrayMesh4D (user-constructed mesh from arrays)
  |     +-- PrimitiveMesh4D (base for procedural primitives)
  |           +-- HyperBoxMesh4D (analog of BoxMesh - a tesseract)
  |           +-- HyperSphereMesh4D (analog of SphereMesh - a 3-sphere/glome)
  |           +-- HyperCylinderMesh4D (analog of CylinderMesh - spherinder or cubinder)
  |           +-- HyperCapsuleMesh4D (analog of CapsuleMesh)
  |
  +-- Shape4D (base for all physics collision shapes)
  |     +-- BoxShape4D (tesseract collision shape)
  |     +-- SphereShape4D (3-sphere collision shape)
  |     +-- CapsuleShape4D
  |     +-- CylinderShape4D
  |     +-- ConvexPolygonShape4D (convex polytope in 4D)
  |     +-- ConcavePolygonShape4D (triangle-mesh analog - tetrahedral mesh)
  |     +-- WorldBoundaryShape4D (infinite hyperplane - analog of WorldBoundaryShape3D)
  |     +-- SeparationRayShape4D
  |
  +-- SpriteFrames4D (for AnimatedSprite4D - identical to SpriteFrames but referencing Texture4D)
  +-- Texture4D (base - a 4D texture is a 3D volume texture with W-slicing)
  +-- Material4D (wrapper holding material properties for 4D surfaces)
  +-- Environment4D (ambient light, fog, etc. in 4D space)
```

Verify the 3D analogs by checking Godot docs. For instance, confirm that `BoxShape3D`, `SphereShape3D`, `CapsuleShape3D`, `CylinderShape3D`, `ConvexPolygonShape3D`, `ConcavePolygonShape3D`, `WorldBoundaryShape3D`, and `SeparationRayShape3D` all exist as children of `Shape3D`. Add any that are missing from the list above.

### 3.3 Mesh4D

This is the most complex resource. Study Godot's `Mesh` class thoroughly:
https://github.com/godotengine/godot/blob/master/scene/resources/mesh.h

#### 3.3.1 What a 4D Mesh Contains

A 3D mesh is made of triangles (3 vertices forming a 2-simplex). A 4D mesh is made of **tetrahedra** (4 vertices forming a 3-simplex). Each tetrahedron defines a "face" of the 4D volume, analogous to how triangles define the surface of a 3D volume.

**Mesh4D data arrays (analog of Mesh::ARRAY_*):**

| Array | Type | Description |
|---|---|---|
| `ARRAY_VERTEX` | `PackedFloat32Array` (4 floats per vertex) | 4D vertex positions (x,y,z,w) |
| `ARRAY_NORMAL` | `PackedFloat32Array` (4 floats per vertex) | 4D normals (unit Vector4D, perpendicular to the tetrahedral cell surface) |
| `ARRAY_TANGENT` | `PackedFloat32Array` (multiple per vertex) | In 4D, the tangent space of a 3-cell boundary is 3-dimensional. Store appropriately. |
| `ARRAY_TEX_UV` | `PackedFloat32Array` (3 floats per vertex) | UV coordinates are 3D for 4D geometry (mapping from 3D boundary to 3D texture space). Alternatively, keep 2D UVs if textures remain 2D. |
| `ARRAY_COLOR` | `PackedColorArray` | Vertex colors, same as 3D |
| `ARRAY_INDEX` | `PackedInt32Array` | Indices into vertex array, in groups of 4 (each group = one tetrahedron) |

**CRITICAL:** In 3D, indices come in groups of 3 (triangles). In 4D, indices come in groups of 4 (tetrahedra). The slicing pipeline (Stage 4) will take these tetrahedra and intersect them with a hyperplane to produce triangles for rendering.

#### 3.3.2 ArrayMesh4D

Analog of `ArrayMesh`. The user constructs a mesh by providing arrays:

```gdscript
var mesh = ArrayMesh4D.new()
var arrays = []
arrays.resize(Mesh4D.ARRAY_MAX)
arrays[Mesh4D.ARRAY_VERTEX] = vertex_data
arrays[Mesh4D.ARRAY_INDEX] = index_data
mesh.add_surface(arrays)
```

Study `ArrayMesh::add_surface_from_arrays()` in the Godot source for the pattern.

Internal storage: each surface stores its arrays and metadata (material, primitive type). The surfaces are what the slicer operates on.

#### 3.3.3 PrimitiveMesh4D and Primitives

Analog of `PrimitiveMesh`. Generates mesh data procedurally when parameters change.

Study `PrimitiveMesh::_update()` in Godot source. The pattern:
1. Parameters are set (e.g., size).
2. Mesh is marked dirty.
3. On next access, `_create_mesh_array()` is called (virtual, overridden by each primitive).
4. The generated arrays are cached until parameters change.

**HyperBoxMesh4D (Tesseract):**

A tesseract has 8 cubic cells (the 4D analog of a cube having 6 square faces). Each cubic cell is made of 6 faces, each face of 2 triangles. But since your mesh format uses tetrahedra, you need to tetrahedralize the volume.

A tesseract can be decomposed into 24 tetrahedra. The vertices of a unit tesseract are all 16 combinations of (+/-0.5, +/-0.5, +/-0.5, +/-0.5).

For generating the tetrahedralization, use a Delaunay tetrahedralization of the 16 vertices, or manually construct the decomposition. The manual decomposition is well-documented:
- https://en.wikipedia.org/wiki/Tesseract
- Search for "tesseract tetrahedralization" or "hypercube simplicial decomposition"

Parameters: `Vector4D size` (default: (1,1,1,1))

**HyperSphereMesh4D (Glome/3-sphere):**

The boundary of a 4D sphere is a 3-sphere. Approximate it by recursive subdivision or an icosahedral-analog approach. Start with a regular polytope inscribed in the 3-sphere (e.g., the 24-cell, 600-cell) and subdivide.

The 600-cell is a natural starting point - it's the 4D analog of the icosahedron and tessellates the 3-sphere into 600 tetrahedral cells.

Parameters: `real_t radius`, `int subdivisions`

Reference for 600-cell vertices: https://en.wikipedia.org/wiki/600-cell

**HyperCylinderMesh4D and HyperCapsuleMesh4D:**

The 4D analogs are less standardized. A "spherinder" (sphere x line segment) is one natural choice for the cylinder. A capsule is a cylinder capped with hemisphere analogs.

For the cylinder: take a 3-sphere cross-section, extrude it along the W axis, tetrahedralize. For the capsule: same but with hemispherical caps.

Parameters should mirror the 3D versions: `real_t radius`, `real_t height`, plus subdivision controls.

### 3.4 Shape4D (Physics Shapes)

These are simpler than meshes because the physics engine works with analytical shapes, not explicit tetrahedra.

#### 3.4.1 BoxShape4D

Analog of `BoxShape3D`. Defined by a `Vector4D size` property representing the full extents (not half-extents).

**Confirmed convention:** In Godot 4.x, `BoxShape3D.size` is the full size (e.g., size = (2,2,2) means 2m x 2m x 2m). This changed from Godot 3.x which used `extents` for half-extents. Internally, Jolt uses half-extents, so convert: `half_extents = size * 0.5` when passing to the physics engine. Match the public API to BoxShape3D.size convention.

#### 3.4.2 SphereShape4D

Analog of `SphereShape3D`. Single `real_t radius`. The simplest shape. In 4D, a sphere is still defined by center + radius; the difference is just the dimensionality of the center point.

#### 3.4.3 CapsuleShape4D

Analog of `CapsuleShape3D`. `real_t radius` + `real_t height`. The 4D capsule is a Minkowski sum of a line segment and a 3-sphere (just as the 3D capsule is a line segment + 2-sphere).

#### 3.4.4 CylinderShape4D

Analog of `CylinderShape3D`. `real_t radius` + `real_t height`. The 4D cylinder here means a 2-sphere extruded along one axis (the W axis by convention).

#### 3.4.5 ConvexPolygonShape4D

Analog of `ConvexPolygonShape3D`. Defined by a set of 4D points; the physics engine computes the convex hull. In 4D, the convex hull of N points is a convex polytope bounded by tetrahedral cells.

Property: `PackedFloat32Array points` (groups of 4 floats, each group a 4D point).

#### 3.4.6 ConcavePolygonShape4D

Analog of `ConcavePolygonShape3D` (trimesh). In 4D, this is a "tetra-mesh" - an arbitrary collection of tetrahedra that define a collision surface. Used for static level geometry.

Property: `PackedFloat32Array faces` (groups of 16 floats = 4 vertices x 4 components per vertex = one tetrahedron).

**WARNING:** Just like `ConcavePolygonShape3D`, this is for static bodies only. Do NOT allow it on RigidBody4D or CharacterBody4D. The physics server should reject it or produce an error.

#### 3.4.7 WorldBoundaryShape4D

A single `Plane4D` (hyperplane). An infinite boundary. Analog of `WorldBoundaryShape3D`.

#### 3.4.8 SeparationRayShape4D

Analog of `SeparationRayShape3D`. A ray of given length used for separation. Property: `real_t length`, `bool slide_on_slope`.

### 3.5 Texture4D and Material4D

These are simpler wrappers:

**Texture4D:** A 4D texture is conceptually a volumetric texture sampled at 3D coordinates (since 4D surface UVs are 3D). In practice, for the initial implementation, wrap a Godot `Texture3D` (which is a 3D volume texture) and use it for UV lookup. This can be expanded later.

Alternatively, if textures are kept 2D (which is simpler and often sufficient for games), Texture4D can just wrap a regular `Texture2D` and the UV mapping ignores the third coordinate. Start with this simpler approach.

**Material4D:** Wraps the properties that affect rendering of 4D surfaces after slicing. Since sliced geometry is ultimately rendered by Godot's 3D renderer, Material4D should produce a `StandardMaterial3D` (or `ShaderMaterial`) that gets applied to the sliced 3D mesh. Properties to include:

- `albedo_color` (Color)
- `albedo_texture` (Ref<Texture2D>)
- `metallic`, `roughness`, `emission` (same as StandardMaterial3D)
- `transparency` mode

Internally, Material4D generates and caches a `StandardMaterial3D` instance. When the slicer produces a 3D mesh for rendering, it applies this material to it.

### 3.6 SpriteFrames4D

Direct analog of `SpriteFrames`. Holds named animations, each being a sequence of textures (or `Texture4D` references). For the initial implementation, each frame is simply a `Texture2D` displayed on a 4D-positioned quad/cell. The slicing determines whether and how the sprite appears.

Copy the API of `SpriteFrames` exactly: `add_animation()`, `add_frame()`, `get_frame_texture()`, `set_animation_speed()`, etc.

### 3.7 Environment4D

Analog of `Environment` resource. Holds ambient light settings, fog, and other global rendering parameters for 4D scenes. Since the final rendering goes through Godot's 3D pipeline after slicing, many of these can delegate to a standard `Environment` resource internally.

Properties to include:
- `ambient_light_color`, `ambient_light_energy`
- `fog_enabled`, `fog_density`, `fog_color` (4D fog attenuates based on 4D distance, but after slicing it maps to 3D fog)
- `background_mode`

### 3.8 Registration

All resource classes register at `MODULE_INITIALIZATION_LEVEL_SERVERS`:

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
    ClassDB::register_abstract_class<Mesh4D>();
    ClassDB::register_abstract_class<PrimitiveMesh4D>();
    ClassDB::register_class<ArrayMesh4D>();
    ClassDB::register_class<HyperBoxMesh4D>();
    ClassDB::register_class<HyperSphereMesh4D>();
    ClassDB::register_class<HyperCylinderMesh4D>();
    ClassDB::register_class<HyperCapsuleMesh4D>();
    ClassDB::register_abstract_class<Shape4D>();
    ClassDB::register_class<BoxShape4D>();
    ClassDB::register_class<SphereShape4D>();
    // ... etc
}
```

**WARNING:** `Mesh4D`, `Shape4D`, and `PrimitiveMesh4D` are abstract base classes. Register them with `register_abstract_class`. The concrete classes (`ArrayMesh4D`, `HyperBoxMesh4D`, `BoxShape4D`, etc.) use `register_class`. Do NOT duplicate registrations - register each class exactly once.

### 3.9 Serialization

All `Resource`-derived classes get serialization for free through Godot's property system, as long as all stored data is in exported properties of Variant-compatible types. Since your math types (Vector4D, etc.) are RefCounted, they serialize as sub-resources.

However, for large mesh data, use `PackedFloat32Array` and `PackedInt32Array` properties directly rather than arrays of RefCounted `Vector4D` objects. This is much more efficient for storage and loading.

Test that you can:
1. Create a `HyperBoxMesh4D` in the inspector.
2. Save a scene containing a `MeshInstance4D` with that mesh.
3. Reload the scene and confirm the mesh data persists.

---

## Stage 4: Visual System (VisualInstance4D Hierarchy and GPU Slicing Pipeline)

This is the most technically demanding stage. The visual system must take 4D geometry, slice it with a 3D hyperplane determined by the Camera4D, and feed the resulting 3D mesh to Godot's RenderingServer for display.

### 4.1 Study Material

- Godot's RenderingServer API: https://docs.godotengine.org/en/stable/classes/class_renderingserver.html
- How `VisualInstance3D` and `MeshInstance3D` interact with RenderingServer: study `visual_instance_3d.cpp` and `mesh_instance_3d.cpp` in Godot source
- Vulkan compute shaders in Godot: study how Godot's renderer dispatches compute work. The relevant code is in `servers/rendering/renderer_rd/` in Godot source. Pay special attention to `RenderingDevice` class.
- `RenderingDevice` API: https://docs.godotengine.org/en/stable/classes/class_renderingdevice.html - this is how you submit compute shader work from GDExtension
- Cross-section algorithms for simplicial meshes: the Marching Tetrahedra algorithm or direct plane-tetrahedron intersection

### 4.2 Architecture Overview

The rendering pipeline for a 4D object works as follows:

```
4D Mesh (tetrahedra)
    |
    | [GPU compute shader: hyperplane slice]
    v
3D Mesh (triangles) -- generated every frame or when slice plane changes
    |
    | [Godot's RenderingServer]
    v
Screen
```

Each `VisualInstance4D` node:
1. Holds a reference to 4D geometry data (Mesh4D, or sprite data).
2. On each frame (or when the slice plane changes), submits the 4D data to the slicer.
3. The slicer produces a 3D mesh.
4. The 3D mesh is uploaded to a RenderingServer instance (an `RID`) for display.

The slicer is the bridge between the 4D world and Godot's 3D renderer.

### 4.3 The Slice Plane

The slice plane is a 3D hyperplane in 4D space, defined by the active `Camera4D`. It is a `Plane4D` (normal + distance) in world space.

How the Camera4D defines the slice plane:

- The camera has a `Transform4D` giving its position and orientation in 4D.
- The camera's "forward" direction in 4D determines its viewing axis.
- The slice hyperplane passes through the camera's 4D position and is perpendicular to one specific axis of the camera's orientation (the W axis of the camera's local frame, by convention).
- Concretely: the slice plane normal is `camera_global_transform.basis.get_column(3)` (the W column of the Basis4D), and the distance is `normal.dot(camera_global_transform.origin)`.

This means:
- Moving the camera along its local W axis changes which "layer" of 4D space you see (like moving a 2D slice through a 3D object).
- Rotating the camera in any plane involving W tilts the slice, revealing different cross-sections.
- Rotating the camera in XY, XZ, or YZ only rotates the 3D view within the slice, same as a normal 3D camera rotation.

The Camera4D computes and exposes the current slice `Plane4D` each frame. All visual nodes read from the active Camera4D.

### 4.4 GPU Slicing Pipeline

#### 4.4.1 Why GPU?

A tetrahedral mesh with thousands of cells must be sliced every frame. Doing this on the CPU would be too slow for real-time. Godot uses Vulkan (via RenderingDevice), so you will write a Vulkan compute shader that performs the slicing in parallel.

#### 4.4.2 RenderingDevice Compute Shader Approach

Use Godot's `RenderingDevice` API to dispatch compute shaders. This is the same GPU API that Godot's own renderer uses.

**How to use RenderingDevice from GDExtension:**

```cpp
RenderingDevice *rd = RenderingServer::get_singleton()->get_rendering_device();
```

If the local RenderingDevice is not directly accessible from GDExtension, create a local one:

```cpp
RenderingDevice *rd = RenderingServer::get_singleton()->create_local_rendering_device();
```

Study the GDScript compute shader tutorial for the overall pattern, then implement it in C++:
https://docs.godotengine.org/en/stable/tutorials/shaders/compute_shaders.html

The workflow:
1. Create a compute shader (`RID shader`).
2. Create storage buffers for input (4D mesh data) and output (3D mesh data).
3. Create a uniform set binding the buffers.
4. Create a compute pipeline.
5. Each frame: update the slice plane uniform, dispatch the compute shader, read back the output buffer.
6. Upload the output 3D mesh to RenderingServer.

#### 4.4.3 The Slicing Compute Shader

The compute shader operates per-tetrahedron. For each tetrahedron in the 4D mesh, it computes the intersection with the slice hyperplane.

**Algorithm for intersecting a tetrahedron with a hyperplane:**

A tetrahedron has 4 vertices: v0, v1, v2, v3. Each vertex is on one side of the hyperplane or on it. Compute the signed distance of each vertex to the hyperplane:

```
d_i = dot(normal, v_i) - plane_d
```

Count how many vertices are on each side (positive vs negative). The intersection produces:

| Positive | Negative | On plane | Result |
|---|---|---|---|
| 4 | 0 | 0 | No intersection (tetrahedron entirely on one side) |
| 0 | 4 | 0 | No intersection |
| 3 | 1 | 0 | Triangle (3 edge intersection points) |
| 1 | 3 | 0 | Triangle (3 edge intersection points) |
| 2 | 2 | 0 | Quadrilateral (4 edge intersection points) - split into 2 triangles |
| * | * | 1+ | Degenerate cases - vertex on plane, edge on plane, etc. Handle with epsilon tolerance |

For each edge that crosses the hyperplane (one endpoint positive, one negative), compute the intersection point by linear interpolation:

```
t = d_a / (d_a - d_b)
intersection = v_a + t * (v_b - v_a)
```

Also interpolate normals, UVs, and colors the same way.

**Compute shader structure (GLSL):**

```glsl
#[compute]
#version 450

layout(local_size_x = 64) in;

layout(set = 0, binding = 0, std430) restrict readonly buffer InputVertices {
    vec4 vertices[];    // 4D positions (x,y,z,w)
};

layout(set = 0, binding = 1, std430) restrict readonly buffer InputNormals {
    vec4 normals[];     // 4D normals
};

layout(set = 0, binding = 2, std430) restrict readonly buffer InputIndices {
    uint indices[];     // groups of 4
};

layout(set = 0, binding = 3, std430) restrict writeonly buffer OutputVertices {
    vec3 out_vertices[];    // 3D positions after slice
};

layout(set = 0, binding = 4, std430) restrict writeonly buffer OutputNormals {
    vec3 out_normals[];     // 3D normals after slice
};

layout(set = 0, binding = 5, std430) restrict buffer OutputCounter {
    uint triangle_count;    // atomic counter for output triangles
};

layout(push_constant) uniform SlicePlane {
    vec4 plane_normal;      // 4D normal of slice hyperplane
    float plane_d;          // distance
    mat4 slice_basis;       // 3x4 matrix mapping 4D intersection points to 3D coordinates in the slice plane
};
```

**CRITICAL - Mapping 4D intersection points to 3D coordinates:**

When you compute an intersection point on the hyperplane, it is a 4D point. But you need to express it as a 3D point for rendering. You must project it onto 3 orthonormal axes that lie within the hyperplane.

The Camera4D provides these axes: the first three columns of its Basis4D (the X, Y, Z axes of the camera's local frame) all lie within the slice hyperplane (since the W axis is the normal). So the projection is:

```
3d_point.x = dot(camera_basis_col_0, intersection_4d - camera_origin)
3d_point.y = dot(camera_basis_col_1, intersection_4d - camera_origin)
3d_point.z = dot(camera_basis_col_2, intersection_4d - camera_origin)
```

This `slice_basis` (a 3x4 matrix) is passed as a push constant or uniform. It encodes the camera's frame and origin.

Similarly, 4D normals are projected to 3D by extracting the components along the same 3 axes.

#### 4.4.4 Output Buffer and Atomic Counter

Since the number of output triangles is not known in advance (some tetrahedra produce 0, some produce 1, some produce 2 triangles), use an atomic counter:

1. Before dispatch, set `triangle_count = 0`.
2. Each workgroup thread atomically increments `triangle_count` and writes its output triangles at the returned offset.
3. After dispatch, read back `triangle_count` to know the output size.

This is a standard GPU stream compaction pattern.

**WARNING:** Atomic operations on storage buffers require the buffer to have the appropriate usage flags (`STORAGE_BUFFER_USAGE_DISPATCH_INDIRECT` may be needed). Study Godot's RenderingDevice buffer creation flags.

#### 4.4.5 Uploading Sliced Mesh to RenderingServer

After the compute shader runs and you have the 3D triangle data:

1. Read back the output buffer (or use it directly if you can share the buffer with the renderer - this is GPU-GPU, no readback needed).
2. Create or update a RenderingServer mesh instance.

The most efficient approach is to avoid CPU readback entirely. Use the output buffer directly as a vertex buffer for rendering. This requires some careful RID management:

```cpp
RID mesh_rid = RenderingServer::get_singleton()->mesh_create();
// Update surfaces each frame from the compute output
```

Study how Godot's `ImmediateMesh` or `SurfaceTool` work internally for updating mesh data each frame. Also look at `RenderingServer::mesh_surface_update_vertex_region()` for partial updates.

If direct buffer sharing is too complex for the first pass, an acceptable fallback is:
1. Read the compute output buffer back to CPU.
2. Use `RenderingServer::mesh_add_surface()` or the `SurfaceTool` to create a new surface each frame.
3. Delete the old surface.

This is slower but simpler. Get it working first, then optimize.

### 4.5 VisualInstance4D

Base class for all renderable 4D objects. Analog of `VisualInstance3D`.

Study: https://github.com/godotengine/godot/blob/master/scene/3d/visual_instance_3d.h

```
File: src/nodes/visual/visual_instance_4d.h
File: src/nodes/visual/visual_instance_4d.cpp
```

**Core responsibilities:**

- Holds an `RID instance` from RenderingServer (the 3D instance used for display after slicing).
- Holds an `RID mesh_rid` from RenderingServer (the 3D mesh that Slicer4D writes sliced geometry into).
- On `NOTIFICATION_ENTER_TREE`: create the RenderingServer instance and mesh RID, add instance to the current scenario, register with `Slicer4D` singleton.
- On `NOTIFICATION_EXIT_TREE`: unregister from `Slicer4D`, free the RenderingServer instance and mesh RID.
- On `NOTIFICATION_TRANSFORM_4D_CHANGED`: notify `Slicer4D` that this instance's transform is dirty (needs re-slicing).

The node does NOT perform slicing itself. It provides its 4D mesh data and RenderingServer RIDs to `Slicer4D`, which handles all GPU compute work in a batched manner.

**Properties:**

| Property | Type | Description |
|---|---|---|
| `layers` | `uint32_t` | Render layers bitmask (same concept as VisualInstance3D) |
| `sorting_offset` | `real_t` | Sorting offset for transparency |

**Key methods for Slicer4D interaction:**

```cpp
virtual RID get_mesh_rid() const;
virtual const Mesh4DData &get_4d_mesh_data() const;
virtual Ref<Material4D> get_active_material(int surface) const;
```

These are called by `Slicer4D` when collecting data for the batched compute dispatch. Subclasses override them to provide their specific geometry.

### 4.6 GeometryInstance4D

Analog of `GeometryInstance3D`. Adds material and geometry-related properties.

**Properties (mirror GeometryInstance3D):**

| Property | Type | Description |
|---|---|---|
| `material_override` | `Ref<Material4D>` | Override material for all surfaces |
| `cast_shadow` | `ShadowCastingSetting` | Whether to cast shadows (after slicing, shadows work normally in 3D) |
| `gi_mode` | `GIMode` | Global illumination mode |
| `transparency` | `real_t` | Transparency value |

These properties map to the RenderingServer instance properties after slicing.

### 4.7 MeshInstance4D

The primary node for displaying 4D geometry. Analog of `MeshInstance3D`.

```
File: src/nodes/visual/mesh_instance_4d.h
File: src/nodes/visual/mesh_instance_4d.cpp
```

**Properties:**

| Property | Type | Description |
|---|---|---|
| `mesh` | `Ref<Mesh4D>` | The 4D mesh to display |
| `surface_material_overrides` | Array of `Ref<Material4D>` | Per-surface material overrides |

**Behavior:**

1. When `mesh` is set, upload the 4D mesh data to GPU storage buffers (via Slicer4D) for the compute slicer.
2. Provide its mesh data, material, and RenderingServer mesh RID to Slicer4D through the VisualInstance4D interface.
3. Slicer4D handles the actual compute dispatch and writes sliced 3D geometry into this node's mesh RID.
4. The node applies materials from Material4D (which internally holds StandardMaterial3D) to its RenderingServer instance.

**What MeshInstance4D is responsible for (node-level state):**
- Storing the Mesh4D reference and uploading its data when it changes.
- Managing its RenderingServer instance RID (creation, scenario assignment, freeing).
- Setting materials on the instance.
- Notifying Slicer4D when its mesh data or transform changes (so Slicer4D knows to re-slice it).

**What MeshInstance4D is NOT responsible for:**
- Running compute shaders. That is Slicer4D's job.
- Determining the slice plane. That comes from Camera4D via Slicer4D.

**WARNING:** The RenderingServer instance transform should be `Transform3D::IDENTITY` because the slicing compute shader already outputs world-space 3D coordinates (relative to the camera). Do NOT also apply the node's 4D transform as a 3D transform - that would double-transform.

**Optimization - only re-slice when needed:**

Track whether the slice plane or the object's 4D transform has changed since last frame. Slicer4D should manage this: each registered instance has a dirty flag. If neither the slice plane nor the object's transform changed, Slicer4D skips it in the next batch. The node sets its dirty flag via `Slicer4D::mark_dirty(this)` when its transform or mesh changes.

### 4.8 SpriteBase4D, Sprite4D, AnimatedSprite4D

#### 4.8.1 SpriteBase4D

Abstract base. Analog of `SpriteBase3D`. A sprite in 4D is a textured quadrilateral (or more precisely, a textured 3-cell) positioned in 4D space. When sliced, it may appear as a textured polygon in 3D.

The simplest approach: a sprite is a flat 4D object with zero thickness along one axis. Its 4D mesh is a single tetrahedralized quad (2 tetrahedra forming a thin slab). The slicing pipeline handles it like any other mesh.

Alternatively, treat sprites as special cases in the slicer: a sprite has a known simple geometry (a hyperplane-aligned rectangle), so its intersection with the slice plane can be computed analytically without the full compute shader.

Go with the analytical approach for efficiency. The intersection of two hyperplanes (the sprite's plane and the slice plane) is a 2D plane in 4D space. Clip this to the sprite's rectangular bounds to get a line segment, polygon, or nothing. Then render that as a textured 3D quad.

**Properties (from SpriteBase3D):**

| Property | Type | Description |
|---|---|---|
| `centered` | `bool` | Whether sprite is centered on origin |
| `offset` | `Vector4D` | Pixel offset |
| `flip_h`, `flip_v` | `bool` | Horizontal/vertical flip |
| `modulate` | `Color` | Color modulation |
| `pixel_size` | `real_t` | Size of one pixel in 4D units |
| `axis` | int | Which 4D axis the sprite faces (0=X, 1=Y, 2=Z, 3=W) |
| `billboard_mode` | `BillboardMode` | Billboard behavior |
| `alpha_cut` | `AlphaCutMode` | Alpha test mode |

#### 4.8.2 Sprite4D

Analog of `Sprite3D`. Displays a single texture.

**Properties (from Sprite3D):**
- `texture` (Ref<Texture2D>)
- `region_enabled`, `region_rect` (for atlas textures)
- `hframes`, `vframes`, `frame` (for sprite sheets)

The texture is applied to the sliced polygon using UVs computed during the analytical intersection.

#### 4.8.3 AnimatedSprite4D

Analog of `AnimatedSprite3D`. Plays frame-by-frame animation.

**Properties (from AnimatedSprite3D):**
- `sprite_frames` (Ref<SpriteFrames4D>)
- `animation` (StringName)
- `frame` (int)
- `speed_scale` (real_t)
- `autoplay` (StringName)

**Signals:** `animation_finished`, `animation_changed`, `frame_changed`

Copy the animation playback logic from `AnimatedSprite3D` exactly. The only difference is the spatial representation (4D transform, 4D slicing for display).

### 4.9 Slicer4D Singleton

The `Slicer4D` singleton is the central GPU slicing pipeline. It exists because the slicing step (4D to 3D) is entirely new functionality with no 3D analog in Godot - in 3D, meshes go directly to RenderingServer with no intermediate transformation. Since this step requires batched GPU compute dispatches for efficiency, a central coordinator is necessary.

```
File: src/slicer/slicer_4d.h
File: src/slicer/slicer_4d.cpp
File: src/slicer/slice_compute.glsl
```

#### 4.9.1 Responsibilities

- Maintains a list of all active `VisualInstance4D` nodes (registered on enter tree, unregistered on exit tree).
- Owns the GPU compute pipeline: shader RID, pipeline RID, buffer management.
- Each frame, receives the current slice plane and camera basis from the active `Camera4D`.
- Batches all dirty instances into one or a few compute dispatches.
- Writes the resulting 3D triangle data into each instance's RenderingServer mesh RID.
- Tracks dirty flags per instance to avoid redundant re-slicing.

#### 4.9.2 API

```cpp
class Slicer4D {
    static Slicer4D *singleton;
public:
    static Slicer4D *get_singleton();

    void register_instance(VisualInstance4D *instance);
    void unregister_instance(VisualInstance4D *instance);
    void mark_dirty(VisualInstance4D *instance);

    void slice_all(const Plane4D &slice_plane, const Basis4D &camera_basis, const Vector4D &camera_origin);

private:
    Vector<VisualInstance4D *> instances;
    HashSet<VisualInstance4D *> dirty_set;
    RID compute_shader;
    RID compute_pipeline;
    // GPU buffer management...
};
```

#### 4.9.3 Frame Loop

`Camera4D` drives the frame loop. In its `_process()` or via a `frame_pre_draw` connection:

```cpp
void Camera4D::_process(double delta) {
    if (!is_current()) return;

    Plane4D slice_plane = _compute_slice_plane();
    Basis4D basis = get_global_transform_4d().basis;
    Vector4D origin = get_global_transform_4d().origin;

    // If slice plane changed from last frame, mark ALL instances dirty
    if (slice_plane != cached_slice_plane) {
        Slicer4D::get_singleton()->mark_all_dirty();
        cached_slice_plane = slice_plane;
    }

    Slicer4D::get_singleton()->slice_all(slice_plane, basis, origin);
}
```

**How to run each frame:** Use `RenderingServer::get_singleton()->connect("frame_pre_draw", ...)` to hook into the rendering frame. The pre-draw callback is ideal because it runs right before rendering, ensuring the sliced meshes are up-to-date. Alternatively, use `_process()` on Camera4D since it runs every frame when the node is in the tree.

#### 4.9.4 Batched Compute Dispatch

The key optimization. Instead of one compute dispatch per mesh:

1. Collect all dirty instances.
2. Pack their 4D vertex and index data into a single large GPU buffer (or a few buffers if total size exceeds limits).
3. Include per-mesh metadata: offset into the vertex buffer, tetrahedron count, the mesh's 4D transform (world space), and the offset into the output buffer where this mesh's 3D triangles should be written.
4. Dispatch one compute shader over all tetrahedra from all meshes.
5. The compute shader reads per-mesh metadata to apply the correct 4D transform before slicing.
6. After dispatch, read back per-mesh triangle counts and update each instance's RenderingServer mesh RID with its portion of the output buffer.

This amortizes the dispatch overhead and saturates the GPU with a large parallel workload.

For the initial implementation, a simpler approach is acceptable: one dispatch per mesh, but still managed centrally by Slicer4D. Optimize to full batching once the basic pipeline works.

#### 4.9.5 Registration Pattern

```cpp
void VisualInstance4D::_notification(int p_what) {
    if (p_what == NOTIFICATION_ENTER_TREE) {
        Slicer4D::get_singleton()->register_instance(this);
        // Also create RenderingServer instance and mesh RIDs here
    }
    if (p_what == NOTIFICATION_EXIT_TREE) {
        Slicer4D::get_singleton()->unregister_instance(this);
        // Also free RenderingServer RIDs here
    }
}
```

When a node's 4D transform changes or its mesh data changes:
```cpp
void MeshInstance4D::set_mesh(Ref<Mesh4D> p_mesh) {
    mesh = p_mesh;
    // Re-upload 4D data to GPU buffers
    Slicer4D::get_singleton()->mark_dirty(this);
}
```

#### 4.9.6 Singleton Lifecycle

Register `Slicer4D` at `MODULE_INITIALIZATION_LEVEL_SERVERS` since it manages GPU resources. Create it in `initialize_4d_module()`, destroy it in `uninitialize_4d_module()`.

```cpp
void initialize_4d_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
        Slicer4D *slicer = memnew(Slicer4D);
        // slicer sets its own singleton pointer in constructor
    }
}
void uninitialize_4d_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
        memdelete(Slicer4D::get_singleton());
    }
}
```

### 4.10 Handling the 3D Viewport

The sliced 3D meshes are standard RenderingServer instances in the current `World3D`. They appear in the normal 3D viewport alongside any regular 3D nodes.

This means:
- Regular Godot lighting (3D lights) illuminates the sliced meshes.
- Regular Godot cameras see the sliced meshes.
- You do NOT need a custom rendering backend.

The Camera4D itself does not replace Camera3D. Instead, Camera4D computes the slice plane, and a companion Camera3D (either explicit or auto-created by Camera4D) handles the actual 3D rendering. Camera4D should create and manage an internal Camera3D that mirrors the 3D components of the 4D camera's orientation.

**WARNING:** Do NOT try to implement a custom renderer or bypass Godot's RenderingServer. The entire point is to slice 4D to 3D and then let Godot render the 3D result normally. Stay within the existing rendering pipeline.

---

## Stage 5: PhysicsServer4D (Fork of Jolt Physics, Generalized to 4D)

The PhysicsServer4D is a direct copy of Godot's Jolt physics module (`modules/jolt_physics/`) with the Jolt Physics library itself forked and modified to work in 4 dimensions. Everything that is not dimension-dependent stays identical. This stage describes exactly what needs to change and what stays the same.

### 5.1 Study Material

**You MUST read all of these before writing any code:**

- Godot's Jolt physics module: `https://github.com/godotengine/godot/tree/master/modules/jolt_physics` - This is the Godot-side wrapper. Study every file.
- The Jolt Physics library: `https://github.com/jrouwe/JoltPhysics` - This is the actual engine. Study the `Jolt/` source directory.
- Jolt architecture documentation: `https://github.com/jrouwe/JoltPhysics/blob/master/Docs/Architecture.md`
- GDC talk slides: "Architecting Jolt Physics for Horizon Forbidden West" - linked from the Jolt README.

### 5.2 Approach

**Two layers need modification:**

**Layer 1: The Jolt Physics library itself** (lives in `thirdparty/jolt_physics/` in Godot's tree, or `https://github.com/jrouwe/JoltPhysics` upstream). Fork this library. Go through the source and change the dimensional constants, vector types, and dimension-dependent algorithms. The result is "Jolt4D" - a 4D physics library with the same API shape as Jolt, but operating in 4 dimensions.

**Layer 2: Godot's Jolt module wrapper** (lives in `modules/jolt_physics/` in Godot's tree). Copy this module. Change all Godot 3D types (Vector3, Transform3D, Basis, AABB) to your 4D equivalents (Vector4D, Transform4D, Basis4D, AABB4D). Change the server class name from `JoltPhysicsServer3D` to `PhysicsServer4D`. Adapt the registration to work as a GDExtension rather than a built-in module.

### 5.3 Jolt Physics Library Structure and What Changes

The Jolt library source tree (from the SCsub file) organizes into these areas. For each, here is what changes for 4D:

#### 5.3.1 Jolt/Math/ - Vector and Matrix Types

**This is the foundation. Change here first.**

| Jolt 3D type | 4D change |
|---|---|
| `JPH::Vec3` (SIMD 3-component vector) | Becomes a 4-component vector. Jolt's Vec3 is internally a 4-wide SIMD register with the 4th lane unused as padding. For 4D, all 4 SIMD lanes carry real data, so basic vector operations (add, dot, multiply) actually map to SSE intrinsics equally well or better than 3D. |
| `JPH::Vec4` | In Jolt, Vec4 is used for homogeneous coordinates. In your fork, Vec4 becomes the base spatial vector (the thing that was Vec3). Introduce a new Vec5 if needed for homogeneous 4D coordinates. |
| `JPH::Mat44` (4x4 matrix) | Becomes a 5x5 matrix for full 4D affine transforms (4x4 rotation/scale + 4-component translation + 1 homogeneous row). Or use a 4x4 basis + Vec4 origin pair, matching how Jolt internally uses Mat44 (which is a 3x3 rotation + Vec3 translation packed into 4x4). |
| `JPH::Quat` (quaternion for 3D rotation) | 4D rotations are NOT representable by a single quaternion. Replace with a 4x4 orthogonal matrix or a pair of quaternions (isoclinic decomposition). Every place Jolt uses Quat for rotation, you use your 4D rotation representation instead. |
| `JPH::Float3` | Becomes Float4 (4 plain floats). |

**WARNING (possibly false, but worth keeping in mind):** Jolt uses SIMD heavily (SSE/AVX). The core vector operations (Vec4 add, dot, multiply, etc.) actually fit SSE 128-bit registers perfectly in 4D since 4 floats = 128 bits - better than 3D where the 4th lane was wasted padding. However, some derived types do not align as cleanly: the 6-component bivector (angular velocity/torque) needs padding to 8 floats for AVX or two SSE registers; a 5x5 affine matrix (if used) doesn't pack neatly. Avoid 5x5 matrices by using Basis4D (4x4) + Vec4 origin, which mirrors how Jolt packs its Mat44 internally. For the bivector, pad to 8 components and use AVX, or two SSE registers with 2 lanes unused. Overall, SIMD should remain viable for the hot paths. Get correctness first, then verify SIMD performance.

#### 5.3.2 Jolt/Physics/Body/ - Bodies and Motion Properties

| File | What changes |
|---|---|
| `Body.cpp` | Position becomes Vec4. Rotation becomes 4x4 matrix (or rotor pair) instead of Quat. |
| `BodyCreationSettings.cpp` | All position/rotation fields change dimension. |
| `BodyInterface.cpp` | All methods taking Vec3/Quat change to Vec4/Rotation4D. |
| `BodyManager.cpp` | No algorithmic changes. Just type substitutions. |
| `MassProperties.cpp` | **Significant change.** Inertia tensor becomes 6x6 instead of 3x3 (see 5.4). Mass computation formulas for primitive shapes change. |
| `MotionProperties.cpp` | Angular velocity becomes a 6-component bivector (see 5.4). Angular damping operates on 6 components. Allowed DOFs change from 6 (3 linear + 3 angular) to 10 (4 linear + 6 angular). |

#### 5.3.3 Jolt/Physics/Collision/ - Collision Detection

| Component | What changes |
|---|---|
| **BroadPhase (QuadTree/BruteForce)** | AABB becomes AABB4D (4 min + 4 max components). The QuadTree becomes an "OctTree" or more precisely a 4D spatial tree. The splitting logic (which axis to split on) now has 4 candidate axes instead of 3. The tree structure and traversal algorithms are otherwise identical. |
| **NarrowPhaseQuery** | Type substitutions (Vec3->Vec4). Ray becomes a 4D ray. |
| **GJK (inside ConvexShape collision)** | GJK works in any dimension. The simplex goes up to 5 vertices (pentachoron) instead of 4 (tetrahedron). The `ClosestPoint` and simplex management functions must handle the 5-vertex case. This is the main algorithmic change. |
| **EPA** | EPA faces become tetrahedra instead of triangles. The horizon-finding algorithm during expansion must work with tetrahedral faces of a 4D polytope instead of triangular faces of a 3D polytope. |
| **CollisionDispatch** | No algorithmic change. Just dispatches between shape type pairs. Add entries for any new shape types if needed. |
| **ManifoldBetweenTwoFaces** | In 3D, contact manifolds are generated on a 2D contact plane. In 4D, contact manifolds are generated on a 3D contact hyperplane. The clipping algorithm (Sutherland-Hodgman or similar) gains a dimension. |
| **CastConvexVsTriangles, CollideSphereVsTriangles, etc.** | In 4D, triangles become tetrahedra as the surface elements. These become CastConvexVsTetrahedra, CollideSphereVsTetrahedra, etc. The sweep and intersection math gains a dimension. |
| **TransformedShape** | Type substitutions. |
| **GroupFilter, CollisionGroup** | No change. These are purely logical filtering, dimension-independent. |
| **EstimateCollisionResponse** | Type substitutions for vectors. |

#### 5.3.4 Jolt/Physics/Collision/Shape/ - Shapes

Each shape needs its support function updated for 4D and its inertia tensor recomputed:

| Shape | 4D change |
|---|---|
| `SphereShape` | Trivial. Radius stays the same. Support function: `center + radius * dir.normalized()`. Inertia: `(2/5) * m * r^2` for all 6 rotation planes (uniform). |
| `BoxShape` | Half-extents become Vec4 (4 components). Support function: sign of each direction component * half-extent. Inertia: standard formula extended to 4D (look up hyperrectangle inertia tensor). |
| `CapsuleShape` | Height + radius, same concept. Support function gains a component. |
| `CylinderShape` | Height + radius. In 4D, a "cylinder" is a 3-sphere extruded along one axis. |
| `ConvexHullShape` | Vertices become Vec4. Convex hull computation in 4D (faces are tetrahedra). The convex hull builder (`ConvexHullBuilder.cpp`) must be generalized - this is a significant piece of work. Study incremental convex hull algorithms that work in arbitrary dimension. |
| `MeshShape` (ConcavePolygonShape equivalent) | Triangle mesh becomes tetrahedral mesh. BVH over tetrahedra instead of triangles. |
| `PlaneShape` | Normal becomes Vec4. Distance stays real_t. |
| `CompoundShape, StaticCompoundShape, MutableCompoundShape` | No algorithmic change. These just hold collections of sub-shapes with transforms. Type substitutions. |
| `HeightFieldShape` | **No direct 4D analog.** A heightfield maps 2D coordinates to height. A 4D analog would map 3D coordinates to "height" (a 3D heightfield). This is much more memory-intensive. Skip for initial implementation, or implement as a special case. |
| `ScaledShape, RotatedTranslatedShape, OffsetCenterOfMassShape, DecoratedShape` | Type substitutions. These are shape wrappers. |
| `EmptyShape` | No change. |
| `TriangleShape` | Becomes `TetrahedronShape` (4 vertices instead of 3). |
| `TaperedCapsuleShape, TaperedCylinderShape` | Generalize to 4D if needed, or skip for initial implementation. |

#### 5.3.5 Jolt/Physics/Constraints/ - Joints

| Constraint | 4D change |
|---|---|
| `PointConstraint` | Anchor point becomes Vec4. Same algorithm (spring pulling two points together). |
| `DistanceConstraint` | Same algorithm. Points are Vec4. |
| `FixedConstraint` | Locks all 10 DOFs instead of 6. |
| `HingeConstraint` | In 3D, rotation around one axis. In 4D, rotation in one plane. The constraint locks 5 angular DOFs and allows 1. The "axis" becomes a "plane" (specified by two basis vectors or a bivector). |
| `SliderConstraint` | Allows translation along one axis. Same concept in 4D. |
| `ConeConstraint` | Limits angle from a reference direction. In 4D the "cone" is a hypercone. The math generalizes. |
| `SixDOFConstraint` | Becomes a TenDOFConstraint (4 linear + 6 angular). Per-DOF limits and motors. |
| `SwingTwistConstraint` | The swing-twist decomposition changes in 4D. In 3D, swing = 2 DOF, twist = 1 DOF. In 4D, the decomposition is more complex (swing has more DOFs). Study carefully before implementing. |
| `GearConstraint, RackAndPinionConstraint, PulleyConstraint, PathConstraint` | These are higher-level mechanical constraints. Generalize the underlying point/axis math to 4D. |
| `SpringSettings, MotorSettings` | No change. These are scalar parameters. |

#### 5.3.6 Jolt/Physics/Character/ - Character Physics

| File | 4D change |
|---|---|
| `CharacterBase, Character, CharacterVirtual` | The character controller uses shape casts and ground detection. All positions/velocities become 4D. The "up" direction becomes a Vec4. Ground detection raycasts become 4D raycasts. The slide-along-surface algorithm works the same way with 4D normals. |

#### 5.3.7 Jolt/Physics/ - Top Level

| File | 4D change |
|---|---|
| `PhysicsSystem.cpp` | Type substitutions. The step loop logic (broad phase, narrow phase, solve, integrate) is unchanged. |
| `IslandBuilder.cpp` | No change. Graph algorithm, dimension-independent. |
| `LargeIslandSplitter.cpp` | No change. |
| `PhysicsUpdateContext.cpp` | Type substitutions. |

#### 5.3.8 Jolt/Geometry/ - Geometric Utilities

| File | 4D change |
|---|---|
| `ConvexHullBuilder.cpp` | **Major change.** Must compute convex hulls in 4D. Faces are tetrahedra. The incremental algorithm generalizes but the bookkeeping is more complex. |
| `ConvexHullBuilder2D.cpp` | Becomes ConvexHullBuilder3D (used as a sub-step in 4D hull building, same as 2D is used in 3D). |
| `OrientedBox.cpp` | Becomes oriented hyperbox (4D OBB). Basis4D + half-extents Vec4. |

#### 5.3.9 Jolt/AABBTree/ - Bounding Volume Hierarchy

Type substitutions (AABB -> AABB4D). The tree building algorithm is dimension-agnostic. Surface area heuristic for splitting becomes hyper-surface area heuristic (the "surface area" of a 4D AABB is the sum of the 3D "face" volumes of its 8 bounding hyperfaces).

#### 5.3.10 Jolt/Physics/SoftBody/, Jolt/Physics/Vehicle/, Jolt/Physics/Ragdoll/

**Skip these for initial implementation.** Soft bodies, vehicles, and ragdolls are complex subsystems. Get rigid body physics working first. These can be added later following the same fork-and-generalize approach.

#### 5.3.11 Jolt/Renderer/DebugRenderer

Type substitutions for debug line/triangle drawing. In 4D, debug primitives would need to go through the slicer to be visible, or be drawn in a simplified 3D projection. Skip or stub for initial implementation.

### 5.4 Key Mathematical Changes - Angular Quantities

This is the single most important conceptual change in the entire physics fork. In 3D, angular velocity/torque/momentum are pseudovectors (3 components). In 4D, they are **bivectors** (6 components).

**The 6 rotation planes:**
```
Index 0: XY plane
Index 1: XZ plane
Index 2: XW plane
Index 3: YZ plane
Index 4: YW plane
Index 5: ZW plane
```

**What changes:**
- `angular_velocity`: Vec3 (3 components) -> Bivector4D (6 components)
- `torque`: Vec3 -> Bivector4D
- `angular_momentum`: Vec3 -> Bivector4D
- `inertia_tensor`: 3x3 matrix -> 6x6 matrix (maps bivectors to bivectors)
- `inverse_inertia_tensor`: 3x3 -> 6x6
- **Cross product `r x F` (for torque from force)**: In 3D, `torque = r x F`. In 4D, there is no cross product. Instead, use the **wedge product**: `torque_bivector = r ^ F`, which produces a bivector with 6 components: `(r_i * F_j - r_j * F_i)` for each pair `(i,j)` with `i < j`.
- **Angular velocity to linear velocity at offset**: In 3D, `v = omega x r`. In 4D, this becomes the contraction of the angular velocity bivector with the position vector. For bivector `W` and vector `r`, the result is: `v_i = sum_j (W_ij * r_j)` where `W` is treated as an antisymmetric 4x4 matrix.
- **Rotation integration**: In 3D, `orientation += 0.5 * quat(0, omega) * orientation * dt`. In 4D, construct the antisymmetric 4x4 matrix `W` from the bivector, then: `basis_new = (I + W * dt) * basis_old`, followed by Gram-Schmidt re-orthonormalization. Or use matrix exponential for better accuracy.

**Inertia tensors for primitive shapes (4D):**

You will need to derive these. For shapes with symmetry:
- **Sphere**: Diagonal 6x6, all entries `(2/5) * m * r^2` (uniform because sphere is isotropic).
- **Box** with half-extents (a,b,c,d): Each diagonal entry of the 6x6 inertia tensor corresponds to a rotation plane. For the IJ plane, the moment of inertia is `m * (h_i^2 + h_j^2) / 3` where h_i, h_j are the half-extents along axes I and J. Off-diagonal entries are zero for an axis-aligned box.
- For other shapes, compute numerically or derive from the shape geometry.

### 5.5 Godot Module Wrapper - File-by-File Changes

The Godot Jolt module (`modules/jolt_physics/`) contains these directories:

```
modules/jolt_physics/
    register_types.cpp      # Module registration
    jolt_physics_server_3d.h/.cpp  # Main server class (implements PhysicsServer3D)
    objects/
        jolt_body_3d.h/.cpp
        jolt_area_3d.h/.cpp
        jolt_soft_body_3d.h/.cpp
        jolt_shaped_object_3d.h/.cpp
        jolt_physics_direct_body_state_3d.h/.cpp
    shapes/
        jolt_shape_3d.h/.cpp   # Base shape + all shape type implementations
    spaces/
        jolt_space_3d.h/.cpp   # Physics space (world)
    joints/
        jolt_joint_3d.h/.cpp
        jolt_hinge_joint_3d.h/.cpp
        jolt_slider_joint_3d.h/.cpp
        jolt_cone_twist_joint_3d.h/.cpp
        jolt_generic_6dof_joint_3d.h/.cpp
        jolt_pin_joint_3d.h/.cpp
    misc/
        jolt_type_conversions.h  # to_godot() / to_jolt() conversion functions
        ...
```

**For each file, create a 4D copy:**

| Original | Your copy | Changes |
|---|---|---|
| `jolt_physics_server_3d.h/.cpp` | `physics_server_4d.h/.cpp` | Replace all Vector3->Vector4D, Transform3D->Transform4D, Basis->Basis4D, AABB->AABB4D. Replace PhysicsServer3D base class usage with your own PhysicsServer4D. Remove soft body methods (skip for now). |
| `jolt_body_3d.h/.cpp` | `jolt_body_4d.h/.cpp` | All position/velocity/force types change. Angular velocity becomes Bivector4D. Inertia becomes 6x6. Allowed DOFs becomes 10. |
| `jolt_area_3d.h/.cpp` | `jolt_area_4d.h/.cpp` | Type substitutions. Overlap detection uses 4D shapes. |
| `jolt_shaped_object_3d.h/.cpp` | `jolt_shaped_object_4d.h/.cpp` | Scale validation with 4 axes. Compound shape building with 4D transforms. |
| `jolt_shape_3d.h/.cpp` | `jolt_shape_4d.h/.cpp` | Each shape type's data and support function changes to 4D. |
| `jolt_space_3d.h/.cpp` | `jolt_space_4d.h/.cpp` | Gravity becomes Vec4. Broad phase layer mapping unchanged. Step logic unchanged. |
| `jolt_physics_direct_body_state_3d` | `physics_direct_body_state_4d` | All vector types change. Angular quantities become bivectors. |
| `jolt_type_conversions.h` | `type_conversions_4d.h` | New conversion functions between your 4D math types and the forked Jolt4D types. |
| Joint files | Joint 4D files | Per-joint changes as described in 5.3.5. |

### 5.6 Registration as GDExtension

Unlike Godot's built-in Jolt module which registers via `register_types.cpp` as an engine module, yours registers as a GDExtension.

**PhysicsServer4D is NOT a PhysicsServer3D replacement.** It does not implement `PhysicsServer3DExtension`. It is its own independent singleton, registered with a unique name.

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
    ClassDB::register_class<PhysicsServer4D>();
    ClassDB::register_class<PhysicsDirectBodyState4D>();
    ClassDB::register_class<PhysicsDirectSpaceState4D>();

    PhysicsServer4D *server = memnew(PhysicsServer4D);
    Engine::get_singleton()->register_singleton("PhysicsServer4D", server);
}
```

The physics step must be called manually since you're not plugged into Godot's physics pipeline. Hook into `_physics_process` via a singleton node or use `MainLoop` notification:

```cpp
void PhysicsServer4D::step(real_t p_delta) {
    for (auto &space : active_spaces) {
        space->step(p_delta);
    }
}
```

Call `PhysicsServer4D::step()` from a hidden autoload node's `_physics_process()`, or from each `PhysicsBody4D` node's `_physics_process` (less ideal). The autoload approach matches how an independent physics system would integrate.

### 5.7 What to Skip for Initial Implementation

To make this tractable, skip these on first pass:

- **Soft bodies** (JoltSoftBody) - complex, not in the node list
- **Vehicles** (VehicleConstraint etc.) - specialized
- **Ragdolls** - can be built from joints later
- **HeightFieldShape** - no clean 4D analog
- **TaperedCapsuleShape, TaperedCylinderShape** - minor variants
- **Debug renderer** - nice to have, not essential
- **Thread safety / running on separate thread** - get single-threaded working first

Everything else must be implemented.

### 5.8 Testing Strategy

1. Test forked Jolt4D math types in isolation: Vec4 operations, matrix multiplication, bivector wedge product.
2. Test support functions for each shape: known directions should return known extreme points.
3. Test GJK with the 5-vertex simplex: overlapping and separated sphere-sphere, box-box, sphere-box pairs.
4. Test EPA: two overlapping boxes, verify penetration depth matches analytical expectation.
5. Test full pipeline: single rigid sphere falling onto a static box floor.
6. Test character controller: basic movement and ground detection.
7. Test joints: two bodies connected by a point constraint.
8. Compare behavior qualitatively with the 3D Jolt server for equivalent setups.

---

## Stage 6: Physics Nodes (CollisionObject4D, PhysicsBody4D Hierarchy, Area4D)

These are the scene-tree nodes that users interact with. They are thin wrappers that translate between the node system (transforms, signals, inspector properties) and PhysicsServer4D's RID-based API. Study the 3D analogs in Godot source and replicate their logic with 4D types.

### 6.1 Study Material

Study these source files in detail - your 4D nodes must mirror their behavior:

- `CollisionObject3D`: https://github.com/godotengine/godot/blob/master/scene/3d/physics/collision_object_3d.h and `.cpp`
- `PhysicsBody3D`: https://github.com/godotengine/godot/blob/master/scene/3d/physics/physics_body_3d.h and `.cpp`
- `StaticBody3D`, `AnimatableBody3D`, `CharacterBody3D`, `RigidBody3D`: all in the same directory
- `Area3D`: https://github.com/godotengine/godot/blob/master/scene/3d/physics/area_3d.h and `.cpp`

### 6.2 CollisionObject4D

Abstract base class. Inherits from `Node4D`. Analog of `CollisionObject3D`.

```
File: src/nodes/physics/collision_object_4d.h
File: src/nodes/physics/collision_object_4d.cpp
```

**Core responsibility:** Owns a body or area RID on PhysicsServer4D and manages collision shapes attached to it.

**What it does (copy from CollisionObject3D):**

- On `NOTIFICATION_ENTER_TREE`: creates its RID on PhysicsServer4D (body or area, depending on subclass), sets it into the active physics space, syncs the transform.
- On `NOTIFICATION_EXIT_TREE`: removes the body/area from the physics space.
- On `NOTIFICATION_TRANSFORM_4D_CHANGED`: pushes the new Transform4D to PhysicsServer4D via `body_set_transform()` or `area_set_transform()`.
- Manages a list of "shape owners" - child CollisionShape4D and CollisionPolygon4D nodes register their shapes here. Study how `CollisionObject3D::shape_owner_*` methods work and replicate.

**Properties (from CollisionObject3D):**

| Property | Type | Description |
|---|---|---|
| `collision_layer` | `uint32_t` | Which layers this object is on |
| `collision_mask` | `uint32_t` | Which layers this object scans |
| `collision_priority` | `real_t` | Priority for collision solving |
| `disable_mode` | `DisableMode` enum | What happens when node is disabled (REMOVE, MAKE_STATIC, KEEP_ACTIVE) |
| `input_ray_pickable` | `bool` | Whether this can be picked by input rays |

**Methods:**

| Method | Description |
|---|---|
| `_create_physics_body()` | Pure virtual. Subclasses override to call `PhysicsServer4D::body_create()` or `area_create()`. |
| `shape_owner_create(owner_id)` | Create a shape owner slot. |
| `shape_owner_add_shape(owner_id, Ref<Shape4D>)` | Add a shape to the body via PhysicsServer4D. |
| `shape_owner_set_transform(owner_id, Transform4D)` | Set the local transform of a shape within the body. |
| `shape_owner_set_disabled(owner_id, bool)` | Enable/disable a shape. |
| `get_rid()` | Returns the PhysicsServer4D RID. |

**Signals (from CollisionObject3D):**
- `input_event(camera, event, position, normal, shape_idx)`
- `mouse_entered()`, `mouse_exited()`

### 6.3 PhysicsBody4D

Abstract base class. Inherits from `CollisionObject4D`. Analog of `PhysicsBody3D`.

```
File: src/nodes/physics/physics_body_4d.h
File: src/nodes/physics/physics_body_4d.cpp
```

**What it adds over CollisionObject4D:**

- Calls `PhysicsServer4D::body_create()` in `_create_physics_body()`.
- Provides `move_and_collide()` - the fundamental collision-aware movement method. Study `PhysicsBody3D::move_and_collide()` carefully. It calls `PhysicsServer4D::body_test_motion()` and returns a `KinematicCollision4D` result.
- Provides `test_move()` - test if a motion would result in collision without actually moving.
- Manages axis lock flags (4D has 4 linear axes and 6 rotation planes to lock).

**Properties:**

| Property | Type | Description |
|---|---|---|
| `axis_lock_linear_x/y/z/w` | `bool` | Lock linear motion on each axis |
| `axis_lock_angular_xy/xz/xw/yz/yw/zw` | `bool` | Lock rotation in each plane |

**NOTE:** In 3D, there are 3 linear locks and 3 angular locks. In 4D, there are 4 linear locks and 6 angular locks, for 10 total DOFs. The angular locks correspond to the 6 rotation planes, not axes.

**KinematicCollision4D:**

Analog of `KinematicCollision3D`. A RefCounted class returned by `move_and_collide()`. Study `KinematicCollision3D` for the API:

- `get_position()` -> Vector4D (collision point)
- `get_normal()` -> Vector4D (collision normal)
- `get_travel()` -> Vector4D (how far the body actually moved)
- `get_remainder()` -> Vector4D (remaining motion after collision)
- `get_depth()` -> real_t
- `get_collider()` -> Object*
- `get_collider_id()` -> ObjectID
- `get_collider_rid()` -> RID
- `get_collider_shape_index()` -> int
- `get_collider_velocity()` -> Vector4D
- `get_angle(up_direction)` -> real_t (angle between collision normal and up)

### 6.4 StaticBody4D

Concrete class. Inherits from `PhysicsBody4D`. Analog of `StaticBody3D`.

```
File: src/nodes/physics/static_body_4d.h
File: src/nodes/physics/static_body_4d.cpp
```

A non-moving body. Sets body mode to `STATIC` on PhysicsServer4D.

**Properties (from StaticBody3D):**

| Property | Type | Description |
|---|---|---|
| `physics_material_override` | `Ref<PhysicsMaterial>` | Friction and bounce (use Godot's built-in PhysicsMaterial - it's scalar, dimension-independent) |
| `constant_linear_velocity` | `Vector4D` | Constant linear velocity for conveyor-belt behavior |
| `constant_angular_velocity` | `Bivector4D` | Constant angular velocity. **NOTE:** In 3D this is a Vector3 (axis-angle encoding where direction = axis, magnitude = angular speed). In 4D, angular velocity is a bivector with 6 components (one per rotation plane). Use Bivector4D for correctness. |

**Implementation:** Nearly trivial. On enter tree, create body, set mode STATIC. Apply physics material properties. Sync transform when it changes.

### 6.5 AnimatableBody4D

Concrete class. Inherits from `StaticBody4D`. Analog of `AnimatableBody3D`.

```
File: src/nodes/physics/animatable_body_4d.h
File: src/nodes/physics/animatable_body_4d.cpp
```

A static body that can be moved via animation and will push other bodies. Sets body mode to `KINEMATIC` on PhysicsServer4D.

**Properties (from AnimatableBody3D):**
- `sync_to_physics` (bool) - Whether to sync the node's transform from the physics server each frame.

**Behavior:** Study `AnimatableBody3D` source. The key behavior is that when the node's transform changes (via animation), it calls `PhysicsServer4D::body_set_state(body, BODY_STATE_TRANSFORM, new_transform)`, and the physics server handles pushing other bodies out of the way during the next step. When `sync_to_physics` is true, the node reads its transform back from the physics server after the step.

### 6.6 RigidBody4D

Concrete class. Inherits from `PhysicsBody4D`. Analog of `RigidBody3D`.

```
File: src/nodes/physics/rigid_body_4d.h
File: src/nodes/physics/rigid_body_4d.cpp
```

A fully simulated dynamic body. Sets body mode to `RIGID` on PhysicsServer4D.

Study `RigidBody3D` source thoroughly - it has the most complex interaction with the physics server of any body type.

**Properties (from RigidBody3D):**

| Property | Type | Description |
|---|---|---|
| `mass` | `real_t` | Body mass (default 1.0) |
| `inertia` | `Vector4D` | Custom inertia override. In 3D this is Vector3 (diagonal of 3x3 inertia). In 4D, for full generality, this should be a 6-component value (diagonal of 6x6). For simplicity, expose it as a PackedFloat64Array of 6 elements, or create a Bivector4D property. If zero/empty, the server computes inertia from shapes. |
| `center_of_mass_mode` | `CenterOfMassMode` | AUTO or CUSTOM |
| `center_of_mass` | `Vector4D` | Custom center of mass |
| `physics_material_override` | `Ref<PhysicsMaterial>` | Friction and bounce |
| `gravity_scale` | `real_t` | Multiplier for gravity |
| `linear_damp_mode` | `DampMode` | COMBINE or REPLACE |
| `linear_damp` | `real_t` | Linear damping |
| `angular_damp_mode` | `DampMode` | COMBINE or REPLACE |
| `angular_damp` | `real_t` | Angular damping |
| `linear_velocity` | `Vector4D` | Current linear velocity |
| `angular_velocity` | `Bivector4D` or `PackedFloat64Array` | Current angular velocity (6 components) |
| `constant_force` | `Vector4D` | Constant applied force |
| `constant_torque` | `Bivector4D` or `PackedFloat64Array` | Constant applied torque (6 components) |
| `freeze` | `bool` | Whether body is frozen |
| `freeze_mode` | `FreezeMode` | STATIC or KINEMATIC when frozen |
| `can_sleep` | `bool` | Whether body can sleep |
| `sleeping` | `bool` | Whether body is currently sleeping |
| `continuous_cd` | `bool` | Enable continuous collision detection |
| `max_contacts_reported` | `int` | Max contacts tracked |
| `contact_monitor` | `bool` | Whether to emit contact signals |
| `custom_integrator` | `bool` | Whether to use custom force integration |
| `lock_rotation` | `bool` | If true, the body cannot rotate at all. Gravity and forces only apply linear movement. This is a convenience property equivalent to locking all 6 angular DOFs. |

**Virtual method (from RigidBody3D):**
- `_integrate_forces(PhysicsDirectBodyState4D *state)` - Called during physics processing when `custom_integrator` is true (or always, before standard integration). Override this in GDScript/C++ to do custom force integration. Expose as virtual via `GDVIRTUAL_BIND`.

**Signals (from RigidBody3D):**
- `body_entered(Node body)`
- `body_exited(Node body)`
- `body_shape_entered(body_rid, body, body_shape_index, local_shape_index)`
- `body_shape_exited(body_rid, body, body_shape_index, local_shape_index)`
- `sleeping_state_changed()`

**Methods:**
- `apply_force(force: Vector4D, position: Vector4D)`
- `apply_central_force(force: Vector4D)`
- `apply_impulse(impulse: Vector4D, position: Vector4D)`
- `apply_central_impulse(impulse: Vector4D)`
- `apply_torque(torque: Bivector4D)`
- `apply_torque_impulse(torque: Bivector4D)`
- `get_contact_count()`, `get_contact_body(idx)`, etc.

**Key implementation detail - state synchronization:**

Study how `RigidBody3D` uses `_body_state_changed()` callback. The physics server calls this after each step to sync the body's position back to the node:

```cpp
void RigidBody4D::_body_state_changed(PhysicsDirectBodyState4D *p_state) {
    set_transform_4d(p_state->get_transform());
    linear_velocity = p_state->get_linear_velocity();
    angular_velocity = p_state->get_angular_velocity();
    // ... etc
}
```

This callback is registered with PhysicsServer4D via `body_set_state_sync_callback()`. Make sure to replicate this pattern exactly.

**WARNING:** The node's transform is set FROM the physics server, not the other way around. Do NOT push the node's transform to the server each frame for a RigidBody - that would fight the simulation. The server is authoritative. The node only pushes its transform when first entering the tree or when `set_transform_4d()` is called explicitly by user code.

### 6.7 CharacterBody4D

Concrete class. Inherits from `PhysicsBody4D`. Analog of `CharacterBody3D`.

```
File: src/nodes/physics/character_body_4d.h
File: src/nodes/physics/character_body_4d.cpp
```

A kinematic body with built-in `move_and_slide()` logic for character controllers. Sets body mode to `KINEMATIC` on PhysicsServer4D.

Study `CharacterBody3D` source very carefully - it has the most complex movement logic of any node.

**Properties (from CharacterBody3D):**

| Property | Type | Description |
|---|---|---|
| `motion_mode` | `MotionMode` | GROUNDED or FLOATING |
| `up_direction` | `Vector4D` | Which direction is "up" (default: (0,1,0,0)) |
| `slide_on_ceiling` | `bool` | Whether to slide on ceiling |
| `wall_min_slide_angle` | `real_t` | Minimum angle for wall sliding |
| `floor_stop_on_slope` | `bool` | Stop on slopes when not moving |
| `floor_constant_speed` | `bool` | Maintain constant speed on slopes |
| `floor_block_on_wall` | `bool` | Block on walls when on floor |
| `floor_max_angle` | `real_t` | Maximum floor angle |
| `floor_snap_length` | `real_t` | Snap distance to floor |
| `platform_on_leave` | `PlatformOnLeave` | Behavior when leaving a moving platform |
| `platform_floor_layers` | `uint32_t` | Collision layers for floor platforms |
| `platform_wall_layers` | `uint32_t` | Collision layers for wall platforms |
| `max_slides` | `int` | Maximum slide iterations |
| `velocity` | `Vector4D` | Current velocity |
| `safe_margin` | `real_t` | Collision margin |

**Methods (from CharacterBody3D):**
- `move_and_slide()` - The main method. Moves the body by `velocity * delta`, sliding along surfaces.
- `is_on_floor()`, `is_on_wall()`, `is_on_ceiling()` - State queries.
- `get_floor_normal()` -> Vector4D
- `get_wall_normal()` -> Vector4D
- `get_last_motion()` -> Vector4D
- `get_platform_velocity()` -> Vector4D
- `get_slide_collision(idx)` -> KinematicCollision4D
- `get_slide_collision_count()` -> int

**The `move_and_slide()` algorithm (from CharacterBody3D):**

This is a multi-step collision resolution loop. Copy the logic from `CharacterBody3D::move_and_slide()` exactly, changing only the types. The algorithm:

1. Apply floor snap if grounded.
2. Move by velocity * delta using `move_and_collide()`.
3. If collision: determine if floor, wall, or ceiling based on angle between collision normal and `up_direction`.
4. Slide the remaining velocity along the collision surface.
5. Repeat up to `max_slides` times.
6. Update `is_on_floor/wall/ceiling` state.

In 4D, the "floor/wall/ceiling" classification works identically - you compare the dot product of the collision normal with `up_direction`. A surface is "floor" if `normal.dot(up_direction) > cos(floor_max_angle)`, "ceiling" if `normal.dot(up_direction) < -cos(floor_max_angle)`, "wall" otherwise. The math is dimension-agnostic.

### 6.8 Area4D

Concrete class. Inherits from `CollisionObject4D` (NOT from PhysicsBody4D). Analog of `Area3D`.

```
File: src/nodes/physics/area_4d.h
File: src/nodes/physics/area_4d.cpp
```

A region that detects overlapping bodies and can override physics parameters (gravity, damping) for bodies inside it.

**Creates an area (not a body) on PhysicsServer4D:**
```cpp
void Area4D::_create_physics_body() {
    rid = PhysicsServer4D::get_singleton()->area_create();
}
```

**Properties (from Area3D):**

| Property | Type | Description |
|---|---|---|
| `monitoring` | `bool` | Whether the area detects bodies entering/exiting |
| `monitorable` | `bool` | Whether other areas can detect this area |
| `priority` | `int` | Override priority when areas overlap |
| `gravity_space_override` | `SpaceOverride` | How this area's gravity interacts with the space default (DISABLED, COMBINE, COMBINE_REPLACE, REPLACE, REPLACE_COMBINE) |
| `gravity_point` | `bool` | Whether gravity points toward a center |
| `gravity_point_center` | `Vector4D` | Center of point gravity |
| `gravity_point_unit_distance` | `real_t` | Distance for unit gravity strength |
| `gravity_direction` | `Vector4D` | Direction of directional gravity |
| `gravity` | `real_t` | Gravity strength |
| `linear_damp_space_override` | `SpaceOverride` | Linear damp override mode |
| `linear_damp` | `real_t` | Linear damp value |
| `angular_damp_space_override` | `SpaceOverride` | Angular damp override mode |
| `angular_damp` | `real_t` | Angular damp value |
| `audio_bus_override` | `bool` | Whether to override audio bus |
| `audio_bus_name` | `StringName` | Audio bus name |

**Signals (from Area3D):**
- `body_entered(Node body)`
- `body_exited(Node body)`
- `body_shape_entered(body_rid, body, body_shape_index, local_shape_index)`
- `body_shape_exited(body_rid, body, body_shape_index, local_shape_index)`
- `area_entered(Area4D area)`
- `area_exited(Area4D area)`
- `area_shape_entered(area_rid, area, area_shape_index, local_shape_index)`
- `area_shape_exited(area_rid, area, area_shape_index, local_shape_index)`

**Implementation:** Study `Area3D` source. The monitoring callbacks are registered with PhysicsServer4D via `area_set_monitor_callback()` and `area_set_area_monitor_callback()`. The area receives callbacks from the physics server when bodies enter/exit and translates these into node-level signals.

**NOTE - skipped Area3D properties:** Area3D also has `wind_force_magnitude`, `wind_attenuation_factor`, `wind_source_path` (only affects SoftBody3D, which we are skipping), and `reverb_bus_enabled`, `reverb_bus_name`, `reverb_bus_amount`, `reverb_bus_uniformity` (reverb bus properties for spatial audio). Skip all of these for the initial implementation. They can be added later. Also Area3D has `has_overlapping_areas()`, `has_overlapping_bodies()`, `get_overlapping_areas()`, `get_overlapping_bodies()`, `overlaps_area()`, `overlaps_body()` methods - include these as they delegate to PhysicsServer4D.

### 6.9 Joint Nodes

Joints constrain the relative motion of two physics bodies. In Godot 3D, joints inherit from `Joint3D`, which inherits from `Node3D`. Your 4D joints inherit from `Joint4D`, which inherits from `Node4D`.

Study the joint source files:
- `Joint3D`: https://github.com/godotengine/godot/blob/master/scene/3d/physics/joints/joint_3d.h and `.cpp`
- Each specific joint type in the same directory

#### 6.9.1 The Joint Hierarchy

```
Node4D
  +-- Joint4D (abstract base)
        +-- PinJoint4D
        +-- HingeJoint4D
        +-- SliderJoint4D
        +-- ConeTwistJoint4D
        +-- Generic10DOFJoint4D (was Generic6DOFJoint3D)
```

Add this to the full inheritance tree from Stage 2 - it was missing from the original list. Joint4D is a sibling of CollisionObject4D, VisualInstance4D, Camera4D, etc. under Node4D.

#### 6.9.2 Joint4D (Abstract Base)

```
File: src/nodes/physics/joints/joint_4d.h
File: src/nodes/physics/joints/joint_4d.cpp
```

Analog of `Joint3D`. Manages the PhysicsServer4D joint RID and the two connected bodies.

**Properties (from Joint3D):**

| Property | Type | Description |
|---|---|---|
| `node_a` | `NodePath` | Path to the first body (PhysicsBody4D) |
| `node_b` | `NodePath` | Path to the second body (PhysicsBody4D) |
| `solver_priority` | `int` | Solver priority |
| `exclude_nodes_from_collision` | `bool` | Whether to disable collision between connected bodies |

**Behavior:** Study `Joint3D` source. On enter tree, it resolves node_a and node_b to PhysicsBody4D pointers, gets their PhysicsServer4D body RIDs, and creates the joint on the server. When the joint node's transform changes, it updates the joint frames (reference transforms for each body).

#### 6.9.3 PinJoint4D

Analog of `PinJoint3D`. Connects two bodies at a point, allowing free rotation around that point.

**Properties (from PinJoint3D):**

| Property | Type | Description |
|---|---|---|
| `damping` | `real_t` | Damping of the pin spring |
| `impulse_clamp` | `real_t` | Maximum impulse |
| `bias` | `real_t` | Bias for position correction |

Dimension-independent. The pin point is defined by the joint node's position in 4D.

#### 6.9.4 HingeJoint4D

Analog of `HingeJoint3D`. In 3D, a hinge allows rotation around a single axis. In 4D, the analog allows rotation in a single plane.

**Properties (from HingeJoint3D):**

| Property | Type | Description |
|---|---|---|
| `angular_limit/enabled` | `bool` | Whether angular limits are active |
| `angular_limit/upper` | `real_t` | Upper angle limit |
| `angular_limit/lower` | `real_t` | Lower angle limit |
| `angular_limit/bias` | `real_t` | Bias |
| `angular_limit/softness` | `real_t` | Softness |
| `angular_limit/relaxation` | `real_t` | Relaxation |
| `motor/enabled` | `bool` | Whether the motor is active |
| `motor/target_velocity` | `real_t` | Target angular velocity |
| `motor/max_impulse` | `real_t` | Maximum motor impulse |

**4D note:** The "axis" of the hinge in 3D becomes the "plane" of the hinge in 4D. The joint node's orientation defines which rotation plane is free. Specifically, two of the joint's local basis vectors define the rotation plane; the other two axes are locked. The angle limits and motor work identically - they constrain/drive rotation within that single plane.

#### 6.9.5 SliderJoint4D

Analog of `SliderJoint3D`. Allows linear motion along one axis and rotation around that axis.

**Properties:** Copy from `SliderJoint3D` - it has numerous parameters for linear/angular limits and motor.

**4D note:** Linear sliding along one axis is the same in any dimension. The rotational freedom around that axis in 3D becomes rotational freedom in a plane in 4D. Use the same approach as HingeJoint4D for the rotational part.

#### 6.9.6 ConeTwistJoint4D

Analog of `ConeTwistJoint3D`. Allows rotation within a cone-shaped limit around a twist axis.

**Properties (from ConeTwistJoint3D):**

| Property | Type | Description |
|---|---|---|
| `swing_span` | `real_t` | Cone angle |
| `twist_span` | `real_t` | Twist angle |
| `bias` | `real_t` | Correction bias |
| `softness` | `real_t` | Constraint softness |
| `relaxation` | `real_t` | Relaxation |

**4D note:** In 3D, the "cone" restricts the direction of one axis (2 angular DOF) while "twist" constrains rotation around that axis (1 angular DOF). In 4D, the cone restricts the direction of one axis within a 3D subspace (3 angular DOF of swing type), and twist constrains rotation in the remaining planes. The generalization requires careful thought about which DOFs map to swing vs twist. Study the swing-twist decomposition in the context of SO(4) before implementing. For an initial implementation, you can implement a simpler version that limits the angle between corresponding basis vectors.

#### 6.9.7 Generic10DOFJoint4D

Analog of `Generic6DOFJoint3D`. In 3D, this provides per-DOF limits and motors for all 6 DOFs (3 linear + 3 angular). In 4D, it provides per-DOF control for all 10 DOFs (4 linear + 6 angular).

**Properties:** For each of the 10 DOFs, expose:
- `enabled` (bool)
- `upper_limit` (real_t)
- `lower_limit` (real_t)
- `motor_enabled` (bool)
- `motor_target_velocity` (real_t)
- `motor_max_force` (real_t)
- `spring_enabled` (bool)
- `spring_stiffness` (real_t)
- `spring_damping` (real_t)

Use the same property naming pattern as `Generic6DOFJoint3D` but extended for the additional DOFs. The linear DOFs are `linear_x`, `linear_y`, `linear_z`, `linear_w`. The angular DOFs are `angular_xy`, `angular_xz`, `angular_xw`, `angular_yz`, `angular_yw`, `angular_zw`.

**This is the most versatile joint.** If other joint types prove too complex to generalize to 4D initially, this joint can approximate their behavior through per-DOF configuration.

### 6.10 Registration

All physics node classes register at `MODULE_INITIALIZATION_LEVEL_SCENE`:

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    ClassDB::register_abstract_class<CollisionObject4D>();
    ClassDB::register_abstract_class<PhysicsBody4D>();
    ClassDB::register_class<StaticBody4D>();
    ClassDB::register_class<AnimatableBody4D>();
    ClassDB::register_class<CharacterBody4D>();
    ClassDB::register_class<RigidBody4D>();
    ClassDB::register_class<Area4D>();
    ClassDB::register_class<KinematicCollision4D>();
    ClassDB::register_abstract_class<Joint4D>();
    ClassDB::register_class<PinJoint4D>();
    ClassDB::register_class<HingeJoint4D>();
    ClassDB::register_class<SliderJoint4D>();
    ClassDB::register_class<ConeTwistJoint4D>();
    ClassDB::register_class<Generic10DOFJoint4D>();
}
```

`CollisionObject4D`, `PhysicsBody4D`, and `Joint4D` are abstract. The rest are concrete.

### 6.10 Key Implementation Pattern

All physics nodes follow the same pattern:

1. On `NOTIFICATION_ENTER_TREE`: create RID on PhysicsServer4D, set into space, sync transform, register shapes from child CollisionShape4D nodes.
2. On `NOTIFICATION_TRANSFORM_4D_CHANGED`: push Transform4D to PhysicsServer4D (except RigidBody4D during simulation - the server is authoritative).
3. On `NOTIFICATION_EXIT_TREE`: remove from space, free RID.
4. Properties map 1:1 to PhysicsServer4D setter calls (e.g., setting `mass` calls `PhysicsServer4D::body_set_param(rid, BODY_PARAM_MASS, mass)`).
5. Signals are driven by callbacks from PhysicsServer4D (body contact callbacks, area monitor callbacks).

Do NOT invent new patterns. Copy the 3D node source, change types, done.

---

## Stage 7: Collision Nodes (CollisionShape4D, CollisionPolygon4D, RayCast4D)

These are utility nodes that attach to physics bodies or perform spatial queries. They are among the simplest nodes to implement since they mostly just hold data and register it with a parent CollisionObject4D or with PhysicsServer4D.

### 7.1 Study Material

Consult these documentation pages and source files directly:

- CollisionShape3D docs: https://docs.godotengine.org/en/stable/classes/class_collisionshape3d.html
- CollisionShape3D source: https://github.com/godotengine/godot/blob/master/scene/3d/physics/collision_shape_3d.h and `.cpp`
- CollisionPolygon3D docs: https://docs.godotengine.org/en/stable/classes/class_collisionpolygon3d.html
- CollisionPolygon3D source: https://github.com/godotengine/godot/blob/master/scene/3d/physics/collision_polygon_3d.h and `.cpp`
- CollisionPolygon3D XML (canonical property list): https://github.com/godotengine/godot/blob/master/doc/classes/CollisionPolygon3D.xml
- RayCast3D docs: https://docs.godotengine.org/en/stable/classes/class_raycast3d.html
- RayCast3D source: https://github.com/godotengine/godot/blob/master/scene/3d/physics/ray_cast_3d.h and `.cpp`
- Collision shapes guide: https://docs.godotengine.org/en/stable/tutorials/physics/collision_shapes_3d.html

Read the source code for all three classes before implementing. They are relatively short files.

### 7.2 CollisionShape4D

Inherits from `Node4D`. Analog of `CollisionShape3D`.

```
File: src/nodes/collision/collision_shape_4d.h
File: src/nodes/collision/collision_shape_4d.cpp
```

**Purpose:** Provides a `Shape4D` resource to a parent `CollisionObject4D`. This is how you give collision geometry to physics bodies and areas. The node must be a direct child of a CollisionObject4D (or a node that acts as a shape owner).

**Properties (verified from CollisionShape3D docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `shape` | `Ref<Shape4D>` | null | The actual collision shape resource. Must be a Shape4D subclass (BoxShape4D, SphereShape4D, etc.). |
| `disabled` | `bool` | false | If true, this collision shape has no effect in the physics world. |
| `debug_color` | `Color` | Color(0,0,0,0) | Color used to display the shape in the editor and when debug collision shapes are visible. The default Color(0,0,0,0) is a placeholder - the actual default comes from `ProjectSettings.debug/shapes/collision/shape_color`. |
| `debug_fill` | `bool` | true | If true, the shape debug visualization shows a solid fill in addition to wireframe. |

**Methods (verified from CollisionShape3D docs):**

| Method | Description |
|---|---|
| `make_convex_from_siblings()` | Sets this shape to the convex hull of all sibling MeshInstance4D nodes' geometry. The 4D analog would compute a 4D convex hull from sibling MeshInstance4D mesh data. |

**Behavior (from source code study):**

1. On `NOTIFICATION_PARENTED`: check if the parent (or an ancestor) is a `CollisionObject4D`. If so, register as a shape owner by calling `parent->shape_owner_create()` and `parent->shape_owner_add_shape()`.
2. When `shape` property changes: update the shape on the parent CollisionObject4D via `shape_owner_set_shape()`.
3. When `disabled` changes: call `parent->shape_owner_set_disabled()`.
4. On `NOTIFICATION_UNPARENTED`: remove the shape from the parent CollisionObject4D.
5. On `NOTIFICATION_TRANSFORM_4D_CHANGED`: update the shape's local transform within the parent body via `shape_owner_set_transform()`. This is how you offset or rotate a shape relative to its body.

**WARNING from docs:** A non-uniformly scaled CollisionShape4D will likely not behave as expected. Keep its scale uniform and adjust the shape resource dimensions instead. Replicate this warning in the 4D node's documentation.

**Debug visualization:** In 3D, CollisionShape3D draws a wireframe of the shape in the editor viewport. For 4D, the debug visualization must go through the slicing pipeline - slice the shape with the current hyperplane and draw the 3D cross-section as a wireframe. This can be handled by registering with Slicer4D similarly to VisualInstance4D, or by generating a debug mesh on demand. Implement debug visualization after the core functionality works.

### 7.3 CollisionPolygon4D

Inherits from `Node4D`. Analog of `CollisionPolygon3D`.

```
File: src/nodes/collision/collision_polygon_4d.h
File: src/nodes/collision/collision_polygon_4d.cpp
```

**Purpose:** Provides a thickened polygon shape to a parent CollisionObject4D. In 3D, CollisionPolygon3D takes a 2D polygon (PackedVector2Array) and extrudes it along the Z axis by a given depth, creating a prism. In 4D, the analog takes a 3D polygon (a set of 3D vertices defining a polyhedron) and extrudes it along the W axis by a given depth, creating a 4D prism (a polychoron).

**Properties (verified from CollisionPolygon3D docs/XML):**

| Property | Type | Default | Description |
|---|---|---|---|
| `polygon` | `PackedVector3Array` | empty | Array of vertices defining the 3D polyhedron in the local XYZ space. In 3D, this is a PackedVector2Array defining a 2D polygon. In 4D, this becomes a PackedVector3Array defining a 3D polyhedron (the cross-section). |
| `depth` | `real_t` | 1.0 | The extrusion depth along the W axis. In 3D, this extrudes the 2D polygon along Z. In 4D, this extrudes the 3D polyhedron along W. The extrusion goes from `-depth/2` to `+depth/2` in local W. |
| `disabled` | `bool` | false | If true, this collision polygon has no effect. |
| `margin` | `real_t` | 0.04 | Collision margin for the generated shape. |
| `debug_color` | `Color` | Color(0,0,0,0) | Debug visualization color (same semantics as CollisionShape4D). |
| `debug_fill` | `bool` | true | Whether to show solid fill in debug visualization. |

**Behavior:**

1. When `polygon` or `depth` changes, internally generates a `ConvexPolygonShape4D` (if the polyhedron is convex) or `ConcavePolygonShape4D` (if concave) from the extruded geometry.
2. Registers the generated shape with the parent CollisionObject4D, same pattern as CollisionShape4D.
3. The polygon can be concave or convex (matching CollisionPolygon3D behavior).

**4D generalization note:** In 3D, defining a polygon by a 2D vertex array is simple - it's a flat shape. In 4D, defining a 3D polyhedron by a vertex array is more complex because you also need face/connectivity information to define the surface. Options:

- **Simple approach:** Treat `polygon` as the vertices of a convex hull and compute the convex hull automatically. This limits CollisionPolygon4D to convex polyhedra but is much simpler.
- **Full approach:** Also accept a face index array. More flexible but more complex API.

Start with the simple convex hull approach. It matches how most users use CollisionPolygon3D (with convex polygons).

### 7.4 RayCast4D

Inherits from `Node4D`. Analog of `RayCast3D`.

```
File: src/nodes/collision/ray_cast_4d.h
File: src/nodes/collision/ray_cast_4d.cpp
```

**Purpose:** Casts a ray in 4D space from its origin to `target_position` and reports the closest collision. The ray is updated every physics frame automatically. The node's global 4D position is the ray's starting point; `target_position` is relative to the node's local space.

**Properties (verified from RayCast3D docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `enabled` | `bool` | true | If true, collisions will be reported. The ray updates every physics frame when enabled. |
| `target_position` | `Vector4D` | Vector4D(0, -1, 0, 0) | The ray's destination point, relative to the node's position. In 3D, default is Vector3(0, -1, 0) (pointing down). |
| `collision_mask` | `uint32_t` | 1 | The ray's collision mask. Only objects in at least one collision layer enabled in the mask will be detected. |
| `exclude_parent` | `bool` | true | If true, collisions will be ignored for this RayCast4D's immediate parent. |
| `hit_from_inside` | `bool` | false | If true, the ray will detect collisions even if it starts inside a shape. |
| `hit_back_faces` | `bool` | true | If true, the ray will hit back faces of concave polygon shapes with back face collision enabled. |
| `collide_with_areas` | `bool` | false | If true, collisions with Area4D nodes will be reported. |
| `collide_with_bodies` | `bool` | true | If true, collisions with PhysicsBody4D nodes will be reported. |
| `debug_shape_custom_color` | `Color` | Color(0,0,0,0) | Custom color for the debug shape. If black (default), uses the project setting color. Highlighted at runtime when colliding. |
| `debug_shape_thickness` | `int` | 2 | If set to 1, a line is used as the debug shape. Otherwise, a truncated pyramid shape is used. |

**Methods (verified from RayCast3D docs):**

| Method | Return | Description |
|---|---|---|
| `is_colliding()` | `bool` | Whether the ray intersects any object. |
| `get_collider()` | `Object` | The physics body or area that was hit. |
| `get_collider_rid()` | `RID` | The RID of the collider on PhysicsServer4D. |
| `get_collider_shape()` | `int` | The shape index of the collider at the collision point. |
| `get_collision_point()` | `Vector4D` | The collision point in global 4D coordinates. |
| `get_collision_normal()` | `Vector4D` | The surface normal at the collision point. Returns Vector4D(0,0,0,0) if `hit_from_inside` is true and the ray starts inside a shape. |
| `get_collision_face_index()` | `int` | The face index at the collision point. Returns -1 if the intersected shape is not a ConcavePolygonShape4D. |
| `force_raycast_update()` | `void` | Forces the ray to update immediately rather than waiting for the next physics frame. Useful for multiple queries in one frame. |
| `add_exception(CollisionObject4D)` | `void` | Add a collision exception so the ray ignores a specific object. |
| `add_exception_rid(RID)` | `void` | Add a collision exception by RID. |
| `remove_exception(CollisionObject4D)` | `void` | Remove a collision exception. |
| `remove_exception_rid(RID)` | `void` | Remove a collision exception by RID. |
| `clear_exceptions()` | `void` | Remove all collision exceptions. |
| `set_collision_mask_value(layer_number, value)` | `void` | Enable or disable a specific collision mask layer (1-32). |
| `get_collision_mask_value(layer_number)` | `bool` | Check if a specific collision mask layer is enabled. |

**Behavior (from source study):**

1. In `_physics_process()` (when `enabled` is true): constructs a ray query and calls `PhysicsDirectSpaceState4D::intersect_ray()` from the node's global 4D position to the global position of `target_position`.
2. Stores the result (collider, point, normal, etc.) for retrieval via the getter methods.
3. Manages an exception list of RIDs to exclude from the query.
4. If `exclude_parent` is true, automatically adds the parent's body RID to the exception list.

**Implementation:** The actual raycasting is done by PhysicsServer4D via the `PhysicsDirectSpaceState4D::intersect_ray()` method (see Stage 5.9). RayCast4D is just a node-level convenience wrapper. The implementation is almost purely type substitutions from the 3D version.

**Debug visualization:** In 3D, RayCast3D draws a line (or truncated pyramid) from origin to target. In 4D, this line exists in 4D space and must be sliced to be visible. The sliced result is a point, a line segment, or nothing depending on how the 4D line intersects the view hyperplane. For debug drawing, compute the intersection of the 4D line segment with the slice plane analytically and draw the 3D result.

### 7.5 Registration

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    ClassDB::register_class<CollisionShape4D>();
    ClassDB::register_class<CollisionPolygon4D>();
    ClassDB::register_class<RayCast4D>();
}
```

All three are concrete classes.

### 7.6 Relationship to Parent Nodes

To be clear about how these nodes interact with the scene tree:

- `CollisionShape4D` and `CollisionPolygon4D` must be direct children of a `CollisionObject4D` (or a shape owner). They provide collision geometry to that body/area. They do NOT interact with PhysicsServer4D directly - they go through their parent CollisionObject4D's shape_owner interface.

- `RayCast4D` can be a child of any node. It queries PhysicsServer4D directly via the space's direct state. It does NOT need a CollisionObject4D parent.

This matches the 3D behavior exactly.

---

## Stage 8: Camera4D and Lighting

Camera4D is unique among the 4D nodes because it bridges the 4D world and the 3D rendering pipeline. It defines both the 4D slice hyperplane (which 3D cross-section of the 4D world to see) and the 3D camera that renders the sliced result. Light4D nodes define illumination in 4D space, which must also be sliced/projected into 3D for rendering.

### 8.1 Study Material

Consult these documentation pages:

- Camera3D: https://docs.godotengine.org/en/stable/classes/class_camera3d.html
- Light3D (base): https://docs.godotengine.org/en/stable/classes/class_light3d.html
- DirectionalLight3D: https://docs.godotengine.org/en/stable/classes/class_directionallight3d.html
- OmniLight3D: https://docs.godotengine.org/en/stable/classes/class_omnilight3d.html
- SpotLight3D: https://docs.godotengine.org/en/stable/classes/class_spotlight3d.html
- 3D lights and shadows tutorial: https://docs.godotengine.org/en/stable/tutorials/3d/lights_and_shadows.html

### 8.2 Camera4D

Inherits from `Node4D`. This is NOT a subclass of Camera3D. It owns an internal Camera3D that it manages.

```
File: src/nodes/camera_4d.h
File: src/nodes/camera_4d.cpp
```

**Dual role:**

1. **4D role:** Its 4D transform (from Node4D) defines the slice hyperplane and the coordinate frame for projecting sliced geometry into 3D. The camera's local W axis (column 3 of its Basis4D) is the hyperplane normal. The first three columns (X, Y, Z of Basis4D) provide the 3D coordinate axes within the slice. Moving the camera along its local W changes which "layer" of the 4D world is visible. Rotating in W-involving planes (XW, YW, ZW) tilts the slice.

2. **3D role:** Owns an internal `Camera3D` node (or uses the RenderingServer camera API directly) that performs the actual 3D rendering of the sliced geometry. All standard Camera3D properties (FOV, near/far, projection mode, etc.) apply to this internal camera.

**Properties from Camera3D (verified from docs):**

These are exposed on Camera4D and forwarded to the internal Camera3D:

| Property | Type | Default | Description |
|---|---|---|---|
| `projection` | `ProjectionType` enum | PERSPECTIVE | PROJECTION_PERSPECTIVE, PROJECTION_ORTHOGONAL, or PROJECTION_FRUSTUM. This controls how the 3D sliced scene is projected to 2D screen. |
| `fov` | `real_t` | 75.0 | Field of view in degrees. Only applicable in perspective mode. |
| `size` | `real_t` | 1.0 | Camera size (half-width or half-height). Only applicable in orthogonal and frustum modes. |
| `frustum_offset` | `Vector2` | (0,0) | Frustum offset for tilted frustum effects. Only for PROJECTION_FRUSTUM. |
| `near` | `real_t` | 0.05 | Near clip plane distance. |
| `far` | `real_t` | 4000.0 | Far clip plane distance. |
| `keep_aspect` | `KeepAspect` enum | KEEP_HEIGHT | Which axis to lock during aspect ratio changes. KEEP_WIDTH or KEEP_HEIGHT. |
| `current` | `bool` | false | If true, this is the active camera for its viewport. Only one Camera4D can be current per viewport. |
| `cull_mask` | `uint32_t` | 0xFFFFF (20 bits) | Which VisualInstance3D layers are rendered. Controls which sliced 3D geometry is visible. |
| `h_offset` | `real_t` | 0.0 | Horizontal viewport offset. |
| `v_offset` | `real_t` | 0.0 | Vertical viewport offset. |
| `doppler_tracking` | `DopplerTracking` enum | DISABLED | Doppler effect mode. DISABLED, IDLE_STEP, or PHYSICS_STEP. |
| `environment` | `Ref<Environment>` | null | The Environment resource for post-processing (tone mapping, fog, SSAO, etc.). Uses Godot's built-in Environment since post-processing applies to the 3D rendered result. |
| `attributes` | `Ref<CameraAttributes>` | null | The CameraAttributes resource for exposure and DOF. |

**4D-specific properties (not from Camera3D):**

| Property | Type | Default | Description |
|---|---|---|---|
| `slice_offset` | `real_t` | 0.0 | Additional offset along the camera's local W axis for the slice plane. Allows fine-tuning the slice position without moving the camera node. |

**Methods from Camera3D (verified from docs, adapted to 4D):**

| Method | Return | Description |
|---|---|---|
| `make_current()` | `void` | Makes this the active Camera4D for the viewport. |
| `clear_current(enable_next)` | `void` | Removes this camera as the current one. If `enable_next` is true, activates the next Camera4D in the tree. |
| `is_current()` | `bool` | Whether this is the current camera. |
| `get_camera_transform()` | `Transform3D` | Returns the internal Camera3D's transform (includes h_offset and v_offset adjustments). |
| `get_camera_projection()` | `Projection` | Returns the projection matrix used by the internal Camera3D. |
| `get_frustum()` | `Array[Plane]` | Returns the 3D frustum planes of the internal camera. |
| `project_ray_origin(screen_point)` | `Vector3` | Projects a 2D screen point to a 3D ray origin (in the sliced 3D space). |
| `project_ray_normal(screen_point)` | `Vector3` | Projects a 2D screen point to a 3D ray direction. |
| `project_position(screen_point, z_depth)` | `Vector3` | Projects a 2D screen point to a 3D position at the given depth. |
| `unproject_position(world_3d_point)` | `Vector2` | Projects a 3D world point to a 2D screen position. |
| `get_camera_rid()` | `RID` | Returns the internal Camera3D's RID from RenderingServer. |

**Slicing integration (from Stage 4):**

Each frame (in `_process` or `_notification(NOTIFICATION_PROCESS)`):

1. Compute the slice hyperplane from the camera's global 4D transform: the normal is the W column of the global Basis4D, the distance is the dot product of the W column with the global origin.
2. Compute the 3D basis within the slice: the first three columns of the global Basis4D.
3. Call `Slicer4D::get_singleton()->slice_all(hyperplane, basis_3d, origin_4d)`.
4. The Slicer4D produces 3D geometry already in the coordinate frame defined by the camera's 3D basis. The internal Camera3D is placed at the 3D origin (identity transform, or with h/v offset) since the sliced geometry is already in camera-relative 3D coordinates.

**Internal Camera3D management:**

Option A: Create an actual Camera3D node as a child, hidden from the user. Set it as the current viewport camera. Forward all property changes to it.

Option B: Use `RenderingServer::camera_create()` directly, without a Camera3D node. Set projection, transform, etc. via the RenderingServer API. This avoids having a hidden node in the tree but requires more manual management.

Option A is simpler and recommended. The internal Camera3D should not be visible in the editor's scene tree if possible (use `set_meta("_edit_lock_", true)` or similar).

### 8.3 Light4D (Abstract Base)

Inherits from `VisualInstance4D`. Analog of `Light3D`.

```
File: src/nodes/lights/light_4d.h
File: src/nodes/lights/light_4d.cpp
```

**How 4D lights work:**

Lights in 4D space illuminate 4D geometry. However, since the final rendering happens in 3D (after slicing), lights must be projected into the 3D slice. The approach:

1. Each Light4D node has a 4D position/direction.
2. When the scene is sliced, the light's 4D position is projected into the 3D slice coordinate system (same projection used for geometry: dot with the camera's 3D basis vectors).
3. An internal Light3D (or RenderingServer light RID) is created/updated with the projected 3D position and the light's properties (color, energy, etc.).
4. The 3D renderer handles the actual lighting and shadows using this projected light.

This means each Light4D owns an internal Light3D (DirectionalLight3D, OmniLight3D, or SpotLight3D depending on subclass) whose transform is updated each frame based on the 4D-to-3D projection.

**Properties from Light3D (verified from docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `light_color` | `Color` | Color(1,1,1,1) | Base color of emitted light. |
| `light_energy` | `real_t` | 1.0 | Energy multiplier. |
| `light_indirect_energy` | `real_t` | 1.0 | Multiplier for indirect light contribution (GI). |
| `light_volumetric_fog_energy` | `real_t` | 1.0 | Multiplier for volumetric fog contribution. |
| `light_bake_mode` | `BakeMode` enum | BAKE_DYNAMIC | BAKE_DISABLED, BAKE_STATIC, or BAKE_DYNAMIC. For 4D, baking is not applicable in the traditional sense (geometry changes with the slice). Default to BAKE_DISABLED or BAKE_DYNAMIC. |
| `light_cull_mask` | `uint32_t` | 0xFFFFFFFF | Which VisualInstance layers this light affects. |
| `light_specular` | `real_t` | 0.5 | Intensity of specular highlights. 0 = pure diffuse. |
| `light_temperature` | `real_t` | 6500.0 | Color temperature in Kelvin. |
| `shadow_enabled` | `bool` | false | Whether the light casts shadows. |
| `shadow_bias` | `real_t` | 0.1 | Shadow mapping bias. |
| `shadow_normal_bias` | `real_t` | 1.0 | Shadow normal bias. |
| `shadow_reverse_cull_face` | `bool` | false | Reverse cull face for shadow rendering. |
| `shadow_transmittance_bias` | `real_t` | 0.05 | Shadow transmittance bias. |
| `shadow_opacity` | `real_t` | 1.0 | Shadow opacity. |
| `shadow_blur` | `real_t` | 1.0 | Shadow blur amount. |
| `distance_fade_enabled` | `bool` | false | Enable distance-based fading (OmniLight/SpotLight only). |
| `distance_fade_begin` | `real_t` | 40.0 | Distance where fading starts. |
| `distance_fade_shadow` | `real_t` | 50.0 | Distance where shadow fading starts. |
| `distance_fade_length` | `real_t` | 10.0 | Distance over which fading occurs. |
| `editor_only` | `bool` | false | If true, the light is only visible in the editor. |
| `light_projector` | `Ref<Texture2D>` | null | Projector texture. |
| `light_size` | `real_t` | 0.0 | Light size for PCSS soft shadows (OmniLight/SpotLight only). |
| `light_angular_distance` | `real_t` | 0.0 | Angular size for PCSS (DirectionalLight only). |
| `light_negative` | `bool` | false | If true, light subtracts rather than adds. |
| `light_intensity_lumens` | `real_t` | | Intensity in lumens (when physical light units are enabled). |
| `light_intensity_lux` | `real_t` | | Intensity in lux (DirectionalLight, when physical units enabled). |

All of these are forwarded to the internal Light3D. Light4D is abstract.

### 8.4 DirectionalLight4D

Inherits from `Light4D`. Analog of `DirectionalLight3D`.

```
File: src/nodes/lights/directional_light_4d.h
File: src/nodes/lights/directional_light_4d.cpp
```

A directional light has no position, only a direction. In 4D, the direction is a Vector4D. When projecting to 3D, the 4D direction is projected into the 3D slice coordinate system, yielding a 3D direction for the internal DirectionalLight3D.

**Additional properties from DirectionalLight3D (verified from docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `directional_shadow_mode` | `ShadowMode` enum | PSSM_4_SPLITS | Shadow split mode: ORTHOGONAL, PSSM_2_SPLITS, or PSSM_4_SPLITS. |
| `directional_shadow_split_1` | `real_t` | 0.1 | First PSSM split distance (fraction of max distance). |
| `directional_shadow_split_2` | `real_t` | 0.2 | Second PSSM split distance. |
| `directional_shadow_split_3` | `real_t` | 0.5 | Third PSSM split distance. |
| `directional_shadow_blend_splits` | `bool` | false | Blend between shadow splits for smoother transitions. |
| `directional_shadow_fade_start` | `real_t` | 0.8 | Distance at which shadow starts fading (fraction of max). |
| `directional_shadow_max_distance` | `real_t` | 100.0 | Maximum shadow rendering distance. |
| `directional_shadow_pancake_size` | `real_t` | 20.0 | Shadow pancake size for directional shadow rendering. |
| `sky_mode` | `SkyMode` enum | LIGHT_AND_SKY | SKY_ONLY, LIGHT_ONLY, or LIGHT_AND_SKY. |

**4D note:** If the directional light's 4D direction is exactly along W (perpendicular to the slice), it has no component in the 3D slice and contributes no directional illumination to the sliced scene. This is physically correct - a light shining "through" the 4th dimension would illuminate all W layers equally but have no visible direction within any single slice.

### 8.5 OmniLight4D

Inherits from `Light4D`. Analog of `OmniLight3D`.

```
File: src/nodes/lights/omni_light_4d.h
File: src/nodes/lights/omni_light_4d.cpp
```

A point light emitting in all directions from a position in 4D space. When projected into 3D, its 4D position becomes a 3D position. The distance from the light's 4D position to the slice hyperplane affects the effective brightness (a light far from the slice in the W direction will appear dimmer or be beyond its range).

**Additional properties from OmniLight3D (verified from docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `omni_range` | `real_t` | 5.0 | Light range in world units. |
| `omni_attenuation` | `real_t` | 1.0 | Attenuation curve exponent. |
| `omni_shadow_mode` | `ShadowMode` enum | SHADOW_DUAL_PARABOLOID | SHADOW_DUAL_PARABOLOID or SHADOW_CUBE. |

**4D attenuation consideration:** The omni light's range in 4D is a 4D radius. If the light is at distance `d` along W from the slice plane, the effective 3D range within the slice is `sqrt(range^2 - d^2)` (by Pythagoras). If `d > range`, the light does not reach the slice at all and should be culled. Update the internal OmniLight3D's `omni_range` accordingly each frame, or compute the effective energy reduction.

### 8.6 SpotLight4D

Inherits from `Light4D`. Analog of `SpotLight3D`.

```
File: src/nodes/lights/spot_light_4d.h
File: src/nodes/lights/spot_light_4d.cpp
```

A cone-shaped light. In 4D, the "cone" is technically a hypercone. When projected into the 3D slice, the intersection of a hypercone with a hyperplane is a cone (or the full interior, or nothing, depending on geometry). For simplicity, project the spot light's position and direction into 3D and use a standard SpotLight3D with adjusted parameters.

**Additional properties from SpotLight3D (verified from docs):**

| Property | Type | Default | Description |
|---|---|---|---|
| `spot_range` | `real_t` | 5.0 | Light range in world units. |
| `spot_attenuation` | `real_t` | 1.0 | Attenuation curve exponent. |
| `spot_angle` | `real_t` | 45.0 | Cone angle in degrees. |
| `spot_angle_attenuation` | `real_t` | 1.0 | Attenuation based on angle from cone center. |

**4D projection:** Same approach as OmniLight4D for range adjustment. The spot direction (a 4D vector) is projected to 3D. The cone angle may need adjustment based on how the hypercone intersects the slice, but for an initial implementation, keeping the angle unchanged and just projecting position + direction works well enough.

### 8.7 Light Projection Implementation Pattern

All three light types follow the same pattern each frame:

```
1. Get Camera4D's slice hyperplane (normal, origin).
2. Get Camera4D's 3D basis (columns 0-2 of the Basis4D).
3. For each Light4D:
   a. Project the light's 4D position into 3D slice coordinates:
      3d_pos.x = dot(basis_col_0, light_4d_pos - camera_4d_origin)
      3d_pos.y = dot(basis_col_1, light_4d_pos - camera_4d_origin)
      3d_pos.z = dot(basis_col_2, light_4d_pos - camera_4d_origin)
   b. For directional/spot: project direction similarly.
   c. Compute W-distance for range/energy adjustment.
   d. Update the internal Light3D's transform and properties.
   e. If the light is out of range (W-distance > range), disable the internal Light3D.
```

This can be done by each Light4D node individually in its `_process`, or coordinated by the Slicer4D singleton alongside mesh slicing. Individual node processing is simpler and adequate for typical light counts.

### 8.8 Registration

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
    ClassDB::register_class<Camera4D>();
    ClassDB::register_abstract_class<Light4D>();
    ClassDB::register_class<DirectionalLight4D>();
    ClassDB::register_class<OmniLight4D>();
    ClassDB::register_class<SpotLight4D>();
}
```

Camera4D and the concrete light classes are concrete. Light4D is abstract.

---

## Stage 9: Editor Integration (Plugin, Inspectors, Gizmos, Viewport)

The editor integration makes 4D nodes usable in the Godot editor. Without it, users would have to set properties via code only, with no visual feedback. This stage creates an EditorPlugin that registers custom inspectors, 3D gizmos, and a viewport panel for visualizing the 4D scene.

### 9.1 Study Material

Read these before implementing:

- Making plugins: https://docs.godotengine.org/en/stable/tutorials/plugins/editor/making_plugins.html
- Inspector plugins: https://docs.godotengine.org/en/stable/tutorials/plugins/editor/inspector_plugins.html
- 3D gizmo plugins: https://docs.godotengine.org/en/stable/tutorials/plugins/editor/3d_gizmos.html
- EditorPlugin class: https://docs.godotengine.org/en/stable/classes/class_editorplugin.html
- EditorInspectorPlugin class: https://docs.godotengine.org/en/stable/classes/class_editorinspectorplugin.html
- EditorNode3DGizmoPlugin class: https://docs.godotengine.org/en/stable/classes/class_editornode3dgizmoplugin.html
- EditorNode3DGizmo class: https://docs.godotengine.org/en/stable/classes/class_editornode3dgizmo.html

### 9.2 EditorPlugin4D

The main editor plugin class. Registers all editor-side components.

```
File: src/editor/editor_plugin_4d.h
File: src/editor/editor_plugin_4d.cpp
```

Inherits from `EditorPlugin`. Registered at `MODULE_INITIALIZATION_LEVEL_EDITOR`.

**Responsibilities:**

1. Register the `EditorInspectorPlugin4D` (custom inspectors for 4D types).
2. Register the `EditorNode3DGizmoPlugin4D` (gizmos for 4D nodes in the 3D viewport).
3. Add a 4D viewport panel (bottom panel or side panel) for visualizing the sliced scene with interactive W-axis controls.
4. Add menu items under the editor toolbar for 4D-specific actions.

```cpp
void EditorPlugin4D::_enter_tree() {
    inspector_plugin = memnew(EditorInspectorPlugin4D);
    add_inspector_plugin(inspector_plugin);

    gizmo_plugin = memnew(EditorGizmoPlugin4D);
    add_node_3d_gizmo_plugin(gizmo_plugin);

    viewport_panel = memnew(Viewport4DPanel);
    add_control_to_bottom_panel(viewport_panel, "4D View");
}

void EditorPlugin4D::_exit_tree() {
    remove_inspector_plugin(inspector_plugin);
    remove_node_3d_gizmo_plugin(gizmo_plugin);
    remove_control_from_bottom_panel(viewport_panel);
    memdelete(viewport_panel);
}
```

### 9.3 Custom Inspector Properties

4D types (Vector4D, Basis4D, Transform4D, Bivector4D, AABB4D) are custom RefCounted classes, not built-in Godot types. The default inspector will show them as generic objects. Custom property editors make them editable.

```
File: src/editor/inspector_plugin_4d.h
File: src/editor/inspector_plugin_4d.cpp
```

Inherits from `EditorInspectorPlugin`.

**`_can_handle(Object *object)`:** Return true for any object that has 4D-typed properties. In practice, return true for all Node4D subclasses, all Shape4D subclasses, and any resource with 4D properties.

**`_parse_property(object, type, name, hint_type, hint_string, usage_flags, wide)`:** Check the property type. For properties of type Vector4D, Basis4D, Transform4D, etc., create and return a custom `EditorProperty` subclass. Return true to replace the default editor.

**Custom property editors needed:**

| Type | Editor widget |
|---|---|
| `Vector4D` | 4 float fields labeled X, Y, Z, W. Similar to Vector3 editor but with an extra field. |
| `Basis4D` | 4x4 matrix grid of float fields. Optionally, expose rotation as 6 plane-angle pairs (XY, XZ, XW, YZ, YW, ZW) with angle sliders. |
| `Transform4D` | Basis4D editor + Vector4D origin editor. Similar to how Transform3D shows a Basis + origin. |
| `AABB4D` | Position (Vector4D) + Size (Vector4D). 8 float fields total. |
| `Plane4D` | Normal (Vector4D) + Distance (float). 5 fields. |
| `Bivector4D` | 6 float fields labeled XY, XZ, XW, YZ, YW, ZW. |

Each custom editor extends `EditorProperty` and follows the pattern from the inspector plugins tutorial:

1. In `_init()`, create the UI controls (SpinBoxes or EditorSpinSliders for each component).
2. When a control value changes, call `emit_changed(property_name, new_value)`.
3. Override `_update_property()` to read the current value from the object and update the controls.

### 9.4 3D Gizmos for 4D Nodes

Since 4D nodes inherit from Node4D (not Node3D), they do not automatically get 3D gizmos. However, the editor's 3D viewport shows the sliced scene. Gizmos must visualize the 4D nodes' sliced positions in this 3D viewport.

```
File: src/editor/gizmo_plugin_4d.h
File: src/editor/gizmo_plugin_4d.cpp
```

Inherits from `EditorNode3DGizmoPlugin`.

**Problem:** `EditorNode3DGizmoPlugin` attaches gizmos to Node3D nodes. Your Node4D nodes are not Node3D nodes. There are two approaches:

**Approach A (Recommended): Proxy Node3D for each Node4D.**

Each Node4D that needs a gizmo creates a hidden Node3D "proxy" in the editor. This proxy's 3D transform is set to the sliced position of the Node4D. The gizmo plugin attaches to these proxy Node3Ds. When the gizmo is manipulated (dragged), the proxy's 3D transform change is converted back to a 4D transform change on the Node4D.

This is similar to how Camera4D already manages an internal Camera3D. Extend the pattern: during editor sessions, each Node4D creates a proxy Node3D for gizmo purposes.

**Approach B: Custom overlay drawing.**

Skip the gizmo system entirely. Instead, use `EditorPlugin::_forward_3d_draw_over_viewport()` to draw custom 2D overlays on the 3D viewport, or use `EditorPlugin::_forward_3d_gui_input()` to handle mouse input for custom manipulation handles. More flexible but much more work.

**Start with Approach A.** It leverages the existing gizmo infrastructure.

**Gizmo visuals for each node type:**

| Node | Gizmo visualization |
|---|---|
| Node4D (generic) | 4D axis indicator: slice the 4 basis vectors with the view hyperplane, draw the resulting 3D vectors as colored arrows (X=red, Y=green, Z=blue, W=magenta or white). |
| Camera4D | Camera frustum wireframe (from the internal Camera3D) plus a W-axis indicator showing the slice direction. |
| Light4D subclasses | Same as the standard light gizmos (directional arrow, omni sphere, spot cone) but positioned at the sliced 3D location. |
| CollisionShape4D | Slice the shape with the view hyperplane and draw the wireframe of the cross-section. |
| RayCast4D | Draw the sliced ray line segment. |
| Area4D, PhysicsBody4D | Axis indicator at the sliced position. Collision shapes are drawn via their CollisionShape4D children. |

**Gizmo handles for transforms:**

When a Node4D is selected, display movable handles for the 4 translation axes and rotation in the 6 planes. For initial implementation, provide handles for the 3 axes visible in the slice (X, Y, Z within the slice frame) using standard 3D gizmo handles. W-axis movement can be handled via a special handle or via the inspector.

### 9.5 4D Viewport Panel

A bottom panel that provides 4D-specific editor controls.

```
File: src/editor/viewport_4d_panel.h
File: src/editor/viewport_4d_panel.cpp
```

**Contents:**

1. **W-axis slider/spinbox:** Controls the editor camera's W position. Dragging this slides the slice plane through the 4th dimension. This is the primary way to "explore" the 4D scene in the editor.

2. **Rotation plane selectors:** Six buttons or dropdown for the 6 rotation planes (XY, XZ, XW, YZ, YW, ZW). Selecting one + dragging an angle slider rotates the editor view in that plane.

3. **Slice visualization info:** Displays the current hyperplane equation (normal + distance) and the W coordinate being viewed.

4. **Projection mode toggle:** Switch between perspective and orthographic 4D-to-3D projection. In the orthographic 4D mode, the slice is a flat hyperplane. In a 4D perspective mode, the slice could have a 4D "field of view" that warps objects based on their W distance.

5. **Quick navigation:** Buttons to snap to W=0, or to center on a selected Node4D's W coordinate.

**Implementation:** This is a VBoxContainer (or HBoxContainer) with Godot UI nodes (SpinBox, Slider, Button, Label). When the W slider changes, it updates the editor's 4D camera position and triggers a re-slice of all 4D geometry. The re-sliced geometry appears in the standard 3D editor viewport.

### 9.6 Editor Camera (4D)

The editor needs a 4D camera that controls slicing during editing. This is separate from any Camera4D the user places in their scene.

**Implementation:** The editor plugin creates an internal Camera4D (or just a Transform4D + slice parameters) that the Slicer4D singleton uses when rendering the editor viewport. The 4D viewport panel's controls manipulate this editor camera.

When the user places their own Camera4D in the scene and clicks "Preview," the editor switches to viewing through that camera instead (like the Camera3D preview button).

### 9.7 Node4D Transform Toolbar

When a Node4D is selected in the scene tree, the standard 3D toolbar (move, rotate, scale) should work on the sliced 3D representation. Implement this via `EditorPlugin::_forward_3d_gui_input()`:

1. Intercept mouse events in the 3D viewport when a Node4D is selected.
2. Use the existing 3D manipulation tools' behavior but convert the 3D transform changes back to 4D.
3. For XYZ movement within the slice: project the 3D delta back into 4D using the camera's basis.
4. For W movement: use a separate handle or shortcut key + drag.

This is the most complex part of editor integration. For initial implementation, rely on the inspector for precise transforms and provide only basic move handles in the viewport.

### 9.8 Registration

```cpp
if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
    EditorPlugins::add_by_type<EditorPlugin4D>();
}
```

The editor plugin registers itself and all its sub-components (inspector plugin, gizmo plugin, panels) in its `_enter_tree` method.

---

**END OF STAGE 9**

---

## Stage 10: Icons, Registration, and Final Integration

This stage ties everything together: icons for the editor, the GDExtension entry point, the build system, and verification that all pieces connect correctly.

### 10.1 Icons

Every node class needs an icon for the scene tree and the "Add Node" dialog. Godot uses SVG icons.

```
File: src/icons/
    Node4D.svg
    CollisionObject4D.svg
    PhysicsBody4D.svg
    StaticBody4D.svg
    AnimatableBody4D.svg
    CharacterBody4D.svg
    RigidBody4D.svg
    Area4D.svg
    VisualInstance4D.svg
    GeometryInstance4D.svg
    MeshInstance4D.svg
    Sprite4D.svg
    AnimatedSprite4D.svg
    SpriteBase4D.svg
    Camera4D.svg
    Light4D.svg
    DirectionalLight4D.svg
    OmniLight4D.svg
    SpotLight4D.svg
    CollisionShape4D.svg
    CollisionPolygon4D.svg
    RayCast4D.svg
    Joint4D.svg
    PinJoint4D.svg
    HingeJoint4D.svg
    SliderJoint4D.svg
    ConeTwistJoint4D.svg
    Generic10DOFJoint4D.svg
```

**Design guidelines:**

- Base each icon on the corresponding 3D icon from Godot's editor theme. For example, Camera4D's icon should be recognizably similar to Camera3D's icon.
- Add a visual indicator that it is "4D" - a small "4D" label in the corner, a fourth axis line, or a tesseract motif.
- Match the Godot icon color scheme: nodes use the class family's color. Physics nodes use blue tones. Visual nodes use red/orange. Camera is blue-purple. Lights are yellow.
- SVG format, 16x16 viewport with shapes designed to be legible at that size.
- Study the existing icon SVGs in Godot source: `https://github.com/godotengine/godot/tree/master/editor/icons`

**Registration:** In GDExtension, icons can be registered via the `_get_plugin_icon()` override in EditorPlugin, or by placing them in a known path and referencing them in class registration. The simplest approach from GDExtension is to use `EditorPlugin::_get_plugin_icon()` for the plugin itself, and for individual class icons, use the `set_icon()` approach or register them in the theme.

For GDExtension class icons, the standard approach is:

```cpp
ClassDB::register_class<Node4D>();
// Icon is loaded by name convention: the editor looks for
// "res://addons/your_extension/icons/Node4D.svg"
// or you set it programmatically in the editor plugin
```

In the editor plugin's `_enter_tree()`, load and register icons for each class:

```cpp
Ref<Texture2D> icon = ResourceLoader::load("res://addons/godot4d/icons/Node4D.svg");
// Use get_editor_interface()->get_base_control()->get_theme()->set_icon(...)
```

The exact mechanism depends on Godot version. Study how other GDExtensions (like godot-jolt before it was merged) handle icon registration.

### 10.2 GDExtension Entry Point

The single entry point file that registers everything.

```
File: src/register_types.h
File: src/register_types.cpp
```

```cpp
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_4d_extension(ModuleInitializationLevel p_level) {

    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
        // Math types (RefCounted wrappers)
        ClassDB::register_class<Vector4D>();
        ClassDB::register_class<Basis4D>();
        ClassDB::register_class<Transform4D>();
        ClassDB::register_class<AABB4D>();
        ClassDB::register_class<Plane4D>();
        ClassDB::register_class<Projection4D>();

        // Shape resources
        ClassDB::register_abstract_class<Shape4D>();
        ClassDB::register_class<BoxShape4D>();
        ClassDB::register_class<SphereShape4D>();
        ClassDB::register_class<CapsuleShape4D>();
        ClassDB::register_class<CylinderShape4D>();
        ClassDB::register_class<ConvexPolygonShape4D>();
        ClassDB::register_class<ConcavePolygonShape4D>();
        ClassDB::register_class<WorldBoundaryShape4D>();
        ClassDB::register_class<SeparationRayShape4D>();

        // Mesh resources
        ClassDB::register_abstract_class<Mesh4D>();
        ClassDB::register_class<ArrayMesh4D>();
        ClassDB::register_abstract_class<PrimitiveMesh4D>();
        ClassDB::register_class<HyperBoxMesh4D>();
        ClassDB::register_class<HyperSphereMesh4D>();
        ClassDB::register_class<HyperCylinderMesh4D>();
        ClassDB::register_class<HyperCapsuleMesh4D>();

        // Other resources
        ClassDB::register_class<Material4D>();
        ClassDB::register_class<SpriteFrames4D>();
        ClassDB::register_class<Environment4D>();

        // Physics server singleton
        ClassDB::register_class<PhysicsServer4D>();
        ClassDB::register_class<PhysicsDirectBodyState4D>();
        ClassDB::register_class<PhysicsDirectSpaceState4D>();
        PhysicsServer4D *physics = memnew(PhysicsServer4D);
        Engine::get_singleton()->register_singleton("PhysicsServer4D", physics);

        // Slicer singleton
        ClassDB::register_class<Slicer4D>();
        Slicer4D *slicer = memnew(Slicer4D);
        Engine::get_singleton()->register_singleton("Slicer4D", slicer);
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        // Base node
        ClassDB::register_class<Node4D>();

        // Physics nodes
        ClassDB::register_abstract_class<CollisionObject4D>();
        ClassDB::register_abstract_class<PhysicsBody4D>();
        ClassDB::register_class<StaticBody4D>();
        ClassDB::register_class<AnimatableBody4D>();
        ClassDB::register_class<CharacterBody4D>();
        ClassDB::register_class<RigidBody4D>();
        ClassDB::register_class<Area4D>();
        ClassDB::register_class<KinematicCollision4D>();

        // Joint nodes
        ClassDB::register_abstract_class<Joint4D>();
        ClassDB::register_class<PinJoint4D>();
        ClassDB::register_class<HingeJoint4D>();
        ClassDB::register_class<SliderJoint4D>();
        ClassDB::register_class<ConeTwistJoint4D>();
        ClassDB::register_class<Generic10DOFJoint4D>();

        // Collision utility nodes
        ClassDB::register_class<CollisionShape4D>();
        ClassDB::register_class<CollisionPolygon4D>();
        ClassDB::register_class<RayCast4D>();

        // Visual nodes
        ClassDB::register_abstract_class<VisualInstance4D>();
        ClassDB::register_abstract_class<GeometryInstance4D>();
        ClassDB::register_class<MeshInstance4D>();
        ClassDB::register_abstract_class<SpriteBase4D>();
        ClassDB::register_class<Sprite4D>();
        ClassDB::register_class<AnimatedSprite4D>();

        // Camera
        ClassDB::register_class<Camera4D>();

        // Lights
        ClassDB::register_abstract_class<Light4D>();
        ClassDB::register_class<DirectionalLight4D>();
        ClassDB::register_class<OmniLight4D>();
        ClassDB::register_class<SpotLight4D>();
    }

    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        EditorPlugins::add_by_type<EditorPlugin4D>();
    }
}

void uninitialize_4d_extension(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SERVERS) {
        Engine::get_singleton()->unregister_singleton("PhysicsServer4D");
        Engine::get_singleton()->unregister_singleton("Slicer4D");
    }
}

extern "C" {
GDExtensionBool GDE_EXPORT godot4d_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization *r_initialization
) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);
    init_obj.register_initializer(initialize_4d_extension);
    init_obj.register_terminator(uninitialize_4d_extension);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SERVERS);
    return init_obj.init();
}
}
```

### 10.3 GDExtension Configuration File

```
File: godot4d.gdextension
```

```ini
[configuration]
entry_symbol = "godot4d_init"
compatibility_minimum = "4.3"

[libraries]
linux.debug.x86_64 = "res://addons/godot4d/bin/libgodot4d.linux.template_debug.x86_64.so"
linux.release.x86_64 = "res://addons/godot4d/bin/libgodot4d.linux.template_release.x86_64.so"
windows.debug.x86_64 = "res://addons/godot4d/bin/libgodot4d.windows.template_debug.x86_64.dll"
windows.release.x86_64 = "res://addons/godot4d/bin/libgodot4d.windows.template_release.x86_64.dll"
macos.debug = "res://addons/godot4d/bin/libgodot4d.macos.template_debug.framework"
macos.release = "res://addons/godot4d/bin/libgodot4d.macos.template_release.framework"
```

### 10.4 Build System (SCons)

The SConstruct file at the project root. Uses godot-cpp as a submodule.

```
File: SConstruct
```

```python
#!/usr/bin/env python
import os

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])

# Add Eigen include path
env.Append(CPPPATH=["thirdparty/eigen/"])

# Add Jolt4D include path
env.Append(CPPPATH=["thirdparty/jolt4d/"])

sources = Glob("src/*.cpp")
sources += Glob("src/math/*.cpp")
sources += Glob("src/nodes/*.cpp")
sources += Glob("src/nodes/physics/*.cpp")
sources += Glob("src/nodes/physics/joints/*.cpp")
sources += Glob("src/nodes/collision/*.cpp")
sources += Glob("src/nodes/lights/*.cpp")
sources += Glob("src/resources/*.cpp")
sources += Glob("src/servers/physics/*.cpp")
sources += Glob("src/servers/slicer/*.cpp")
sources += Glob("src/editor/*.cpp")

# Jolt4D sources
sources += Glob("thirdparty/jolt4d/Jolt/**/*.cpp")

library = env.SharedLibrary(
    "addons/godot4d/bin/libgodot4d{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
```

### 10.5 Directory Layout (Final)

```
godot4d/
    SConstruct
    godot-cpp/                    (submodule)
    thirdparty/
        eigen/                    (Eigen headers)
        jolt4d/                   (Forked Jolt, modified for 4D)
    src/
        register_types.h
        register_types.cpp
        math/
            vector4d.h / .cpp
            basis4d.h / .cpp
            transform4d.h / .cpp
            aabb4d.h / .cpp
            plane4d.h / .cpp
            projection4d.h / .cpp
            bivector4d.h / .cpp
        nodes/
            node_4d.h / .cpp
            camera_4d.h / .cpp
            physics/
                collision_object_4d.h / .cpp
                physics_body_4d.h / .cpp
                static_body_4d.h / .cpp
                animatable_body_4d.h / .cpp
                character_body_4d.h / .cpp
                rigid_body_4d.h / .cpp
                area_4d.h / .cpp
                kinematic_collision_4d.h / .cpp
                joints/
                    joint_4d.h / .cpp
                    pin_joint_4d.h / .cpp
                    hinge_joint_4d.h / .cpp
                    slider_joint_4d.h / .cpp
                    cone_twist_joint_4d.h / .cpp
                    generic_10dof_joint_4d.h / .cpp
            collision/
                collision_shape_4d.h / .cpp
                collision_polygon_4d.h / .cpp
                ray_cast_4d.h / .cpp
            visual/
                visual_instance_4d.h / .cpp
                geometry_instance_4d.h / .cpp
                mesh_instance_4d.h / .cpp
                sprite_base_4d.h / .cpp
                sprite_4d.h / .cpp
                animated_sprite_4d.h / .cpp
            lights/
                light_4d.h / .cpp
                directional_light_4d.h / .cpp
                omni_light_4d.h / .cpp
                spot_light_4d.h / .cpp
        resources/
            shape_4d.h / .cpp       (base + all shape types)
            mesh_4d.h / .cpp        (base + ArrayMesh4D + PrimitiveMesh4D subtypes)
            material_4d.h / .cpp
            sprite_frames_4d.h / .cpp
            environment_4d.h / .cpp
            texture_4d.h / .cpp
        servers/
            physics/
                physics_server_4d.h / .cpp
                physics_direct_body_state_4d.h / .cpp
                physics_direct_space_state_4d.h / .cpp
            slicer/
                slicer_4d.h / .cpp
                slicer_4d.glsl      (compute shader)
        editor/
            editor_plugin_4d.h / .cpp
            inspector_plugin_4d.h / .cpp
            gizmo_plugin_4d.h / .cpp
            viewport_4d_panel.h / .cpp
        icons/
            Node4D.svg
            ... (all icons listed in 10.1)
    addons/
        godot4d/
            godot4d.gdextension
            bin/                  (compiled libraries go here)
            icons/                (copied icons for editor)
```

### 10.6 Implementation Order

Build and test in this order. Each step should compile and have basic functionality before moving on:

1. **Math types** (Stage 1) - Vector4D, Basis4D, Transform4D, AABB4D, Plane4D. Write unit tests.
2. **Node4D** (Stage 2) - Base node with transform propagation. Verify it appears in the editor's "Add Node" dialog.
3. **Shape4D resources** (Stage 3) - BoxShape4D, SphereShape4D at minimum. Verify they show in the inspector.
4. **Slicer4D + MeshInstance4D** (Stage 4) - Get a sliced tesseract rendering on screen. This is the first visual milestone.
5. **Camera4D** (Stage 8) - Control the slice and see different cross-sections.
6. **PhysicsServer4D** (Stage 5) - Start with broadphase + GJK for sphere-sphere. Get a sphere resting on a floor.
7. **Physics nodes** (Stage 6) - StaticBody4D + RigidBody4D + CollisionShape4D. First interactive physics.
8. **CharacterBody4D** (Stage 6) - move_and_slide in 4D.
9. **Remaining collision nodes** (Stage 7) - CollisionPolygon4D, RayCast4D.
10. **Lights** (Stage 8) - OmniLight4D first (simplest projection).
11. **Joints** (Stage 6) - PinJoint4D first, then others.
12. **Editor integration** (Stage 9) - Inspector properties, gizmos, viewport panel.
13. **Icons and polish** (Stage 10) - Icons, documentation strings, final testing.

### 10.7 Final Verification Checklist

Before considering the extension complete, verify:

- [ ] All classes appear in the "Add Node" / "Create Resource" dialogs with correct icons
- [ ] Node4D transform propagation works (moving parent moves children)
- [ ] At least BoxShape4D and SphereShape4D are functional for collision
- [ ] Slicer4D correctly slices a tesseract (HyperBoxMesh4D) at various W positions
- [ ] Camera4D controls the slice and renders the result
- [ ] A RigidBody4D with a SphereShape4D falls under gravity onto a StaticBody4D floor
- [ ] CharacterBody4D.move_and_slide() works for basic movement
- [ ] At least one light type (OmniLight4D) illuminates sliced geometry
- [ ] At least one joint type (PinJoint4D) constrains two bodies
- [ ] The inspector shows editable fields for all 4D properties
- [ ] The extension compiles on Linux, Windows, and macOS
- [ ] No memory leaks (RIDs are freed on node exit tree, singletons cleaned up)
- [ ] GDScript users can access PhysicsServer4D and Slicer4D singletons
- [ ] The editor does not crash when switching between scenes with/without 4D nodes

---

**END OF STAGE 10**

**THE SPECIFICATION IS COMPLETE.**

This document covers all 10 stages needed to implement a 4D GDExtension for Godot 4.3+:
1. Project structure and 4D math foundations
2. Node4D base class and scene tree
3. Resource types (meshes, shapes, materials)
4. Visual system and GPU slicing pipeline
5. PhysicsServer4D (Jolt fork generalized to 4D)
6. Physics nodes and joint nodes
7. Collision utility nodes
8. Camera4D and lighting
9. Editor integration
10. Icons, registration, and final integration
