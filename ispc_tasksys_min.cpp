// Minimal single-thread task runtime for ISPC (matches stdlib tasking API)
#include <cstdint>
#include <cstdlib>
#include <algorithm>

#if defined(_MSC_VER)
  #include <malloc.h>
  static inline void* aligned_alloc_portable(std::size_t a, std::size_t s) { return _aligned_malloc(s, a); }
  static inline void  aligned_free_portable(void* p) { _aligned_free(p); }
#else
  #include <cstring>
  static inline void* aligned_alloc_portable(std::size_t a, std::size_t s) {
      void* p = nullptr;
      if (posix_memalign(&p, a, s) != 0) return nullptr;
      return p;
  }
  static inline void  aligned_free_portable(void* p) { free(p); }
#endif

extern "C" {

// Task function type used by ISPC-generated code:
//   void f(void* data, int threadIndex, int threadCount, int taskIndex, int taskCount);
typedef void (*ISPCTaskFunc)(void*, int, int, int, int);

// Alloc aligned memory for ISPC
void* ISPCAlloc(void* /*context*/, int64_t size, int32_t alignment) {
    if (size <= 0) return nullptr;
    if (alignment < static_cast<int32_t>(alignof(void*))) alignment = static_cast<int32_t>(alignof(void*));
    return aligned_alloc_portable(static_cast<std::size_t>(alignment),
                                  static_cast<std::size_t>(size));
}

// Launch tasks (single-thread: run all tasks sequentially)
void ISPCLaunch(void **handle, ISPCTaskFunc f, void *data,
                int count0, int count1, int count2) {
    if (handle) *handle = nullptr;     // no async handle in this minimal runtime
    if (!f) return;

    int dim0 = std::max(count0, 1);
    int dim1 = std::max(count1, 1);
    int dim2 = std::max(count2, 1);
    int tasks = dim0 * dim1 * dim2;

    for (int i = 0; i < tasks; ++i) {
        // single thread => (threadIndex=0, threadCount=1)
        f(data, /*threadIndex*/0, /*threadCount*/1, /*taskIndex*/i, /*taskCount*/tasks);
    }
}

// Wait for tasks (no-op for single-thread)
void ISPCSync(void* /*handle*/) {}

} // extern "C"
