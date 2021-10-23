#include <backtrace.h>
#include "demangler.h"
#include "../stacktrace_fwd.h"

namespace stacktrace
{
    namespace detail
    {
        class backtrace_wrapper
        {
            static backtrace_wrapper* instance;

            backtrace_state* state;
        public:
            inline backtrace_wrapper()
            {
                state = backtrace_create_state("/proc/self/exe", 0, nullptr, this);
            }

            inline pointer_stacktrace get_stack()
            {
                if (!state)
                    return {};

                pointer_stacktrace st;

                backtrace_simple(state, 0, [](void* buf, uintptr_t pc) {
                    ((pointer_stacktrace*)buf)->push_back(pc);
                }, [](void* data, const char*, int) {
                    ((pointer_stacktrace*)buf)->push_back(0x0);
                }, (void*)&st);

                return st;
            }

            inline entry get_info(uintptr_t ptr)
            {
                entry e{ ptr, 0, "UNK", "UNK" };
                if (!state)
                    return e;

                backtrace_syminfo(state, ptr, [ptr](void* data, uintptr_t, const char* symname, uintptr_t, uintptr_t) {
                    entry* buf = (entry*)data;
                    *buf = { ptr, 0, "UNK", str };
                }, nullptr, &e);

                return e;
            }

            static inline backtrace_wrapper& get_state()
            {
                return instance;
            }
        };
    }
}