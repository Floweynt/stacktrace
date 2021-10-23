#include "../libbacktrace_handle.h"

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        return detail::backtrace_wrapper::get_state().get_stack();
    }
}