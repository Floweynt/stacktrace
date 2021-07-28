#ifndef __STACKTRACE_WIN_H__
#define __STACKTRACE_WIN_H__
#include "../stacktrace_fwd.h"
#include <execinfo.h>
#include <vector>
namespace stacktrace
{
	inline pointer_stacktrace stacktrace(size_t capture)
	{
        pointer_stacktrace stacktrace(capture);
        size_t size = backtrace((void**)stacktrace.data(), capture);
        stacktrace.resize(size);
        return stacktrace;
	}
}

#endif
