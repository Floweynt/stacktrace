#include "../../stacktrace_fwd.h"
#include "../common.h"
#include <cstddef>
#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace stacktrace
{
    INLINE auto stacktrace(size_t capture) -> pointer_stacktrace
    {
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace ret;
        unw_cursor_t cursor;
        unw_context_t context;
        unw_word_t addr;

        unw_getcontext(&context);
        unw_init_local(&cursor, &context);
        while (unw_step(&cursor) > 0 && capture--)
        {
            unw_get_reg(&cursor, UNW_REG_IP, &addr);
            ret.push_back(addr);
        }

        return ret;
    }

    template <typename Callback>
    INLINE void signal_safe_stacktrace(Callback callback, size_t capture = MAX_CAPTURE_FRAMES)
    {
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        unw_cursor_t cursor;
        unw_context_t context;
        unw_word_t addr;

        unw_getcontext(&context);
        unw_init_local(&cursor, &context);
        while (unw_step(&cursor) > 0 && capture--)
        {
            unw_get_reg(&cursor, UNW_REG_IP, &addr);
            callback(addr);
        }
    }
} // namespace stacktrace
