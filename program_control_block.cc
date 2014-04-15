
#include <program_control_block.h>

template class std::vector<pIntBarrier_t>;
template class std::shared_ptr<ProgramControlBlock>;

#if __WORDSIZE == 64
# define PRIu32 "%lu"
#else
# define PRIu32 "%u"
#endif

ProgramControlBlock::ProgramControlBlock(const char *name) :
	name(name), tid(0) {
	executor_thd = std::thread(&ProgramControlBlock::Executor, this);
	executor_thd.detach();
}

ProgramControlBlock::~ProgramControlBlock() {};

void ProgramControlBlock::AddInBarrier(pIntBarrier_t pb) {
	in_barriers.push_back(pb);
}

void ProgramControlBlock::AddOutBarrier(pIntBarrier_t pb) {
	out_barriers.push_back(pb);
}

bool ProgramControlBlock::Running() const {
	return running;
}

void ProgramControlBlock::Run() {
	std::unique_lock<std::mutex> ul(status_mtx);
	running = true;
	status_cv.notify_all();
	printf("[%10s] RUN\n", name);
}

void ProgramControlBlock::WaitCompletion() {
	std::unique_lock<std::mutex> ul(status_mtx);
	while(!done)
		status_cv.wait(ul);
	printf("[%10s] COMPLETED\n", name);
}

void ProgramControlBlock::Stop() {
	std::lock_guard<std::mutex> lg(status_mtx);
	done = true;
}

void ProgramControlBlock::WaitStart() {
	std::unique_lock<std::mutex> ul(status_mtx);
	while(!running)
		status_cv.wait(ul);
}

void ProgramControlBlock::WaitEnd() {
	std::unique_lock<std::mutex> ul(status_mtx);
	printf("Waiting end");
	while(!done)
		status_cv.wait(ul);
}

bool ProgramControlBlock::Done() const {
	return done;
}

bool ProgramControlBlock::Debug() const {
	return debug;
}

bool ProgramControlBlock::Repeat() const {
	return repeat;
}

uint16_t ProgramControlBlock::NextSteps() const {
	if (debug)
		return next_steps;
	return 1;
}

void ProgramControlBlock::DebugCheck(int step_id) {
	std::unique_lock<std::mutex> ul(dbg_mtx);

	// Debug disabled, nothing to do
	if (!debug)
		return;

	// Reset step repete mode
	repeat = false;

	// Enter DEBUG after a specified number of steps
	if (--next_steps == 0) {
		dbg_cv.wait(ul);
		return;
	}

	// Enter DEBUG at specified PCB step
	if (!pcb_steps)
		return;
	if (pcb_steps[step_id] == pcb_steps[step_out]) {
		dbg_cv.wait(ul);
		return;
	}

}

void ProgramControlBlock::DebugStart(uint16_t steps) {
	std::lock_guard<std::mutex> lg(dbg_mtx);
	debug = true;
	next_steps = debug_steps;
	if (steps > 1)
		next_steps = steps;
	dbg_cv.notify_one();
}

void ProgramControlBlock::DebugFrom(uint16_t step) {
	std::lock_guard<std::mutex> lg(dbg_mtx);
	if (step > pcb_steps_count)
		step = 0;
	debug = true;
	restart = true;
	SetStepInto(step);
	SetStepOut(step);
	dbg_cv.notify_one();
}

void ProgramControlBlock::SetStepInto(int16_t sin) {
	step_into = sin;
}

void ProgramControlBlock::SetStepOut(int16_t sout) {
	step_out = sout;
}

void ProgramControlBlock::DoStep(uint16_t steps) {
	std::lock_guard<std::mutex> lg(dbg_mtx);
	next_steps = debug_steps;
	if (steps > 1)
		next_steps = steps;
	dbg_cv.notify_one();
}

void ProgramControlBlock::DoRepeat() {
	std::lock_guard<std::mutex> lg(dbg_mtx);
	repeat = true;
	next_steps = 1;
	dbg_cv.notify_one();
}

void ProgramControlBlock::DebugEnd() {
	std::lock_guard<std::mutex> lg(dbg_mtx);
	debug = false;
	step_into = 0;
	step_out = -1;
	dbg_cv.notify_one();
}

uint8_t ProgramControlBlock::Setup() {
	printf("[%10s] onSetup, in: " PRIu32 ", out: " PRIu32 "\n",
			name, in_barriers.size(), out_barriers.size());
	return 0;
};

uint8_t ProgramControlBlock::Loop() {
	std::unique_lock<std::mutex> ul(dbg_mtx);
	printf("[%10s] onLoop\n", name);
	status_cv.wait_for(ul, std::chrono::seconds(5));
	return 0;
}

uint8_t ProgramControlBlock::Cleanup() {
	printf("[%10s] onClenaup\n", name);
	return 0;
}


uint8_t ProgramControlBlock::Executor() {
	std::unique_lock<std::mutex> ul(status_mtx, std::defer_lock);
	uint8_t ret = 0;

	// Initialize thread identifier
	tid = syscall(SYS_gettid);

	printf("[%10s] SETUP\n", name);
	Setup();
	WaitStart();
	printf("[%10s] STARTED\n", name);
	while (!Done()) {
		ret = Loop();
		if (ret)
			break;
	}
	Cleanup();
	ul.lock();
	done = true;
	status_cv.notify_all();
	printf("[%10s] TERMINATED\n", name);
	return ret;
}

