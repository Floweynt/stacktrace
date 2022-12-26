#include "../../stacktrace_fwd.h"
#include <Windows.h>

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        capture = min((size_t)((USHORT)-1), capture);
        capture = min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace ret(capture);
        size_t size = CaptureStackBackTrace(0, capture, (void**)ret.data(), nullptr);
        ret.resize(size);
        return ret;
    }

    template<typename Callback>
    void signal_safe_stacktrace(Callback cb, size_t capture = MAX_CAPTURE_FRAMES)
    {
        capture = min((size_t)((USHORT)-1), capture);
        capture = min(capture, (size_t)MAX_CAPTURE_FRAMES);

        for (size_t i = 0; i < capture; i++)
        {
            uintptr_t buf;
            size_t size = CaptureStackBackTrace(i, 1, (void**) &buf, nullptr);
            if (size == 0)
                break;
            cb(buf);
        }
    }
} // namespace stacktrace