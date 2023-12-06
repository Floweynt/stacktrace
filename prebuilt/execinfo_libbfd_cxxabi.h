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

#include <cxxabi.h>
#include <string>

namespace stacktrace
{
    namespace detail
    {
        inline void demangle(std::string& str)
        {
            int status;
            char* ptr = abi::__cxa_demangle(str.c_str(), 0, 0, &status);
            if (ptr)
            {
                str.assign(ptr);
                free(ptr);
            }
        }
    } // namespace detail
} // namespace stacktrace
#include <algorithm>
#include <execinfo.h>
#include <vector>

namespace stacktrace
{
    inline pointer_stacktrace stacktrace(size_t capture)
    {
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace stacktrace(capture);
        size_t size = backtrace((void**)stacktrace.data(), capture);
        stacktrace.resize(size);
        return stacktrace;
    }

    template <typename Callback>
    void signal_safe_stacktrace(Callback cb, size_t capture = MAX_CAPTURE_FRAMES)
    {
        static thread_local void* buffer[MAX_CAPTURE_FRAMES];
        size_t size = backtrace(buffer, std::min(capture, (size_t)MAX_CAPTURE_FRAMES));

        for (int i = 0; i < size; i++)
            cb((uintptr_t)buffer[i]);
    }
} // namespace stacktrace

#define PACKAGE 1
#define PACKAGE_VERSION 1
#include <bfd.h>
#undef PACKAGE
#undef PACKAGE_VERSION

#include <cstdio>
#include <dlfcn.h>
#include <link.h>

namespace stacktrace
{
    namespace detail
    {
        class libbfd_wrapper
        {
            bool is_valid = false;

            bfd* abfd = nullptr;
            asymbol** syms = nullptr;
            link_map* lm = nullptr;
            Dl_info info;
            bfd_vma pc;

            bool found;

            const char* filename;
            const char* functionname;
            unsigned int line;
            unsigned int discriminator;

        public:
            inline libbfd_wrapper()
            {
                bfd_init();

                abfd = bfd_openr("/proc/self/exe", nullptr);
                if (abfd == NULL)
                {
                    is_valid = false;
                    return;
                }

                if (!bfd_check_format(abfd, bfd_object))
                {
                    is_valid = false;
                    return;
                }

                abfd->flags |= BFD_DECOMPRESS;

                long storage;
                long symcount;
                bfd_boolean dynamic = false;

                if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
                    return;
                storage = bfd_get_symtab_upper_bound(abfd);
                if (storage == 0)
                {
                    storage = bfd_get_dynamic_symtab_upper_bound(abfd);
                    dynamic = true;
                }
                if (storage < 0)
                {
                    is_valid = false;
                    return;
                }
                syms = (asymbol**)malloc(storage);
                if (syms == nullptr)
                {
                    is_valid = false;
                    return;
                }

                if (dynamic)
                    symcount = bfd_canonicalize_dynamic_symtab(abfd, syms);
                else
                    symcount = bfd_canonicalize_symtab(abfd, syms);

                if (symcount < 0)
                {
                    is_valid = false;
                    return;
                }

                is_valid = true;
            }

            inline entry get_info(uintptr_t ptr)
            {
                if (!is_valid)
                    return entry(ptr, 0, "UNK", "UNK");

                // use DL_info to dump
                int status = dladdr1((const void*)ptr, &info, (void**)&lm, RTLD_DL_LINKMAP);
                if (status && info.dli_fname && info.dli_fname[0] != '\0')
                {
                    char buf[100];

                    info.dli_fbase = (void*)lm->l_addr;
                    info.dli_fbase = (void*)lm->l_addr;
                    if (info.dli_sname == NULL)
                        info.dli_saddr = info.dli_fbase;

                    if (info.dli_sname == NULL && info.dli_saddr == 0)
                        snprintf(buf, sizeof(buf), "%p", (void*)ptr);
                    else
                    {
                        char sign;
                        std::ptrdiff_t offset = ptr - (std::ptrdiff_t)info.dli_saddr;
                        sign = offset >= 0 ? '+' : '-';
                        offset = std::abs(offset);
                        snprintf(buf, sizeof(buf), "%c%#tx", sign, offset);
                    }
                    // turn +0x[offset] to symbols using libbfd
                    pc = bfd_scan_vma(buf, NULL, 16);
                    found = false;

                    bfd_map_over_sections(
                        abfd,
                        [](bfd* abfd, asection* section, void* args) {
                            bfd_vma vma;
                            bfd_size_type size;
                            libbfd_wrapper* that = (libbfd_wrapper*)args;

                            if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
                                return;

                            vma = bfd_section_vma(section);
                            if (that->pc < vma)
                                return;

                            size = bfd_section_size(section);
                            if (that->pc >= vma + size)
                                return;

                            that->found = bfd_find_nearest_line_discriminator(abfd, section, that->syms, that->pc - vma,
                                                                              &that->filename, &that->functionname,
                                                                              &that->line, &that->discriminator);
                        },
                        (void*)this);

                    if (!found)
                        return entry(ptr, 0, "UNK", "UNK");

                    filename = filename ? filename : "UNK";
                    functionname = functionname ? functionname : "UNK";

                    entry e(ptr, line, filename, functionname);
                    if (e.file.size() == 0)
                        e.file = "UNK";
                    if (e.function.size() == 0)
                        e.function = "UNK";
                    else
                        demangle(e.function);

                    return e;
                }

                return entry(ptr, 0, "UNK", "UNK");
            }

            ~libbfd_wrapper()
            {
                bfd_close(abfd);
            }
        };
    } // namespace detail

    inline symbol_stacktrace get_symbols(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret;
        static detail::libbfd_wrapper state;
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
