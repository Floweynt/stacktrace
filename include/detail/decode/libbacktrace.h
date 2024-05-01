#include "../libbacktrace_handle.h"
#include "stacktrace_fwd.h"
#include <backtrace.h>
#include "../common.h"

namespace stacktrace
{
    INLINE auto get_symbols(const pointer_stacktrace& trace) -> symbol_stacktrace
    {
        symbol_stacktrace ret;
        ret.reserve(trace.size());
        for (auto entry : trace)
        {
            ret.push_back(detail::get_instance().get_info(entry));
        }

        return ret;
    }
} // namespace stacktrace
