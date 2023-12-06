#ifndef __STACKTRACE_SINGLE_HEADER__

#define __STACKTRACE_SINGLE_HEADER__

#define MAX_CAPTURE_FRAMES 100

#include <cstdint>
#include <string>
#include <vector>

namespace stacktrace
{
    struct entry
    {
        entry() : line(0), address(0), file("UNK"), function("UNK")
        {
        }
        entry(uintptr_t address, size_t line, std::string file, std::string function)
            : address(address), line(line), file(file), function(function)
        {
        }

        uintptr_t address;
        size_t line;
        std::string file;
        std::string function;
    };

    using symbol_stacktrace = std::vector<entry>;
    using pointer_stacktrace = std::vector<uintptr_t>;
    using stacktrace_callback = void (*)(uintptr_t);

    class stack_iterator;
} // namespace stacktrace

#include <string>

namespace stacktrace
{
    namespace detail
    {
        inline void demangle(std::string& str)
        {
        }
    } // namespace detail
} // namespace stacktrace
#include <Windows.h>

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        capture = min((size_t)((USHORT)-1), capture);
        capture = min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace ret(capture);
        size_t size = CaptureStackBackTrace(0, capture, (void**)ret.data(), nullptr);
        ret.resize(size);
        return ret;
    }

    template <typename Callback>
    void signal_safe_stacktrace(Callback cb, size_t capture = MAX_CAPTURE_FRAMES)
    {
        capture = min((size_t)((USHORT)-1), capture);
        capture = min(capture, (size_t)MAX_CAPTURE_FRAMES);

        for (size_t i = 0; i < capture; i++)
        {
            uintptr_t buf;
            size_t size = CaptureStackBackTrace(i, 1, (void**)&buf, nullptr);
            if (size == 0)
                break;
            cb(buf);
        }
    }
} // namespace stacktrace
#include <DbgHelp.h>
#include <Windows.h>

#pragma comment(lib, "DbgHelp.lib")

namespace stacktrace
{
    namespace detail
    {
        class winapi_wrapper
        {
            static constexpr auto MAX_NAME_LEN = 1023;
            alignas(SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + MAX_NAME_LEN + 1]{};
            SYMBOL_INFO* symbol = new (buf) SYMBOL_INFO;
            IMAGEHLP_LINE line;
            HANDLE process;
            bool is_valid;

        public:
            inline winapi_wrapper()
            {
                process = GetCurrentProcess();
                if (!SymInitialize(process, nullptr, true))
                {
                    is_valid = false;
                    return;
                }

                symbol->MaxNameLen = MAX_NAME_LEN;
                symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                is_valid = true;
            }

            inline entry get_info(uintptr_t ptr)
            {
                entry e{ptr, 0, "UNK", "UNK"};
                DWORD tmp;

                if (!is_valid)
                    return e;
                if (SymFromAddr(process, (DWORD64)ptr, 0, symbol))
                    e.function = symbol->Name;
                if (SymGetLineFromAddr(process, (DWORD64)ptr, &tmp, &line))
                {
                    e.file = line.FileName;
                    e.line = line.LineNumber;
                }

                return e;
            }
        };
    } // namespace detail

    inline symbol_stacktrace get_symbols(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret;
        static detail::winapi_wrapper state;
        ret.reserve(trace.size());
        for (auto i : trace)
            ret.push_back(state.get_info(i));

        return ret;
    }
} // namespace stacktrace

#include <iomanip>
#include <iostream>
#include <stdexcept>

namespace stacktrace
{
    namespace detail
    {
        // ostream stuff
        inline int geti()
        {
            static int i = std::ios_base::xalloc();
            return i;
        }

        template <typename T>
        inline void print_hex(T t, std::ostream& os)
        {
            os << std::setw(sizeof(T) * 2) << std::setfill('0') << std::hex << t << std::setw(0) << std::setfill(' ')
               << std::dec;
        }

        inline void default_print(const entry& e, std::ostream& os, size_t num)
        {
            os << "#" << num << " [";
            print_hex(e.address, os);
            os << "] " << e.file << ':' << e.line << " (" << e.function << ')';
        };
    } // namespace detail

    // generates a raw stacktrace
    pointer_stacktrace stacktrace(size_t capture = -1U);
    symbol_stacktrace get_symbols(const pointer_stacktrace& trace);

    stack_iterator begin();
    stack_iterator end();

    using stack_printer = void (*)(const entry&, std::ostream& os, size_t num);

    // base impl for dump_stacktrace
    inline void dump_stacktrace(const symbol_stacktrace& st, std::ostream& os = std::cout,
                                stack_printer printer = detail::default_print)
    {
        for (int i = 0; i < st.size(); i++)
        {
            printer(st[i], os, i);
            os << '\n';
        }
    }

    inline void dump_stacktrace(size_t capture = -1U, std::ostream& os = std::cout,
                                stack_printer printer = detail::default_print)
    {
        symbol_stacktrace st = get_symbols(stacktrace(capture));
        dump_stacktrace(st, os, printer);
    }

    inline std::ostream& shortexcept(std::ostream& os)
    {
        os.iword(detail::geti()) = 0;
        return os;
    }
    inline std::ostream& longexcept(std::ostream& os)
    {
        os.iword(detail::geti()) = 1;
        return os;
    }
    inline std::ostream& stacktrace(std::ostream& os)
    {
        os.iword(detail::geti()) = 2;
        return os;
    }
} // namespace stacktrace

namespace stacktrace
{
    class code_position_exception : std::runtime_error
    {

        const char* file;
        const char* func;
        int line;

    public:
        inline explicit code_position_exception(const char* what, const char* file = __builtin_FILE(),
                                                const char* func = __builtin_FUNCTION(), int line = __builtin_LINE())
            : runtime_error(what), file(file), func(func), line(line)
        {
        }

        friend std::ostream& operator<<(std::ostream&, const code_position_exception&);
        inline const char* get_file() const
        {
            return file;
        }
        inline const char* get_func() const
        {
            return func;
        }
        inline int get_line() const
        {
            return line;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const code_position_exception& e)
    {
        int i = os.iword(detail::geti());
        switch (i)
        {
        case 0:
            os << e.what();
            break;
        case 2:
        case 1:
            os << "what: " << e.what() << '\n';
            os << "at: " << e.file << ":" << e.line << " " << e.func;
        }

        return os;
    }
} // namespace stacktrace

//__CODE_POSITION_EXCEPTION_H__

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

// STACKTRACE_SIMPLE_EXCEPTION_H
#endif
