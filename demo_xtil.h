

extern "C" {

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
	);
/**
 * @brief Another example XTIL function
 */
uint8_t
xtil_function2(
	const char *str,
	uint8_t size
	);

}
