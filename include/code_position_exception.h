#pragma once
#include "detail/common.h"
#include "stacktrace.h"
#include <ostream>
#include <stdexcept>

namespace stacktrace
{
    class code_position_exception : std::runtime_error
    {

        const char* file;
        const char* func;
        int line;

    public:
        INLINE explicit code_position_exception(
            const char* what, const char* file = __builtin_FILE(), const char* func = __builtin_FUNCTION(), int line = __builtin_LINE()
        )
            : runtime_error(what), file(file), func(func), line(line)
        {
        }

        friend auto operator<<(std::ostream& /*os*/, const code_position_exception& /*e*/) -> std::ostream&;
        INLINE auto get_file() const -> const char* { return file; }
        INLINE auto get_func() const -> const char* { return func; }
        INLINE auto get_line() const -> int { return line; }
    };

    INLINE auto operator<<(std::ostream& stream, const code_position_exception& instance) -> std::ostream&
    {
        switch (stream.iword(detail::get_state()))
        {
        case 0:
            stream << instance.what();
            break;
        case 2:
        case 1:
            stream << "what: " << instance.what() << '\n';
            stream << "at: " << instance.file << ":" << instance.line << " " << instance.func;
        default:
            break;
        }

        return stream;
    }
} // namespace stacktrace
