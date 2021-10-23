#include <Windows.h>
#include "../../stacktrace_fwd.h"

namespace stacktrace
{
	inline pointer_stacktrace stacktrace(size_t capture)
	{
		capture = std::min((size_t)((USHORT)-1), capture);
		capture = std::min(capture, (size_t)MAX_CAPTURE_FRAMES);
		pointer_stacktrace ret(capture);
		size_t size = CaptureStackBackTrace(0, capture, (void**)ret.data(), nullptr);
		ret.resize(size);
		return ret;
	}
}