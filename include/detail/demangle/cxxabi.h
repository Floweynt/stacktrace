#include <string>
#include <cxxabi.h>

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
    }
}