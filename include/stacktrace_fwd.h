#ifndef __STACKTRACE_FWD_H__
#define __STACKTRACE_FWD_H__
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

    class stack_iterator;
} // namespace stacktrace

#endif
