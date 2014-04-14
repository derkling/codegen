
#ifndef PROGRAM_CONTROL_BLOCK_H
#define PROGRAM_CONTROL_BLOCK_H

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include <cstdio>

#include <unistd.h>
#include <sys/syscall.h>

#include <sync_barrier.h>

class ProgramControlBlock {
public:

	ProgramControlBlock(const char *name = "undef");
	virtual ~ProgramControlBlock();

	void AddInBarrier(pIntBarrier_t pb);
	void AddOutBarrier(pIntBarrier_t pb);
	bool Running() const;
	void Run();
	void WaitCompletion();
	void Stop();
	void WaitStart();
	void WaitEnd();
	bool Done() const;
	bool Debug() const;
	bool Repeat() const;
	uint16_t NextSteps() const;
	void DebugCheck();
	void DebugStart(uint16_t steps = 1);
	void DoStep(uint16_t steps = 1);
	void DoRepeat();
	void DebugEnd();

	virtual uint8_t Setup();
	virtual uint8_t Loop();
	virtual uint8_t Cleanup();

	template<typename Sfnc, class ...Args>
	void Step(Sfnc sf, Args... args) {
		do {
			sf(args...);
			DebugCheck();
		} while (Repeat());
	}

protected:

	const char *name;
	pid_t tid;

	std::vector<pIntBarrier_t> in_barriers;
	std::vector<pIntBarrier_t> out_barriers;

private:

	bool running = false;
	bool done = false;
	std::condition_variable status_cv;
	std::mutex status_mtx;

	bool debug = false;
	bool repeat = false;
	uint16_t debug_steps = 1;
	uint16_t next_steps;
	std::condition_variable dbg_cv;
	std::mutex dbg_mtx;

	std::thread executor_thd;

	uint8_t Executor();

};

typedef std::shared_ptr<ProgramControlBlock> pcb_t;

#endif /* end of include guard: PROGRAM_CONTROL_BLOCK_H */

