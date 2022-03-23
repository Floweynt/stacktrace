#include "../libbacktrace_handle.h"

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        return detail::get_instance().get_stack();
    }

    template<typename Callback>
    void signal_safe_stacktrace(Callback cb, size_t capture = MAX_CAPTURE_FRAMES)
    {
        detail::get_instance().get_with_callback(cb);
    }
} // namespace stacktrace
