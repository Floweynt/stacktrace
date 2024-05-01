#include "../common.h"
#include "../libbacktrace_handle.h"
#include <cstddef>

namespace stacktrace
{
    INLINE auto stacktrace(size_t /*capture*/) -> pointer_stacktrace { return detail::get_instance().get_stack(); }

    template <typename Callback>
    INLINE void signal_safe_stacktrace(Callback callback, size_t /*capture*/ = MAX_CAPTURE_FRAMES)
    {
        detail::get_instance().get_with_callback(callback);
    }
} // namespace stacktrace
