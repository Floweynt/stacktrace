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
} // namespace stacktrace