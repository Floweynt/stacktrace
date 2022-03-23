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