
#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>

#include <chrono>

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

typedef void (*program_entry_t)();
program_entry_t program_entry;


int main(int argc, const char *argv[])
{
	steady_clock::time_point start;
	steady_clock::time_point stop;
	auto clock_start = [&]() {
		start = steady_clock::now();
	};
	auto clock_stop = [&]() {
		stop = steady_clock::now();
	};
	auto clock_ms = [&]() {
		return duration_cast<milliseconds>(stop-start).count();
	};


	fprintf(stderr, "CORE Demo\n");

	// Loading program library
	fprintf(stderr, "Loading program library [./libprogram.so]... ");
	void *program_library = dlopen("./libprogram.so", RTLD_NOW);
	if (!program_library) {
		fprintf(stderr, "Program loading FAILED\n%s\n", dlerror());
		return -1;
	}
	fprintf(stderr, "DONE!\n");

	fprintf(stderr, "Linking program entry [program_entry]... ");
	program_entry = (program_entry_t) dlsym(program_library, "program_entry");
	if (!program_entry) {
		fprintf(stderr, "Program loading FAILED\n%s\n", dlerror());
		return -1;
	}
	fprintf(stderr, "DONE!\n");

	program_entry();

	dlclose(program_library);
	return 0;
}

