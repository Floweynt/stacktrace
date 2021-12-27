//
// Created by flowey on 12/27/21.
//

#ifndef STACKTRACE_SIMPLE_EXCEPTION_H
#define STACKTRACE_SIMPLE_EXCEPTION_H
#include <stacktrace.h>

namespace stacktrace
{
    class stacktrace_exception : std::runtime_error
    {
        symbol_stacktrace trace;
    public:
        inline explicit stacktrace_exception(const char* what) : runtime_error(what)
        {
            trace = get_traced(stacktrace(100));
        }

        inline const symbol_stacktrace& get_stacktrace() const { return trace; }
        friend std::ostream& operator<<(std::ostream&, const stacktrace_exception&);
    };

    inline std::ostream &operator<<(std::ostream& os, const stacktrace_exception & e)
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
    }
}

#endif //STACKTRACE_SIMPLE_EXCEPTION_H
