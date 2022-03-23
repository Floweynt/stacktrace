#include "../../stacktrace_fwd.h"
#include <algorithm>
#include <execinfo.h>
#include <vector>

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace stacktrace(capture);
        size_t size = backtrace((void**)stacktrace.data(), capture);
        stacktrace.resize(size);
        return stacktrace;
    }

    template<typename Callback>
    void signal_safe_stacktrace(Callback cb, size_t capture = MAX_CAPTURE_FRAMES)
    {
        static thread_local void* buffer[MAX_CAPTURE_FRAMES];
        size_t size = backtrace(buffer, std::min(capture, (size_t)MAX_CAPTURE_FRAMES));

        for(int i = 0; i < size; i++)
            cb((uintptr_t)buffer[i]);
    }
} // namespace stacktrace
