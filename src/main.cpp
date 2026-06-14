/**
 * @file main.cpp
 * @brief Application entry point for the yellow-mug image filter pipeline.
 */

#include <cstdlib>

import std;
import yellow_mug.app;

/**
 * @brief Constructs an @ref yellow_mug::App instance and runs the main loop.
 *
 * @details
 * Any `std::exception` thrown during construction or execution is reported to
 * `std::cerr` and the process exits with `EXIT_FAILURE`.
 *
 * @return `EXIT_SUCCESS` on clean exit, `EXIT_FAILURE` on error.
 */
int main()
{
	using namespace yellow_mug;
	try
	{
		App app;
		app.run();
	}
	catch (const std::exception& e)
	{
		std::println(std::cerr, "Exception caught: {}", e.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
