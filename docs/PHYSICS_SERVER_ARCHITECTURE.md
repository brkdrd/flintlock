# PhysicsServer4D Architecture

## Overview

The PhysicsServer4D implementation uses a **two-layer architecture** to separate internal C++ implementation from the Godot-facing API:

```
GDScript
    ↓
PhysicsServer4DGodot (Godot Singleton)
    ↓
PhysicsServer4D (Internal C++ Singleton)
    ↓
4D Physics Implementation
```

## Components

### 1. PhysicsServer4D (Internal C++ Singleton)

**Location:** `src/server/physics_server_4d.h/.cpp`

- Pure C++ implementation
- Uses `flintlock::PhysicsRID` for internal resource tracking
- Manages spaces, bodies, shapes, and simulation
- Not directly accessible from GDScript

### 2. PhysicsServer4DGodot (Godot Wrapper)

**Location:** `src/server/physics_server_4d_godot.h/.cpp`

- Inherits from `godot::Object`
- Registered as an **Engine singleton** named `"PhysicsServer4D"`
- Converts between `godot::RID` and `flintlock::PhysicsRID`
- Exposes all server methods to GDScript via `ClassDB::bind_method`
- Maintains RID mapping tables for conversion

### 3. Registration

**Location:** `src/register_types.cpp`

The initialization sequence:

1. **SERVER level** - Initialize internal C++ singleton
2. **SERVER level** - Register `PhysicsServer4DGodot` class
3. **SERVER level** - Create singleton instance and register with Engine
4. **SCENE level** - Register node classes (RigidBody4D, etc.)

```cpp
// Internal C++ server
PhysicsServer4D::initialize();

// Godot-facing wrapper
GDREGISTER_CLASS(PhysicsServer4DGodot);
physics_server_4d_singleton = memnew(PhysicsServer4DGodot);
Engine::get_singleton()->register_singleton("PhysicsServer4D", physics_server_4d_singleton);
```

## RID Conversion

Godot uses `godot::RID` for resource identifiers, while our internal physics uses `flintlock::PhysicsRID`. The wrapper maintains bidirectional mapping:

```cpp
std::map<uint64_t, flintlock::PhysicsRID> rid_map;          // Godot ID → Internal RID
std::map<uint64_t, uint64_t> reverse_rid_map;               // Internal ID → Godot ID
```

## Usage from GDScript

The singleton is accessible globally:

```gdscript
# Check singleton exists
if PhysicsServer4D:
    print("Server available!")

# Create resources via server
var body = PhysicsServer4D.body_create()
var space = PhysicsServer4D.space_create()
var shape = PhysicsServer4D.shape_create(PhysicsServer4D.SHAPE_HYPER_SPHERE)

# Configure body
PhysicsServer4D.body_set_space(body, space)
PhysicsServer4D.body_set_mode(body, PhysicsServer4D.BODY_MODE_RIGID)
PhysicsServer4D.body_set_param(body, PhysicsServer4D.BODY_PARAM_MASS, 10.0)

# Apply forces
PhysicsServer4D.body_apply_impulse(body, Vector4(1, 0, 0, 0))

# Step simulation
PhysicsServer4D.step(delta)

# Cleanup
PhysicsServer4D.free_rid(body)
```

## Why Two Layers?

**Separation of Concerns:**
- Internal implementation can use optimal C++ patterns
- Godot-facing API handles GDScript type conversions
- RID conversion is centralized in one place

**Future Extensibility:**
- Can add GDScript-specific convenience methods
- Internal server can optimize without breaking GDScript API
- Testing can use either layer independently

**Compatibility:**
- Matches Godot's own server pattern (PhysicsServer3D, RenderingServer, etc.)
- Familiar API for Godot developers
- Clean integration with Engine singleton system
