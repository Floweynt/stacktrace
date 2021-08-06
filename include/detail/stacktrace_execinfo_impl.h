#ifndef __STACKTRACE_WIN_H__
#define __STACKTRACE_WIN_H__
#include "../stacktrace_fwd.h"
#include <execinfo.h>
#include <vector>
#include <algorithm>
namespace stacktrace
{
	inline pointer_stacktrace stacktrace(size_t capture)
	{
        capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
        pointer_stacktrace stacktrace(capture);
        size_t size = backtrace((void**)stacktrace.data(), capture);
        stacktrace.resize(size);
        return stacktrace;
	}
}

#endif
