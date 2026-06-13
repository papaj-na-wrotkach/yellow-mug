#include <cstdlib>

import std;
import yellow_mug.app;
import yellow_mug.core;

int main() {
	using namespace yellow_mug;
	try {
		App app;
		app.run();
	} catch (const std::exception& e) {
		std::println(std::cerr, "Exception caught: {}", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
