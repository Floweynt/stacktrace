#define UNW_LOCAL_ONLY
#include "../../stacktrace_fwd.h"
#include <libunwind.h>

namespace stacktrace
{
	inline pointer_stacktrace stacktrace(size_t capture)
	{
		pointer_stacktrace ret;
		unw_cursor_t cursor; unw_context_t uc;
		unw_word_t ip;

		unw_getcontext(&uc);
		unw_init_local(&cursor, &uc);
		while (unw_step(&cursor) > 0) 
		{
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			ret.push_back(ip);
		}

		return ret;
	}
}