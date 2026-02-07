cmake_minimum_required(VERSION 3.14)
project(flintlock_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Fetch doctest -----------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.4.11
)
FetchContent_MakeAvailable(doctest)

# --- Include paths -----------------------------------------------------------
# The compatibility shim provides minimal Godot types (Vector4 etc.) so that
# unit tests can compile and run without linking against godot-cpp.
# When the real source headers are implemented they will include godot-cpp types;
# the shim is used ONLY for the standalone test executable.
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/support
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# --- Test sources ------------------------------------------------------------
# Enable stages progressively: uncomment a stage once its source code exists.

set(TEST_SOURCES test_main.cpp)

# Stage 1 — Math4D
list(APPEND TEST_SOURCES
    math/test_basis4d.cpp
    math/test_transform4d.cpp
    math/test_aabb4d.cpp
    math/test_rotor4d.cpp
)

# Stage 2 — Shapes
list(APPEND TEST_SOURCES
    shapes/test_hyper_sphere_shape_4d.cpp
    shapes/test_hyper_box_shape_4d.cpp
)

# Stage 3 — Slicer
list(APPEND TEST_SOURCES
    slicer/test_hyperplane.cpp
    slicer/test_sphere_slice.cpp
    slicer/test_box_slice.cpp
)

# Stage 4 — Collision
list(APPEND TEST_SOURCES
    collision/test_gjk_4d.cpp
    collision/test_broadphase_4d.cpp
)

# Stage 5 — Dynamics
list(APPEND TEST_SOURCES
    dynamics/test_gravity_4d.cpp
    dynamics/test_integrator_4d.cpp
)

# Stage 6 — Server
list(APPEND TEST_SOURCES
    server/test_physics_server_4d.cpp
)

# --- Executable --------------------------------------------------------------
add_executable(flintlock_tests ${TEST_SOURCES})
target_link_libraries(flintlock_tests PRIVATE doctest::doctest)
