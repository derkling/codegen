
#include <stdint.h>
#include <stdio.h>

extern "C" {

static void
xtil_internal_function() {
	printf("XTIL internal function\n");
}

/**
 * @brief An example XTIL function
 */
int
xtil_function1(
	void *buffer_in,
	void *buffer_out,
	uint8_t buffer_type,
	uint16_t buffer_width,
	uint16_t buffer_height
	) {
	xtil_internal_function();
	printf("XTIL Function1 [%d, %d]\n",
			buffer_width, buffer_height);
	return 0;
}

/**
 * @brief Another example XTIL function
 */
uint8_t
xtil_function2(
	const char *str,
	uint8_t size
	) {
	printf("XTIL Function2 [%s]\n", str);
	return 0;
}

} // extern C
