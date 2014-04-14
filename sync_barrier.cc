
#include <sync_barrier.h>

// Explicit specialization
template class SyncBarrier<int *>;

typedef SyncBarrier<int*> IntBarrier_t;
typedef std::shared_ptr<IntBarrier_t> pIntBarrier_t;

