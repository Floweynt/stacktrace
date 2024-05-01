#pragma once
#include "detail/common.h"
#include "stacktrace.h"
#include <ostream>
#include <stdexcept>
#include <string>

namespace stacktrace
{
    class stacktrace_exception : public std::runtime_error
    {
        symbol_stacktrace trace;

    public:
        INLINE explicit stacktrace_exception(const std::string& what) : runtime_error(what) { trace = get_symbols(stacktrace(100)); }

        INLINE auto get_stacktrace() const -> const symbol_stacktrace& { return trace; }
        friend auto operator<<(std::ostream& /*os*/, const stacktrace_exception& /*e*/) -> std::ostream&;
    };

    INLINE auto operator<<(std::ostream& stream, const stacktrace_exception& inst) -> std::ostream&
    {
        switch (stream.iword(detail::get_state()))
        {
        case 0:
        case 1:
            stream << inst.what();
            break;
        case 2:
            stream << "what: " << inst.what() << '\n';
            stream << "stacktrace: \n";
            dump_stacktrace(inst.trace, stream);
            break;
        default:
            break;
        }

        return stream;
    }
} // namespace stacktrace
