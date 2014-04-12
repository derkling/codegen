
#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <cstdio>
#include <stdint.h>
#include <dlfcn.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <rapidxml.hpp>

#include <demo_core.h>
#include <demo_steps.h>
#include <sync_barrier.h>

// Pre-allocate SyncBarrier templates
SyncBarrier<int *> IntBarrier(nullptr, nullptr, "__pcIntBarrier", 0);

