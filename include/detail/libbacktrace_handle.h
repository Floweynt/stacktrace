#pragma once

#include "../stacktrace_fwd.h"
#include "common.h"
#include <backtrace.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace stacktrace
{
    namespace detail
    {
        class backtrace_wrapper
        {
            backtrace_state* state;

        public:
            INLINE backtrace_wrapper() : state(backtrace_create_state("/proc/self/exe", 0, nullptr, this)) {}

            INLINE auto get_stack() -> pointer_stacktrace
            {
                pointer_stacktrace trace;

                get_with_callback([&, trace](uintptr_t addr) mutable { trace.push_back(addr); });

                return trace;
            }

            template <typename Callback>
            void get_with_callback(Callback callback)
            {
                if (!state)
                {
                    return;
                }
                backtrace_simple(
                    state, 0,
                    [](void* buf, uintptr_t trace) {
                        (*((Callback*)buf))(trace);
                        return 0;
                    },
                    [](void* buf, const char*, int) { (*((Callback*)buf))(0x0); }, &callback
                );
            }

            INLINE auto get_info(uintptr_t ptr) -> entry
            {
                entry ent{ptr, 0, "UNK", "UNK"};
                if (state == nullptr)
                {
                    return ent;
                }

                backtrace_pcinfo(
                    state, ptr,
                    [](void* data, uintptr_t addr, const char* file_in, int lineno, const char* func_in) {
                        auto* buf = (entry*)data;
                        std::string file{file_in != nullptr ? file_in : "UNK"};
                        std::string func{func_in != nullptr ? func_in : "UNK"};

                        if (file.empty())
                        {
                            file = "UNK";
                        }
                        if (func.empty())
                        {
                            func = "UNK";
                        }
                        else
                        {
                            demangle(func);
                        }

                        *buf = {addr, (size_t)lineno, file, func};
                        return 0;
                    },
                    nullptr, &ent
                );

                return ent;
            }
        };

        INLINE auto get_instance() -> backtrace_wrapper
        {
            static auto* ptr = new backtrace_wrapper();
            return *ptr;
        }
    } // namespace detail
} // namespace stacktrace
