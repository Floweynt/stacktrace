#include "../libbacktrace_handle.h"
#include <backtrace.h>
namespace stacktrace
{
    inline symbol_stacktrace get_symbols(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret;
        ret.reserve(trace.size());
        for (auto i : trace)
            ret.push_back(detail::get_instance().get_info(i));

        return ret;
    }

} // namespace stacktrace
