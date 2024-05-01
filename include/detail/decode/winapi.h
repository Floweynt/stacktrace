#include "../../stacktrace_fwd.h"
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
