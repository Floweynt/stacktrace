#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace stacktrace
{
    struct entry
    {
        entry() : line(0), address(0), file("UNK"), function("UNK") {}

        entry(uintptr_t address, size_t line, std::string file, std::string function)
            : address(address), line(line), file(std::move(file)), function(std::move(function))
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
