#ifndef __STACKTRACE_H__
#define __STACKTRACE_H__
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <functional>
#include "config.h"
#include "stacktrace_fwd.h"

#pragma message("Dumping backend info")

#if defined(USE_WINAPI_STACKTRACE)
#pragma message ("Using WINAPI")
#include "detail/stacktrace_windows_impl.h"
#elif defined(USE_EXECINFO_STACKTRACE)
#pragma message("Using EXECINFO")
#include "detail/stacktrace_execinfo_impl.h"
#endif

#if defined(USE_LIBBFD_DECODE)
#pragma message("Using LIBBFD")
#include "detail/stacktrace_libbfd_impl.h"
#endif

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

        inline void default_print(const entry& e, std::ostream& os)
        {
            os << "AT: [";
            print_hex(e.address, os);
            os << "] " << e.file << ':' << e.line << " (" << e.function << ')';
        };
    }

    // generates a raw stacktrace
    pointer_stacktrace stacktrace(size_t capture);
    symbol_stacktrace get_traced(const pointer_stacktrace& trace);

    using stack_printer = std::function<void(const entry&, std::ostream& os)>;

    // base impl for dump_stacktrace
    inline void dump_stacktrace(const symbol_stacktrace& st, stack_printer printer, std::ostream& os)
    {
        for (entry e : st)
        {
            printer(e, os);
            os << '\n';
        }
    }

    inline void dump_stacktrace(size_t capture, stack_printer printer, std::ostream& os)
    {
        symbol_stacktrace st = get_traced(stacktrace(capture));
        dump_stacktrace(st, printer, os);
    }

    inline void dump_stacktrace(size_t capture, std::ostream& os)
    {
        symbol_stacktrace st = get_traced(stacktrace(capture));
        dump_stacktrace(st, detail::default_print, os);
    }

    inline void dump_stacktrace(const symbol_stacktrace& st, std::ostream& os)
    {
        dump_stacktrace(st, detail::default_print, os);
    }

    class stack_aware_exception : std::runtime_error
    {
        symbol_stacktrace trace;
        int line;
        const char* file;
        const char* func;
        const char* long_func;
        std::string long_msg;

    public:
        inline stack_aware_exception(
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

        inline const char* get_file() const { return file; }
        inline const char* get_func() const { return func; }
        inline const char* get_long_func() const { return long_func; }
        inline int get_line() const { return line; }
        inline const symbol_stacktrace& get_stacktrace() const { return trace; }
        inline std::string get_long_msg() const { return long_msg; }
        
        friend std::ostream& operator<<(std::ostream& os, const stack_aware_exception& dt);
    };

    inline std::ostream& operator<<(std::ostream& os, const stack_aware_exception& e)
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

    inline std::ostream& shortexcept(std::ostream& os) { os.iword(detail::geti()) = 0; return os; }
    inline std::ostream& longexcept(std::ostream& os) { os.iword(detail::geti()) = 1; return os; }
    inline std::ostream& stacktrace(std::ostream& os) { os.iword(detail::geti()) = 2; return os; }
}

#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__ 
#endif

#define throw_dbg(exception, what, ...) throw exception(what, __LINE__, __FILE__, __func__, __PRETTY_FUNCTION__)
#endif
