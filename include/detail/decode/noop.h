#include "../../stacktrace_fwd.h"
#include "../common.h"

namespace stacktrace
{
    INLINE auto get_symbols(const pointer_stacktrace& trace) -> symbol_stacktrace
    {
        symbol_stacktrace ret(trace.size());
        for (int i = 0; i < trace.size(); i++)
        {
            ret[i].address = trace[i];
        }

        return ret;
    }
} // namespace stacktrace
