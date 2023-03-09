#ifndef __STACKTRACE_SIMPLE_EXCEPTION_H__
#define __STACKTRACE_SIMPLE_EXCEPTION_H__
#include <stacktrace.h>

namespace stacktrace
{
    class stacktrace_exception : public std::runtime_error
    {
        symbol_stacktrace trace;

    public:
        inline explicit stacktrace_exception(const std::string& what) : runtime_error(what)
        {
            trace = get_symbols(stacktrace(100));
        }

        inline const symbol_stacktrace& get_stacktrace() const
        {
            return trace;
        }
        friend std::ostream& operator<<(std::ostream&, const stacktrace_exception&);
    };

    inline std::ostream& operator<<(std::ostream& os, const stacktrace_exception& e)
    {
        int i = os.iword(detail::geti());
        switch (i)
        {
        case 0:
        case 1:
            os << e.what();
            break;
        case 2:
            os << "what: " << e.what() << '\n';
            os << "stacktrace: ";
            dump_stacktrace(e.trace, os);
        }

        return os;
    }
} // namespace stacktrace

#endif // STACKTRACE_SIMPLE_EXCEPTION_H
