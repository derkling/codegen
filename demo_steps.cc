
#include <stdint.h>
#include "demo_xtil.h"

extern "C" {
/*
 * XTIL - GROUP 1
 */

int
step_xtil_function1(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	) {

	xtil_function1(bin, bout, btype, bwidth, bheight);

	return 0;
}

int
step_xtil_function2(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	) {

	xtil_function2("Step_F2", 10);

	return 0;
}

int
step_xtil_function3(
	void *bin,
	void *bout,
	uint8_t btype
	) {

	xtil_function1(bin, bout, btype, 800, 600);

	return 0;
}


/*
 * XTIL - GROUP 2
 */

int
step_xtil_function4(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	) {

	xtil_function2("Hello by STEP_XTIL_F4", 4);

	return 0;
}

int
step_xtil_function5(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	) {

	xtil_function2("Hello by STEP_XTIL_F5", 5);

	return 0;
}

/*
 * MACROS
 */


int
step_macro(
	char *str,
	uint8_t size
	) {

	xtil_function1(0, 0, 0, 1024, 768);
	xtil_function2("STEP MACRO Example", 123);

	return 0;
}



int
step_local(
	uint8_t a,
	uint8_t b
	) {
	return a+b;
}

}

