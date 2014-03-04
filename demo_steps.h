
extern "C" {

////////////////////////////////////////////////////////////////////////////////
/**

\mainpage Introduction

Main documentation text

\section sec_xtil XTIL Documentation

This is the XTIL documentation...

\subsection sec_xtil_functions XTIL Functions

These are the documentation of tht main functions
-# @ref steps_xtil

*/


////////////////////////////////////////////////////////////////////////////////
/**
@defgroup steps_xtil XTIL Functions

This is the set of XTIL exported functions

*/

/**
 * @defgroup steps_xtil_group1 Group1 XTIL Functions
 * @ingroup steps_xtil
 *
 * This is GROUP1 of XTIL Functions
 */

/**
 * @brief An example of a step function which wraps just a single XTIL call
 * @ingroup steps_xtil_group1
 */
int
step_xtil_function1(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth = 1924,
	uint16_t bheight = 768
	);
extern int step_xtil_function1(void*, void*, uint8_t, uint16_t, uint16_t);

/**
 * @brief An example of a step function which wraps just a single XTIL call
 * @ingroup steps_xtil_group1
 */
int
step_xtil_function2(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	);
extern int step_xtil_function2(void*, void*, uint8_t, uint16_t, uint16_t);

/**
 * @brief An example of a step function which wraps just a single XTIL call
 * @ingroup steps_xtil_group1
 */
int
step_xtil_function3(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	);

/**
 * @defgroup steps_xtil_group2 Group2 XTIL Functions
 * @ingroup steps_xtil
 *
 * This is GROUP2 of XTIL Functions
 */

/**
 * @brief Another example of a step function which wraps just a single XTIL call
 * @ingroup steps_xtil_group2
 */
int
step_xtil_function4(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	);

/**
 * @brief Another example of a step function which wraps just a single XTIL call
 * @ingroup steps_xtil_group2
 */
int
step_xtil_function5(
	void *bin,
	void *bout,
	uint8_t btype,
	uint16_t bwidth,
	uint16_t bheight
	);

////////////////////////////////////////////////////////////////////////////////
/**
@defgroup steps_macros XTIL Macros

This is the set of XTIL based Macro Functions

*/

/**
 * @brief Another example step function which wraps multiple XTIL calls
 * @ingroup steps_macros
 */
int
step_macro(
	char *str,
	uint8_t size
	);

////////////////////////////////////////////////////////////////////////////////
/**
@defgroup steps_local Other Functions

This is the set of "local" exported functions

*/

/**
 * @brief A third example of step which does not call XTIL functions
 * @ingroup steps_local
 */
int
step_local(
	uint8_t a,
	uint8_t b
	);

} // extern C

