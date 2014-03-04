
#include <stdint.h>
#include <stdio.h>

#include "demo_core.h"
#include "demo_steps.h"


class MyProgram : public ProgramControlBlock {
	uint8_t Setup();
	uint8_t Loop();
};

uint8_t MyProgram::Setup() {

	return 0;
}

uint8_t MyProgram::Loop() {
	static uint8_t loops = 5;
	printf("Loops remaining %d\n", loops--);
	if (!loops)
		return 1;

////////////////////////////////////////////////////////////////////////////////
// [BEGIN] GENERATED CODE
////////////////////////////////////////////////////////////////////////////////

	DebugCheck();

	do {
		step_xtil_function1(0, 0, 0, 1024, 768);
		DebugCheck();
	} while (Repeat());

	do {
		step_xtil_function2(0, 0, 0, 1024, 768);
		DebugCheck();
	} while(Repeat());

////////////////////////////////////////////////////////////////////////////////
// [END] GENERATED CODE
////////////////////////////////////////////////////////////////////////////////

	return 0;
}

extern "C" {

void program_entry() {
	pcb_t pcb1 = pcb_t(new MyProgram());
	pcb1->Run();
}

}

