#include "../../stacktrace_fwd.h"
#include <bfd.h>
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

    inline symbol_stacktrace get_traced(const pointer_stacktrace& trace)
    {
        symbol_stacktrace ret;
        static detail::libbfd_wrapper state;
        ret.reserve(trace.size());
        for (auto i : trace)
            ret.push_back(state.get_info(i));

        return ret;
    }
} // namespace stacktrace