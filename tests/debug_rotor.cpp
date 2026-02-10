#include <iostream>
#include <iomanip>
#include "support/godot_compat.h"
#include "math/rotor4d.h"
#include "math/basis4d.h"

void print_vector(const char* name, const Vector4& v) {
    std::cout << name << ": [" << std::setprecision(4) << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]\n";
}

void print_matrix(const char* name, const Basis4D& m) {
    std::cout << name << ":\n";
    for (int i = 0; i < 4; i++) {
        std::cout << "  [";
        for (int j = 0; j < 4; j++) {
            std::cout << std::setw(8) << std::setprecision(4) << m.rows[i].components[j];
            if (j < 3) std::cout << ", ";
        }
        std::cout << "]\n";
    }
}

int main() {
    std::cout << "=== Testing 90-degree rotation in XY plane ===\n\n";

    // Create a 90-degree rotation in XY plane
    Rotor4D r = Rotor4D::from_plane_angle(Rotor4D::PLANE_XY, MATH_PI / 2.0);

    std::cout << "Rotor components:\n";
    std::cout << "  s=" << r.s << ", e12=" << r.e12 << ", e13=" << r.e13 << ", e14=" << r.e14 << "\n";
    std::cout << "  e23=" << r.e23 << ", e24=" << r.e24 << ", e34=" << r.e34 << ", e1234=" << r.e1234 << "\n\n";

    // Convert to matrix
    Basis4D mat = r.to_basis();
    print_matrix("Rotor as matrix", mat);

    // Also show what the expected matrix should be (from Basis4D::from_rotation)
    Basis4D expected = Basis4D::from_rotation(Basis4D::PLANE_XY, MATH_PI / 2.0);
    print_matrix("\nExpected matrix (from Basis4D)", expected);

    // Test rotation of basis vectors
    std::cout << "\nRotating basis vectors:\n";
    Vector4 vx(1, 0, 0, 0);
    Vector4 vy(0, 1, 0, 0);
    Vector4 vz(0, 0, 1, 0);
    Vector4 vw(0, 0, 0, 1);

    print_vector("  X -> ", r.rotate(vx));
    print_vector("  Y -> ", r.rotate(vy));
    print_vector("  Z -> ", r.rotate(vz));
    print_vector("  W -> ", r.rotate(vw));

    std::cout << "\nExpected for 90° XY rotation:\n";
    std::cout << "  X should -> [0, 1, 0, 0]\n";
    std::cout << "  Y should -> [-1, 0, 0, 0]\n";
    std::cout << "  Z should -> [0, 0, 1, 0]\n";
    std::cout << "  W should -> [0, 0, 0, 1]\n";

    return 0;
}
