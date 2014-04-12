
#include <stdint.h>
#include <stdio.h>
#include <dlfcn.h>

#include <chrono>

#include "demo_generator.h"

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

typedef int (*product_entry_t)(int program_id);
product_entry_t product_run;


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
	int program_id = 1;

	if (argc > 1)
		program_id = atoi(argv[1]);

	fprintf(stderr, "CORE Demo\n");
	// fprintf(stderr, "%d concurrent threads are supported.\n",
	// 		std::thread::hardware_concurrency());

	// Building a product from XML
	fprintf(stderr, ">>> Building product from XML [./product_test.xml]...\n");
	clock_start();
	ProductGenerator pg;
	pg.Parse();
	pg.Build();
	clock_stop();
	fprintf(stderr, ">>> Product build took: %ld[ms]\n\n", clock_ms());

	// Loading product library
	fprintf(stderr, "Loading product library [./libproduct.so]... ");
	void *product_library = dlopen("./libproduct.so", RTLD_NOW);
	if (!product_library) {
		fprintf(stderr, "Product loading FAILED\n%s\n", dlerror());
		return -1;
	}
	fprintf(stderr, "DONE!\n");

	fprintf(stderr, "Linking product entry [product_run]... ");
	product_run = (product_entry_t) dlsym(product_library, "product_run");
	if (!product_run) {
		fprintf(stderr, "Product loading FAILED\n%s\n", dlerror());
		return -1;
	}
	fprintf(stderr, "DONE!\n");

	clock_stop();
	fprintf(stderr, ">>> Overall product entry latency: %ld[ms]\n\n", clock_ms());

	fprintf(stderr, "Running Program ID: %d\n\n", program_id);

	dlclose(product_library);
	return 0;
}

