#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include "server/physics_server_4d.h"

int main(int argc, char** argv) {
	// Initialize PhysicsServer4D singleton before tests
	PhysicsServer4D::initialize();

	doctest::Context context;
	context.applyCommandLine(argc, argv);

	int res = context.run();

	// Cleanup singleton after tests
	PhysicsServer4D::finalize();

	return res;
}
