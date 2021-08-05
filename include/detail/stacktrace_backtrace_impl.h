#ifndef __STACKTRACE_BACKTRACE_H__
#define __STACKTRACE_BACKTRACE_H__
#include <backtrace.h>
#include "demangler.h"
#include "../stacktrace_fwd.h"
namespace stacktrace
{
    namespace detail
    {
        class backtrace_wrapper
        {
            backtrace_state* state;
            std::string str;
        public:
            inline backtrace_wrapper()
            {
                state = backtrace_create_state("/proc/self/exe", 0, nullptr, this);
            }

            inline entry get_info(uintptr_t ptr)
            {
                if(!state)
                    return entry(ptr, 0, "UNK", "UNK");
                backtrace_syminfo(state, ptr, [](void* data, uintptr_t, const char* symname, uintptr_t, uintptr_t) {
                    backtrace_wrapper* that = (backtrace_wrapper*)data;
                    that->str = symname;
                    demangle(that->str);
                }, nullptr, this);

                return entry(ptr, 0, "UNK", str);
            }
        };
    }

    inline symbol_stacktrace get_traced(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret;
        static detail::backtrace_wrapper state;
        ret.reserve(trace.size());
        for(auto i : trace)
            ret.push_back(state.get_info(i));

        return ret;
    }
}
#endif
