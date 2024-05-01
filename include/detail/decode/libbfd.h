#include "../../stacktrace_fwd.h"
#include "../common.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <link.h>

#define PACKAGE 1
#define PACKAGE_VERSION 1
#include <bfd.h>
#undef PACKAGE
#undef PACKAGE_VERSION

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
            Dl_info info{};
            bfd_vma pc{};

            bool found{};

            const char* filename{};
            const char* functionname{};
            unsigned int line{};
            unsigned int discriminator{};

        public:
            inline libbfd_wrapper()
            {
                bfd_init();

                abfd = bfd_openr("/proc/self/exe", nullptr);
                if (abfd == nullptr)
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

                long storage = 0;
                long symcount = 0;
                bfd_boolean dynamic = false;

                if ((bfd_get_file_flags(abfd) & HAS_SYMS) == 0)
                {
                    return;
                }

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

                syms = as_ptr(malloc(storage));
                if (syms == nullptr)
                {
                    is_valid = false;
                    return;
                }

                if (dynamic)
                {
                    symcount = bfd_canonicalize_dynamic_symtab(abfd, syms);
                }
                else
                {
                    symcount = bfd_canonicalize_symtab(abfd, syms);
                }

                if (symcount < 0)
                {
                    is_valid = false;
                    return;
                }

                is_valid = true;
            }

            inline auto get_info(uintptr_t ptr) -> entry
            {
                if (!is_valid)
                {
                    return {ptr, 0, "UNK", "UNK"};
                }

                // use DL_info to dump
                auto status = dladdr1(as_vptr(ptr), &info, as_ptr(&lm), RTLD_DL_LINKMAP);
                if (status && (info.dli_fname != nullptr) && info.dli_fname[0] != '\0')
                {
                    char buf[100];

                    info.dli_fbase = as_vptr(lm->l_addr);
                    info.dli_fbase = as_vptr(lm->l_addr);
                    if (info.dli_sname == nullptr)
                    {
                        info.dli_saddr = info.dli_fbase;
                    }

                    if (info.dli_sname == nullptr && info.dli_saddr == 0)
                    {
                        snprintf(buf, sizeof(buf), "%p", (void*)ptr);
                    }
                    else
                    {
                        char sign = 0;
                        std::ptrdiff_t offset = ptr - (std::ptrdiff_t)info.dli_saddr;
                        sign = offset >= 0 ? '+' : '-';
                        offset = std::abs(offset);
                        snprintf(buf, sizeof(buf), "%c%#tx", sign, offset);
                    }
                    // turn +0x[offset] to symbols using libbfd
                    pc = bfd_scan_vma(decay_arr(buf), nullptr, 16);
                    found = false;

                    bfd_map_over_sections(
                        abfd,
                        [](bfd* abfd, asection* section, void* args) {
                            bfd_vma vma = 0;
                            bfd_size_type size = 0;
                            libbfd_wrapper* that = as_ptr(args);

                            if ((bfd_section_flags(section) & SEC_ALLOC) == 0)
                            {
                                return;
                            }

                            vma = bfd_section_vma(section);
                            if (that->pc < vma)
                            {
                                return;
                            }

                            size = bfd_section_size(section);
                            if (that->pc >= vma + size)
                            {
                                return;
                            }

                            that->found = bfd_find_nearest_line_discriminator(
                                abfd, section, that->syms, that->pc - vma, &that->filename, &that->functionname, &that->line, &that->discriminator
                            );
                        },
                        (void*)this
                    );

                    if (!found)
                    {
                        return {ptr, 0, "UNK", "UNK"};
                    }

                    filename = filename != nullptr ? filename : "UNK";
                    functionname = functionname != nullptr ? functionname : "UNK";

                    entry ent(ptr, line, filename, functionname);
                    if (ent.file.empty())
                    {
                        ent.file = "UNK";
                    }
                    if (ent.function.empty())
                    {
                        ent.function = "UNK";
                    }
                    else
                    {
                        demangle(ent.function);
                    }
                    return ent;
                }

                return {ptr, 0, "UNK", "UNK"};
            }

            ~libbfd_wrapper() { bfd_close(abfd); }
        };
    } // namespace detail

    INLINE auto get_symbols(const pointer_stacktrace& trace) -> symbol_stacktrace
    {
        symbol_stacktrace ret;
        static detail::libbfd_wrapper state;
        ret.reserve(trace.size());
        for (auto entry : trace)
        {
            ret.push_back(state.get_info(entry));
        }

        return ret;
    }
} // namespace stacktrace
