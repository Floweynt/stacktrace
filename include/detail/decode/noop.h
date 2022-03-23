#include "../../stacktrace_fwd.h"
namespace stacktrace
{
    inline symbol_stacktrace get_symbols(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret(trace.size());
        for (int i = 0; i < trace.size(); i++)
            ret[i].address = trace[i];
        return ret;
    }
} // namespace stacktrace
