#include "../libbacktrace_handle.h"

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        return detail::get_instance().get_stack();
    }
}