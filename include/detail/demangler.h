#include <string>
#ifdef _MSC_VER 
// nothing
#elif __GNUC__ 
#include<cxxabi.h>
#endif

namespace stacktrace
{
    namespace detail 
    {
#ifdef _MSC_VER
#pragma message("CC: MSVC, NOOP demangle!")
        inline void demangle(std::string& str)
        {
            // NOOP
        }
#elif __GNUC__
#pragma message("CC: GNU, abi::__cxa_demangle!")
        inline void demangle(std::string& str)
        {
            int status;
            char* ptr  = abi::__cxa_demangle(str.c_str(), 0, 0, &status);
            if(ptr)
            {
                str.assign(ptr);
                free(ptr);
            }
        }
#endif
    }
}
