
#ifndef SYNC_BARRIER_H
#define SYNC_BARRIER_H

#include <condition_variable>
#include <memory>
#include <mutex>

#include <unistd.h>

template <typename T>
class SyncBarrier {
private:
	T barrier[2];
	bool ready[2] = {false, false};
	int producer_id = 0;
	int consumer_id = 0;

	// The number of consumers
	const int ccount;
	// Number of consumer still to be served
	int pending;
	// The PID of server consumers
	pid_t *cserved;
	// Number of consumers using the output buffer
	pid_t *lserved;
	// The PID 0 is reserved to make undefined consumer thread;
#define CONSUMER_NONE 0
	// A string identifier for this barrier
	const char *name;

	std::mutex barrier_mtx;
	std::condition_variable barrier_cv;

public:
	SyncBarrier(T producer, T consumer, const char *name, int ccount = 1) :
		ccount(ccount),
		pending(ccount),
		name(name) {
		barrier[0] = producer;
		barrier[1] = consumer;
		cserved = new pid_t[ccount];
		lserved = cserved;
		*lserved = CONSUMER_NONE;
	}

	T push_buffer(const char *caller = nullptr) {
		std::unique_lock<std::mutex> ul(barrier_mtx);
		printf("[%10s] P: %-10s => push buffer %d @ %p\n",
				name, caller ? caller : "?",
				producer_id, barrier[producer_id]);
		// Mark current buffer as ready
		ready[producer_id] = true;
		// Switch to next buffer to fill
		producer_id = (producer_id + 1) % 2;
		printf("[%10s] P: %-10s notify C (p:%d, c:%d)\n",
				name, caller ? caller : "?",
				producer_id, consumer_id);
		// Notify all consumers that a new buffer is available
		barrier_cv.notify_all();
		// Wait for consumers to release next buffer
		while (ready[producer_id])
			barrier_cv.wait(ul);
		printf("[%10s] P: %-10s <= get buffer %d @ %p\n",
				name, caller ? caller : "?",
				producer_id, barrier[producer_id]);
		// Return next output buffer
		return barrier[producer_id];
	}

	T pull_buffer(const char *caller = nullptr, pid_t tid = 0) {
		std::unique_lock<std::mutex> ul(barrier_mtx);

		// Release the buffer if we are the last consumer
		if (pending == 0) {
			printf("[%10s] C: %-10s => put buffer %d @ %p\n",
					name, caller ? caller : "?",
					consumer_id, barrier[consumer_id]);

			// Mark current buffer as ready
			ready[consumer_id] = false;
			//... switch to new buffer to get
			consumer_id = (consumer_id + 1) % 2;
			// Reset consumers
			resetConsumerServed();
			// Notify producer a new buffer has been released
			printf("[%10s] C: %-10s notify P (p:%d, c:%d)\n",
					name, caller ? caller : "?",
					producer_id, consumer_id);
			barrier_cv.notify_all();
		}

		// Wait for producer to prepare next buffer
		while (!ready[consumer_id])
			barrier_cv.wait(ul);

		// Wait for other consumers
		while (isConsumerServed(tid))
			barrier_cv.wait(ul);

		// Mark current buffer as consumed
		setConsumerServed(tid);
		printf("[%10s] C: %-10s <= pull buffer %d @ %p (%d pulls pending)\n",
			name, caller ? caller : "?",
			consumer_id, barrier[consumer_id], pending);

		// Return next input buffer
		return barrier[consumer_id];
	}

	inline T producerBuffer() const {
		return barrier[producer_id];
	}

	inline T consumerBuffer() const {
		return barrier[consumer_id];
	}

private:

	inline bool isConsumerServed(pid_t tid) {
		int i = ccount - pending;
		while (i >= 0 && cserved[i] != tid)
			--i;
		return (i >= 0);
	}

	inline void setConsumerServed(pid_t tid) {
		*lserved = tid;
		lserved += 1;
		pending -= 1;
		if (pending)
			*lserved = CONSUMER_NONE;
	}

	inline void resetConsumerServed() {
		// Reload consumers number
		pending = ccount;
		// Reset vector of served consumers
		lserved = cserved;
		*lserved = CONSUMER_NONE;
	}

};

// Supported specializations
extern template class SyncBarrier<int *>;
typedef SyncBarrier<int*> IntBarrier_t;
typedef std::shared_ptr<IntBarrier_t> pIntBarrier_t;

#endif /* end of include guard: SYNC_BARRIER_H */

