
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>
#include <iostream>

#include <cstdio>

#include <unistd.h>
#include <sys/syscall.h>


class ProgramControlBlock;
typedef std::shared_ptr<ProgramControlBlock> pcb_t;

class ProgramControlBlock {
public:

	ProgramControlBlock(const char *name = "undef") :
		name(name), tid(0) {
		executor_thd = std::thread(&ProgramControlBlock::Executor, this);
		executor_thd.detach();
	}

	virtual ~ProgramControlBlock() {};

	inline bool Running() const {
		return running;
	}

	inline void Run() {
		std::unique_lock<std::mutex> ul(status_mtx);
		running = true;
		status_cv.notify_all();
		printf("[%10s] RUN\n", name);
		while(!done)
			status_cv.wait(ul);
		printf("=== Run END ===\n");
	}

	inline void Stop() {
		std::lock_guard<std::mutex> lg(status_mtx);
		done = true;
	}

	inline void WaitStart() {
		std::unique_lock<std::mutex> ul(status_mtx);
		while(!running)
			status_cv.wait(ul);
	}

	inline void WaitEnd() {
		std::unique_lock<std::mutex> ul(status_mtx);
		printf("Waiting end");
		while(!done)
			status_cv.wait(ul);
	}

	inline bool Done() const {
		return done;
	}

	inline bool Debug() const {
		return debug;
	}

	inline bool Repeat() const {
		return repeat;
	}

	inline uint16_t NextSteps() const {
		if (debug)
			return next_steps;
		return 1;
	}

	inline void DebugCheck() {
		std::unique_lock<std::mutex> ul(dbg_mtx);
		if (!debug)
			return;
		repeat = false;
		if (--next_steps == 0)
			dbg_cv.wait(ul);
	}

	void DebugStart(uint16_t steps = 1) {
		std::lock_guard<std::mutex> lg(dbg_mtx);
		debug = true;
		next_steps = debug_steps;
		if (steps > 1)
			next_steps = steps;
		dbg_cv.notify_one();
	}

	inline void DoStep(uint16_t steps = 1) {
		std::lock_guard<std::mutex> lg(dbg_mtx);
		next_steps = debug_steps;
		if (steps > 1)
			next_steps = steps;
		dbg_cv.notify_one();
	}

	inline void DoRepeat() {
		std::lock_guard<std::mutex> lg(dbg_mtx);
		repeat = true;
		next_steps = 1;
		dbg_cv.notify_one();
	}

	void DebugEnd() {
		std::lock_guard<std::mutex> lg(dbg_mtx);
		debug = false;
		dbg_cv.notify_one();
	}

	virtual uint8_t Setup() {
		printf("Program SETUP\n");
		return 0;
	};
	virtual uint8_t Loop() {
		std::unique_lock<std::mutex> ul(dbg_mtx);
		printf("[%10s] onLoop\n", name);
		status_cv.wait_for(ul, std::chrono::seconds(5));
		return 0;
	}

	template<typename Sfnc, class ...Args>
	void Step(Sfnc sf, Args... args) {
		std::cout << ">>> Step wrapper BEGIN" << std::endl;
		do {
			sf(args...);
			DebugCheck();
		} while (Repeat());
		std::cout << "<<< Step wrapper  END" << std::endl;
	}

protected:

	const char *name;
	pid_t tid;

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

	uint8_t Executor() {
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
		ul.lock();
		done = true;
		status_cv.notify_all();
		printf("[%10s] TERMINATED\n", name);
		return ret;
	}

};

