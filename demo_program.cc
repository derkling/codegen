
#include <stdint.h>
#include <stdio.h>

#include "demo_core.h"
#include "demo_steps.h"


class MyProgram : public ProgramControlBlock {
	uint8_t Setup();
	uint8_t Loop();
	uint8_t Cleanup();
};

uint8_t MyProgram::Setup() {

// #include "demo_program_setup.cc"

	return 0;
}

uint8_t MyProgram::Cleanup() {

// #include "demo_program_cleanup.cc"

	return 0;
}

uint8_t MyProgram::Loop() {
	static uint8_t loops = 5;
	printf("Loops remaining %d\n", loops--);
	if (!loops)
		return 1;

	DebugCheck();

////////////////////////////////////////////////////////////////////////////////
// [BEGIN] GENERATED CODE
////////////////////////////////////////////////////////////////////////////////


#include "demo_program_steps.cc"


////////////////////////////////////////////////////////////////////////////////
// [END] GENERATED CODE
////////////////////////////////////////////////////////////////////////////////

	return 0;
}

class MyIncLoop : public ProgramControlBlock {
public:
	MyIncLoop(const char *name = "IntAdder") :
		ProgramControlBlock(name) {};
private:
	uint8_t Loop();
	uint8_t loops = 5;
};

uint8_t MyIncLoop::Loop() {
	printf("[%10s] Loops remaining %d\n", name, loops--);
	if (!loops)
		return 1;

	DebugCheck();

////////////////////////////////////////////////////////////////////////////////
// Consumer Buffers PULL

	// Input buffers
	int *bi1 = nullptr;
	int *bi2 = nullptr;

	if (in_barriers.size() > 0) // Left and Right Adders
		bi1 = in_barriers[0]->pull_buffer(name, tid);
	if (in_barriers.size() > 1) // Sink
		bi2 = in_barriers[1]->pull_buffer(name, tid);

////////////////////////////////////////////////////////////////////////////////
// Processing STEPS

	// Output buffer
	int *bo0 = nullptr;
	// Pipelne check value
	int result;

	if (out_barriers.size() > 0) // All but Sink
		bo0 = out_barriers[0]->producerBuffer();

	switch (in_barriers.size()) {
	case 0:	// Source
		result = loops;
		printf("[%10s] P: produce %d @ %p\n",
				name, result, bo0);
		if (bo0)
			bo0[0] = result;
		break;
	case 1: // Left and Right Adders
		result = bi1[0] + 1;
		printf("[%10s] A: add 1 to %d, produce %d @ %p\n",
				name, bi1[0], result, bo0);
		if (bo0)
			bo0[0] = result;
		break;
	default: // Sink
		result = bi1[0] - bi2[0];
		fprintf(stderr, "[%10s] Check result (%d == 0): %s\n",
				name, result, (result != 0) ? "FAILED" : "Success");
		break;
	}

////////////////////////////////////////////////////////////////////////////////
// Producer Buffers PUSH

	if (out_barriers.size() > 0) // All but Sink
		out_barriers[0]->push_buffer(name);

	return 0;
}


extern "C" {

void program_entry() {

#ifdef EXAMPLE_NETWORK

	// Pre-Allocated Int (double) buffers
	int buffers[5][2] = {
		{0,0}, {0,0}, {0,0}, {0,0}, {0,0}};

	// Source Adder
	pcb_t pcbSource = pcb_t(new MyIncLoop("Source"));
	pIntBarrier_t source = pIntBarrier_t(new IntBarrier_t(buffers[0], buffers[0]+1, "S-LR", 2));
	pcbSource->AddOutBarrier(source);

	// Left Adder 1
	pcb_t pcbL1 = pcb_t(new MyIncLoop("LeftA1"));
	pcbL1->AddInBarrier(source);
	pIntBarrier_t lb1 = pIntBarrier_t(new IntBarrier_t(buffers[1], buffers[1]+1, "LA1-LA2"));
	pcbL1->AddOutBarrier(lb1);

	// Left Adder 2
	pcb_t pcbL2 = pcb_t(new MyIncLoop("LeftA2"));
	pcbL2->AddInBarrier(lb1);
	pIntBarrier_t lb2 = pIntBarrier_t(new IntBarrier_t(buffers[2], buffers[2]+1, "LA2-S"));
	pcbL2->AddOutBarrier(lb2);

	// Right Adder 1
	pcb_t pcbR1 = pcb_t(new MyIncLoop("RightA1"));
	pcbR1->AddInBarrier(source);
	pIntBarrier_t rb1 = pIntBarrier_t(new IntBarrier_t(buffers[3], buffers[3]+1, "RA1-RA2"));
	pcbR1->AddOutBarrier(rb1);

	// Right Adder 2
	pcb_t pcbR2 = pcb_t(new MyIncLoop("RightA2"));
	pcbR2->AddInBarrier(rb1);
	pIntBarrier_t rb2 = pIntBarrier_t(new IntBarrier_t(buffers[4], buffers[4]+1, "RA2-S"));
	pcbR2->AddOutBarrier(rb2);

	// Sink Adder
	pcb_t pcbSink = pcb_t(new MyIncLoop("Sink"));
	pcbSink->AddInBarrier(lb2);
	pcbSink->AddInBarrier(rb2);

	// Start pipeline (from sink to source)
	pcbSink->Run();
	pcbR2->Run();
	pcbL2->Run();
	pcbR1->Run();
	pcbL1->Run();
	pcbSource->Run();

	// Wait completion (from source to sink)
	pcbSource->WaitCompletion();
	pcbL1->WaitCompletion();
	pcbR1->WaitCompletion();
	pcbL2->WaitCompletion();
	pcbR2->WaitCompletion();
	pcbSink->WaitCompletion();

#else // EXAMPLE_NETWORK

	// Singl PCB example
	pcb_t pcb2 = pcb_t(new MyProgram());
	pcb2->Run();
	pcb2->WaitCompletion();

#endif // EXAMPLE_NETWORK

}

}

