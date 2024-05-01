#pragma once
#include "config.h"
#include "detail/common.h"
#include "stacktrace_fwd.h"
#include <cstddef>
#include <iomanip>
#include <iostream>

#include DEMANGLE_BACKEND
#include STACKTRACE_BACKEND
#include DECODE_BACKEND

namespace stacktrace
{
    namespace detail
    {
        // ostream stuff
        inline auto get_state() -> int
        {
            static int index = std::ios_base::xalloc();
            return index;
        }

        template <typename T>
        inline void print_hex(T value, std::ostream& stream)
        {
            stream << std::setw(sizeof(T) * 2) << std::setfill('0') << std::hex << value << std::setw(0) << std::setfill(' ') << std::dec;
        }

        inline void default_print(const entry& entry, std::ostream& stream, size_t num)
        {
            stream << "#" << num << " [";
            print_hex(entry.address, stream);
            stream << "] " << entry.file << ':' << entry.line << " (" << entry.function << ')';
        };
    } // namespace detail

    // generates a raw stacktrace

    auto begin() -> stack_iterator;
    auto end() -> stack_iterator;

    using stack_printer = void (*)(const entry& entry, std::ostream& stream, size_t num);

    // base impl for dump_stacktrace
    inline void dump_stacktrace(const symbol_stacktrace& trace, std::ostream& stream = std::cout, stack_printer printer = detail::default_print)
    {
        for (int i = 0; i < trace.size(); i++)
        {
            printer(trace[i], stream, i);
            stream << '\n';
        }
    }

    inline void dump_stacktrace(size_t capture = -1U, std::ostream& steam = std::cout, stack_printer printer = detail::default_print)
    {
        auto stream = get_symbols(stacktrace(capture));
        dump_stacktrace(stream, steam, printer);
    }

    inline auto shortexcept(std::ostream& stream) -> std::ostream&
    {
        stream.iword(detail::get_state()) = 0;
        return stream;
    }

    inline auto longexcept(std::ostream& stream) -> std::ostream&
    {
        stream.iword(detail::get_state()) = 1;
        return stream;
    }

    inline auto stacktrace(std::ostream& stream) -> std::ostream&
    {
        stream.iword(detail::get_state()) = 2;
        return stream;
    }
} // namespace stacktrace
