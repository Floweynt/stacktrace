#include "../../stacktrace_fwd.h"
#include <iterator>
#include <libunwind.h>
namespace stacktrace
{
	class stack_iterator
	{
	public:
		using value_type = uintptr_t;
		using pointer = uintptr_t*;
	public:
		bool is_end;
		unw_cursor_t cursor;
		unw_context_t uc;
	public:
		inline stack_iterator()
		{
			unw_getcontext(&uc);
			unw_init_local(&cursor, &uc);
		}

		stack_iterator operator++(int)
		{
			stack_iterator tmp = *this;
			++* this;
			return tmp;
		}

		stack_iterator operator++()
		{
			if(!is_end)
				is_end = unw_step(&cursor) <= 0;
		}

		uintptr_t operator*()
		{
			unw_word_t ip;
			unw_get_reg(&cursor, UNW_REG_IP, &ip);
			return ip;
		}
	};

	inline stack_iterator begin()
	{

	}

	inline stack_iterator end()
	{

	}
}