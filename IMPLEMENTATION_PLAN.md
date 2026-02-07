# Flintlock — Implementation Plan

## Stage Dependency Graph

```
Stage 1: Math4D
    │
    ├──► Stage 2: Shapes
    │        │
    │        ├──► Stage 3: Slicer        ──┐
    │        │                              │
    │        └──► Stage 4: Collision    ──┐ │
    │                  │                  │ │
    │                  ▼                  │ │
    │             Stage 5: Dynamics  ─────┤ │
    │                                     │ │
    │                                     ▼ ▼
    └───────────────────────────────► Stage 6: PhysicsServer4D
                                          │
                                          ▼
                                     Stage 7: Scene Nodes
```

Stages 3 (Slicer) and 4 (Collision) are independent of each other and can be developed
in parallel after Stage 2 is complete.

---

## Stage 1 — Math4D Foundation

No dependencies. Everything else builds on this.

### Deliverables

| File | Purpose |
|---|---|
| `src/math/math_defs_4d.h` | Constants, epsilon, `real_t` alias, utility macros |
| `src/math/basis4d.h / .cpp` | 4×4 matrix representing rotation and scale |
| `src/math/transform4d.h / .cpp` | Basis4D + Vector4 origin |
| `src/math/aabb4d.h / .cpp` | 4D axis-aligned bounding box |
| `src/math/rotor4d.h / .cpp` | Bivector-based rotation (geometric algebra) |
| `src/math/hyperplane4d.h / .cpp` | Oriented hyperplane: normal + distance |

### Requirements

**Basis4D** — 4×4 matrix stored as 4 row `Vector4`s:
- Identity, zero construction
- Matrix multiplication: `Basis4D * Basis4D`, `Basis4D * Vector4`
- Transpose, inverse, determinant
- 6 simple rotation constructors — one per plane: XY, XZ, XW, YZ, YW, ZW
- Construct from axis-pair and angle
- Scale matrix construction
- Orthonormalization (Gram-Schmidt)
- `is_orthogonal()`, `is_rotation()` queries

**Transform4D** — Basis4D + Vector4 origin:
- Identity construction
- `xform(Vector4 point)` — applies full transform (basis * point + origin)
- `xform_inv(Vector4 point)` — inverse transform
- `basis_xform(Vector4 dir)` — rotation/scale only, no translation
- Composition: `Transform4D * Transform4D`
- `inverse()`, `affine_inverse()`
- `translated(Vector4)`, `rotated(Basis4D)`, `scaled(Vector4)`

**AABB4D** — min-corner position + size:
- Construction from position + size, or from two points (min/max)
- `has_point(Vector4)` — containment test
- `intersects(AABB4D)` — overlap test
- `intersection(AABB4D)` — returns overlapping region
- `merge(AABB4D)` — returns enclosing AABB
- `grow(real_t)` — expand by margin on all sides
- `get_center()`, `get_volume()`, `get_longest_axis()`
- `get_support(Vector4 dir)` — farthest point in direction

**Rotor4D** — even-grade element of Cl(4,0):

The even subalgebra of 4D geometric algebra has 8 components:
- 1 scalar
- 6 bivector components: e₁₂, e₁₃, e₁₄, e₂₃, e₂₄, e₃₄
- 1 pseudoscalar: e₁₂₃₄

Operations:
- Construct from bivector plane and angle: `R = cos(θ/2) + sin(θ/2) B̂`
- Construct from two vectors (rotation that maps u to v)
- Sandwich product: `rotate(Vector4 v)` computes `R v R†`
- Geometric product: `Rotor4D * Rotor4D` (composition)
- Conjugate (reverse), norm, normalize
- `slerp(Rotor4D other, real_t t)` — spherical interpolation
- Convert to/from Basis4D
- Identity rotor (scalar = 1, rest = 0)

**Hyperplane4D** — plane defined by unit normal + distance:
- Construction from normal + distance, or from normal + point on plane
- `distance_to(Vector4 point)` — signed distance
- `is_point_over(Vector4)` — positive half-space test
- `project(Vector4 point)` — closest point on plane
- `intersects_segment(Vector4 a, Vector4 b)` — returns intersection point
- `get_tangent_basis()` — returns 3 orthonormal vectors spanning the plane

### Tests → `tests/math/`
- `test_basis4d.cpp` — identity, rotation, multiplication, inverse, determinant
- `test_transform4d.cpp` — identity, composition, point transform, inverse
- `test_aabb4d.cpp` — containment, intersection, merge, grow
- `test_rotor4d.cpp` — plane rotation, sandwich product, composition, slerp

---

## Stage 2 — Shape Definitions

**Depends on:** Stage 1 (Math4D)

### Deliverables

| File | Purpose |
|---|---|
| `src/shapes/shape4d.h / .cpp` | Abstract base class |
| `src/shapes/hyper_sphere_shape_4d.h / .cpp` | 4-sphere |
| `src/shapes/hyper_box_shape_4d.h / .cpp` | Tesseract |
| `src/shapes/hyper_capsule_shape_4d.h / .cpp` | Swept hypersphere |
| `src/shapes/hyper_ellipsoid_shape_4d.h / .cpp` | Axis-aligned 4-ellipsoid |
| `src/shapes/convex_hull_4d.h / .cpp` | Convex hull of point set |

### Requirements

**Shape4D** abstract base:
- `virtual Vector4 get_support(const Vector4 &direction) const` — GJK support function
- `virtual AABB4D get_aabb(const Transform4D &transform) const` — world-space AABB
- `virtual bool contains_point(const Vector4 &point) const` — local-space test

**HyperSphereShape4D**:
- Parameter: `real_t radius`
- Support: `normalize(dir) * radius`
- AABB: center ± (radius, radius, radius, radius)
- Contains: `point.length() <= radius`

**HyperBoxShape4D** (tesseract):
- Parameter: `Vector4 half_extents`
- Support: `Vector4(sign(dir.x) * h.x, sign(dir.y) * h.y, sign(dir.z) * h.z, sign(dir.w) * h.w)`
- AABB: transform-dependent; for axis-aligned, position ± half_extents
- Contains: `|point[i]| <= half_extents[i]` for all i
- 16 vertices (2⁴), 32 edges, 24 square faces, 8 cubic cells

**HyperCapsuleShape4D**:
- Parameters: `real_t radius`, `real_t height` (local W axis)
- Line segment from `(0,0,0,-height/2)` to `(0,0,0,+height/2)`
- Support: closest segment endpoint to direction + `radius * normalize(dir)`
- A Minkowski sum of a line segment and a hypersphere

**HyperEllipsoidShape4D**:
- Parameter: `Vector4 radii` (semi-axes)
- Support: `normalize(radii * dir) * radii` (element-wise multiply)
- Useful for non-uniform astronomical bodies

**ConvexHull4D**:
- Parameter: `Vector<Vector4> points`
- Support: brute-force scan of all vertices, `argmax(dot(v, dir))`
- AABB: min/max over all transformed vertices

### Tests → `tests/shapes/`
- `test_hyper_sphere_shape_4d.cpp` — support function, AABB, containment
- `test_hyper_box_shape_4d.cpp` — support function, AABB, containment

---

## Stage 3 — Hyperplane Slicer

**Depends on:** Stage 1 (Math4D), Stage 2 (Shapes)
**Independent of:** Stage 4 (Collision)

### Deliverables

| File | Purpose |
|---|---|
| `src/slicer/slice_result.h` | Output container: 3D verts, normals, indices |
| `src/slicer/slicer4d.h / .cpp` | Dispatches slicing per shape type |
| `src/slicer/sphere_slicer.h / .cpp` | Analytic sphere-hyperplane intersection |
| `src/slicer/polytope_slicer.h / .cpp` | Edge-clipping for boxes, hulls |

### Requirements

**SliceResult**:
- `Vector<Vector3> vertices` — 3D vertices of the cross-section
- `Vector<Vector3> normals` — per-vertex normals
- `Vector<int> indices` — triangle indices
- `Transform3D transform` — positioning in Godot's 3D world
- `bool is_empty()` — true when shape doesn't intersect hyperplane

**Coordinate mapping**:
The hyperplane defines a 3D coordinate system via `Hyperplane4D::get_tangent_basis()`.
4D intersection points are projected onto this basis to produce 3D coordinates.

**HyperSphere slicing** (analytic):
- Signed distance `d` from sphere center to hyperplane
- If `|d| > radius`: empty
- Otherwise: 3D sphere of radius `sqrt(R² - d²)`
- Center: projection of 4D center onto hyperplane
- Generate sphere mesh with configurable tessellation

**HyperBox slicing** (edge clipping):
- Classify each of the 16 vertices against the hyperplane (front / back)
- For each edge where endpoints are on opposite sides, interpolate the crossing point
- The intersection points form a convex 3D polytope
- Compute convex hull in 3D, triangulate
- An axis-aligned hyperplane through the center of a tesseract produces a cube

**General convex shape slicing**:
- Same vertex-classification + edge-interpolation algorithm
- Works for ConvexHull4D directly
- For HyperCapsule/HyperEllipsoid: either approximate with vertices or provide analytic methods

### Tests → `tests/slicer/`
- `test_hyperplane.cpp` — point classification, distance, projection, segment intersection
- `test_sphere_slice.cpp` — center cut, offset cut, miss, radius validation
- `test_box_slice.cpp` — axis-aligned cut through tesseract center

---

## Stage 4 — Collision Detection

**Depends on:** Stage 1 (Math4D), Stage 2 (Shapes)
**Independent of:** Stage 3 (Slicer)

### Deliverables

| File | Purpose |
|---|---|
| `src/collision/collision_result_4d.h` | Contact point, normal, depth |
| `src/collision/gjk_4d.h / .cpp` | GJK intersection test in 4D |
| `src/collision/epa_4d.h / .cpp` | EPA penetration depth in 4D |
| `src/collision/broadphase_4d.h / .cpp` | AABB4D-based candidate pair generation |

### Requirements

**GJK in 4D**:
- Input: two shapes with support functions + transforms
- Minkowski difference support: `support_A(dir) - support_B(-dir)`
- Simplex evolution in 4D: up to 5 points (pentachoron / 5-cell)
  - `do_simplex_2` (line), `do_simplex_3` (triangle), `do_simplex_4` (tetrahedron),
    `do_simplex_5` (pentachoron)
- Output: `bool colliding`, simplex (passed to EPA if colliding)
- Max iteration limit with convergence check

**EPA in 4D**:
- Input: GJK simplex (pentachoron containing the origin)
- Maintain a convex polytope as a set of tetrahedral facets (3-cells)
- Each iteration: find closest facet to origin, expand polytope with new support point
- Output: `Vector4 normal`, `real_t depth`, `Vector4 contact_point`
- Termination: when new support point doesn't expand polytope beyond tolerance

**Broadphase**:
- AABB4D overlap as the fast rejection test
- Options (implement one, design for swapping):
  - **BVH4D**: balanced tree of AABB4Ds, dynamic insertion/removal
  - **Sweep-and-prune**: sort on 4 axes, mark overlapping intervals
- API: `insert(id, AABB4D)`, `remove(id)`, `update(id, AABB4D)`
- `query_pairs()` → returns all overlapping pairs
- `query_aabb(AABB4D)` → returns all ids overlapping a given box

**CollisionResult4D**:
- `Vector4 point_on_A` — contact point in world space on body A
- `Vector4 point_on_B` — contact point in world space on body B
- `Vector4 normal` — from A to B
- `real_t depth` — penetration depth

### Tests → `tests/collision/`
- `test_gjk_4d.cpp` — separated spheres, overlapping spheres, sphere-box, box-box
- `test_broadphase_4d.cpp` — insert, overlap detection, no false negatives

---

## Stage 5 — Dynamics & N-Body Solver

**Depends on:** Stage 1 (Math4D), Stage 4 (Collision)

### Deliverables

| File | Purpose |
|---|---|
| `src/dynamics/body_state_4d.h / .cpp` | Per-body physical state |
| `src/dynamics/integrator_4d.h / .cpp` | Symplectic Euler integration |
| `src/dynamics/gravity_4d.h / .cpp` | Inverse-cube gravity + Barnes-Hut |
| `src/dynamics/constraint_solver_4d.h / .cpp` | Contact resolution |

### Requirements

**BodyState4D**:
- Mass and inverse mass (`real_t`; 0 inv_mass = static)
- Inertia tensor: 6×6 symmetric matrix in bivector space
  (4D has 6 planes of rotation: XY, XZ, XW, YZ, YW, ZW; angular velocity
   and angular momentum are bivectors with 6 components each)
- Position (`Vector4`), orientation (`Rotor4D`)
- Linear velocity (`Vector4`), angular velocity (6-component bivector)
- Force and torque accumulators (cleared each step)
- Damping coefficients (linear, angular)
- Body mode: STATIC, KINEMATIC, RIGID

**Integrator** (semi-implicit / symplectic Euler):
1. `linear_velocity += (force * inverse_mass) * dt`
2. `position += linear_velocity * dt`
3. `angular_velocity += (inverse_inertia * torque) * dt`
4. `orientation = rotor_from_bivector(angular_velocity * dt) * orientation`
5. `orientation.normalize()`
6. Apply damping: `velocity *= (1 - linear_damp * dt)`, etc.
7. Clear force/torque accumulators

**Gravity (inverse-cube law)**:

In 4D the gravitational force is:

```
F = G * m₁ * m₂ / r³  (direction: unit vector from body 1 to body 2)
```

This follows from Gauss's law: flux through a 3-sphere surface ~ r³, so
force ~ 1/r³. This is the physically correct 4D gravity, not a tuning choice.

- Pairwise gravity: O(N²) for small N
- Barnes-Hut with **16-tree** (4D spatial partitioning, 2⁴ = 16 children per node):
  - Build tree each step from body positions
  - Walk tree; if cell angular size < θ, treat as single mass at center-of-mass
  - Target: O(N log N)
  - θ ~ 0.5 is a reasonable default (tunable accuracy/speed)
- Softening: `F = G * m₁ * m₂ / (r² + ε²)^(3/2)` to avoid singularity

**Constraint solver** (sequential impulse):
- Convert each contact to a velocity constraint
- Iteratively apply corrective impulses (Gauss-Seidel)
- Normal impulse: non-penetration along contact normal
- Friction impulse: tangent plane in 4D is 3-dimensional
  (need 3 orthogonal tangent directions per contact)
- Warmstarting: cache impulses from previous frame
- Position correction: Baumgarte stabilization or split-impulse

### Tests → `tests/dynamics/`
- `test_gravity_4d.cpp` — inverse-cube force, direction, softening
- `test_integrator_4d.cpp` — stationary body, linear motion, acceleration, damping

---

## Stage 6 — PhysicsServer4D

**Depends on:** All previous stages

### Deliverables

| File | Purpose |
|---|---|
| `src/server/physics_server_4d.h / .cpp` | Godot singleton server |
| `src/server/space_4d.h / .cpp` | Internal: world container |
| `src/server/body_4d.h / .cpp` | Internal: managed body |
| `src/server/area_4d.h / .cpp` | Internal: area (trigger zone) |
| `src/server/physics_direct_body_state_4d.h / .cpp` | Direct state access |
| `src/server/physics_direct_space_state_4d.h / .cpp` | Spatial queries |

### Requirements

**PhysicsServer4D** singleton — API modelled on `PhysicsServer3D`:

```
Space management:
  space_create() → RID
  space_set_active(RID, bool)
  space_set_param(RID, param, value)

Body management:
  body_create() → RID
  body_set_space(RID body, RID space)
  body_set_mode(RID, BodyMode)         # STATIC, KINEMATIC, RIGID
  body_set_state(RID, BodyState, val)  # transform, velocity, etc.
  body_get_state(RID, BodyState) → val
  body_add_shape(RID body, RID shape, Transform4D local_xform)
  body_remove_shape(RID body, int index)
  body_apply_force(RID, Vector4 force, Vector4 position)
  body_apply_impulse(RID, Vector4 impulse, Vector4 position)
  body_set_force_integration_callback(RID, Callable)

Shape management:
  shape_create(ShapeType) → RID
  shape_set_data(RID, Variant)

Area management:
  area_create() → RID
  area_set_space(RID, RID)
  area_set_param(RID, AreaParam, val)  # gravity override, damping
  area_set_monitor_callback(RID, Callable)

Simulation:
  step(real_t delta)

Queries (via PhysicsDirectSpaceState4D):
  intersect_ray(from, to) → result
  intersect_shape(shape_rid, transform) → results
  cast_motion(shape_rid, transform, motion) → [safe, unsafe]

Slicing:
  set_slice_hyperplane(Hyperplane4D)
  get_body_slice(RID body) → SliceResult
```

**Step loop** (called each physics frame):
1. Update area overlaps
2. Apply gravity (via dynamics gravity solver)
3. Run broadphase → candidate pairs
4. Run narrowphase (GJK/EPA) → contacts
5. Solve constraints (sequential impulse)
6. Integrate all bodies
7. Update broadphase structure
8. Generate slice geometry for visible bodies

### Tests → `tests/server/`
- `test_physics_server_4d.cpp` — create space/body/shape, step, verify movement

---

## Stage 7 — Scene Nodes & GDScript API

**Depends on:** Stage 6 (PhysicsServer4D)

### Deliverables

| File | Purpose |
|---|---|
| `src/nodes/collision_object_4d.h / .cpp` | Base for all 4D physics objects |
| `src/nodes/physics_body_4d.h / .cpp` | Base for bodies |
| `src/nodes/rigid_body_4d.h / .cpp` | Dynamic rigid body |
| `src/nodes/static_body_4d.h / .cpp` | Immovable body |
| `src/nodes/area_4d.h / .cpp` | Trigger / gravity zone |
| `src/nodes/collision_shape_4d.h / .cpp` | Shape attached to a body |
| `src/nodes/camera_4d.h / .cpp` | Drives the slice hyperplane |

### Requirements

All nodes inherit from `Node3D` (so they participate in the Godot scene tree) and
maintain a parallel 4D representation internally.

**CollisionObject4D** (abstract base):
- Owns a server body or area RID
- Manages attached shapes
- Proxies `_enter_tree` / `_exit_tree` to register/unregister with PhysicsServer4D

**RigidBody4D**:
- Exported properties: mass, inertia, linear_damp, angular_damp, gravity_scale
- Modes: rigid, static, kinematic (matching Godot's `RigidBody3D`)
- `_integrate_forces(PhysicsDirectBodyState4D)` virtual callback
- Methods: `apply_force()`, `apply_impulse()`, `apply_torque()`
- Reads back transform from server each frame, updates 3D node transform from slice

**StaticBody4D**:
- Transform set from editor/script, pushed to server
- No dynamics integration

**Area4D**:
- Signals: `body_entered(Node)`, `body_exited(Node)`
- Properties: gravity override (direction, strength), damping override
- Space override modes: combine, replace, etc.

**CollisionShape4D**:
- Property: `Shape4D shape` resource
- Attached as child of a body node
- Pushes shape to parent's server body on ready

**Camera4D**:
- Extends `Camera3D`
- Additional property: `real_t w_position` — position along the W axis
- Property: `Rotor4D hyperplane_rotation` — rotation of the viewing hyperplane
- Computes the `Hyperplane4D` and pushes it to the slicer each frame
- Default: hyperplane normal = (0, 0, 0, 1), positioned at `w_position`

### Tests
- GDScript integration tests in `demo/` project (manual + scripted)
- No standalone C++ tests for this stage — nodes require the Godot runtime

---

## Implementation Order (Suggested)

| Order | Stage | Estimated Scope |
|---|---|---|
| 1 | Math4D | ~1200 lines |
| 2 | Shapes | ~600 lines |
| 3a | Slicer | ~800 lines |
| 3b | Collision (parallel with 3a) | ~1000 lines |
| 4 | Dynamics | ~1000 lines |
| 5 | PhysicsServer4D | ~800 lines |
| 6 | Scene Nodes | ~600 lines |

Total estimate: ~6000 lines of core C++, plus tests.
