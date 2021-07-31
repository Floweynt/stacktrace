#include <iostream>
#include <iomanip>
#include <stdexcept>
#include "stacktrace.h"

namespace stacktrace 
{
    namespace detail
    {
        template<typename T>
        inline void print_hex(T t, std::ostream& os)
        {
            os << std::setw(sizeof(T) * 2) << std::setfill('0') << std::hex << t << std::setw(0) << std::setfill(' ') << std::dec;
        }

        void default_print(const entry& e, std::ostream& os)
        {
            os << "AT: [";
            print_hex(e.address, os);
            os << "] " << e.file << ':' << e.line << " (" << e.function << ')';
        };
    }
    stack_aware_exception::stack_aware_exception(
        const char* what,
        int line,
        const char* file,
        const char* func,
        const char* long_func
    ) : runtime_error(what), line(line), file(file), func(func), long_func(long_func)
    {
        long_msg += "MESSAGE: ";
        long_msg += what;
        long_msg += "\nAT FUNC: ";
        long_msg += long_func;
        long_msg += "\nAT FILE: ";
        long_msg += file;
        long_msg += ":";
        long_msg += std::to_string(line);
        long_msg += " (";
        long_msg += func;
        long_msg += ')';
        trace = get_traced(stacktrace(100));
    }

    std::ostream& operator<<(std::ostream& os, const stack_aware_exception& e)
    {
        int state = os.iword(detail::geti());
        switch (state)
        {
        case 0:
            os << e.what();
            break;
        case 1:
            os << e.get_long_msg();
            break;
        case 2:
            auto flags = os.flags();
            os.clear();

            os << e.get_long_msg() << "\n\n";
            os << "STACKTRACE:" << "\n";
            dump_stacktrace(e.get_stacktrace(), os);

            os.setf(flags);
            break;
        }

        return os;
    } 
}
