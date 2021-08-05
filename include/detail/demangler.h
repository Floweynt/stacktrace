#ifndef __DEMANGLER_H__
#define __DEMANGLER_H__
#include <string>
#if defined(USE_NOOP_DEMANGLE)
// nothing
#elif defined(USE_ABI_CXX_DEMANGLE)
#include<cxxabi.h>
#endif

namespace stacktrace
{
    namespace detail 
    {
#if defined(USE_NOOP_DEMANGLE)
#pragma message("demangle: NOOP demangle!")
        inline void demangle(std::string& str)
        {
            // NOOP
        }
#elif defined(USE_ABI_CXX_DEMANGLE)
#pragma message("demangle: abi::__cxa_demangle!")
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
#endif