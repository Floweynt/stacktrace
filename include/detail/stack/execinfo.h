#include "../../stacktrace_fwd.h"
#include "../common.h"
#include <algorithm>
#include <cstddef>
#include <execinfo.h>

namespace stacktrace
{
    INLINE auto stacktrace(size_t capture) -> pointer_stacktrace
    {
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace stacktrace(capture);
        size_t size = backtrace(as_ptr(stacktrace.data()), capture);
        stacktrace.resize(size);
        return stacktrace;
    }

    template <typename Callback>
    INLINE void signal_safe_stacktrace(Callback callback, size_t capture = MAX_CAPTURE_FRAMES)
    {
        static thread_local void* buffer[MAX_CAPTURE_FRAMES];
        size_t size = backtrace(decay_arr(buffer), std::min(capture, (size_t)MAX_CAPTURE_FRAMES));

        for (int i = 0; i < size; i++)
        {
            callback(as_uptr(buffer[i]));
        }
    }
} // namespace stacktrace
