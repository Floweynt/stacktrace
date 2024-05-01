#include "../common.h"
#include <cstdlib>
#include <cxxabi.h>
#include <string>

namespace stacktrace
{
    namespace detail
    {
        INLINE void demangle(std::string& str)
        {
            int status = 0;
            char* ptr = abi::__cxa_demangle(str.c_str(), 0, 0, &status);
            if (ptr != nullptr)
            {
                str.assign(ptr);
                free(ptr);
            }
        }
    } // namespace detail
} // namespace stacktrace
