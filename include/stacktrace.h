#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include "config.h"
#include "stacktrace_fwd.h"

#include DEMANGLE_BACKEND
#include STACKTRACE_BACKEND
#include DECODE_BACKEND

namespace stacktrace
{
    namespace detail 
    {
        // ostream stuff
        inline int geti() {
            static int i = std::ios_base::xalloc();
            return i;
        }

        template<typename T>
        inline void print_hex(T t, std::ostream& os)
        {
            os << std::setw(sizeof(T) * 2) << std::setfill('0') << std::hex << t << std::setw(0) << std::setfill(' ') << std::dec;
        }

        inline void default_print(const entry& e, std::ostream& os, size_t num)
        {
            os << "#" << num << " [";
            print_hex(e.address, os);
            os << "] " << e.file << ':' << e.line << " (" << e.function << ')';
        };
    }

    // generates a raw stacktrace
    pointer_stacktrace stacktrace(size_t capture = -1U);
    symbol_stacktrace get_traced(const pointer_stacktrace& trace);
    
    stack_iterator begin();
    stack_iterator end();

    using stack_printer = void (*)(const entry&, std::ostream& os, size_t num);

    // base impl for dump_stacktrace
    inline void dump_stacktrace(const symbol_stacktrace& st, std::ostream& os = std::cout, stack_printer printer = detail::default_print)
    {
        for (int i = 0; i < st.size(); i++)
        {
            printer(st[i], os, i);
            os << '\n';
        }
    }

    inline void dump_stacktrace(size_t capture = -1U, std::ostream& os = std::cout, stack_printer printer = detail::default_print)
    {
        symbol_stacktrace st = get_traced(stacktrace(capture));
        dump_stacktrace(st, os, printer);
    }

    inline std::ostream& shortexcept(std::ostream& os) { os.iword(detail::geti()) = 0; return os; }
    inline std::ostream& longexcept(std::ostream& os) { os.iword(detail::geti()) = 1; return os; }
    inline std::ostream& stacktrace(std::ostream& os) { os.iword(detail::geti()) = 2; return os; }
}

#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__ 
#endif

#define rethrow catch(std::exception& e) { throw stacktrace::stack_aware_exception(e.what(), __LINE__, __FILE__, __func__, __PRETTY_FUNCTION__) }
#endif
