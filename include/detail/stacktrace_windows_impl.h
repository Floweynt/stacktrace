#ifndef __STACKTRACE_WIN_H__
#define __STACKTRACE_WIN_H__
#define NOMINMAX
#include <Windows.h>
#include <DbgHelp.h>
#include <algorithm>
#include <concepts>
#include "../stacktrace_fwd.h"
#include "demangler.h"

#pragma comment(lib, "DbgHelp.lib")

namespace stacktrace
{
	namespace detail
	{
		class winapi_wrapper
		{
			static constexpr int MAX_LEN = 1023;
			// hummm yes c++
			alignas (SYMBOL_INFO) char buf[sizeof(SYMBOL_INFO) + MAX_LEN + 1]{ };
			SYMBOL_INFO* symbol = new (buf) SYMBOL_INFO;
			IMAGEHLP_LINE line;
			HANDLE process;
			bool is_valid;

		public:
			inline winapi_wrapper()
			{
				process = GetCurrentProcess();

				if (!SymInitialize(process, nullptr, true))
				{
					is_valid = false;
					return;
				}

				symbol->MaxNameLen = MAX_LEN;
				symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

				is_valid = true;
			}

			inline entry get_info(uintptr_t ptr)
			{
				DWORD tmp;
				entry ret{ ptr, 0, "UNK", "UNK" };

				if (!is_valid)
					return ret;

				if (SymFromAddr(process, (DWORD64)(ptr), 0, symbol))
				{
					ret.address = symbol->Address;
					ret.function = symbol->Name;
					demangle(ret.function);
				}

				if (SymGetLineFromAddr(process, (DWORD64)(ptr), &tmp, &line))
				{
					ret.line = line.LineNumber;
					ret.file = line.FileName;
				}

				return ret;
			}

			inline ~winapi_wrapper()
			{
				SymCleanup(process);
			}
		};
	}

	inline pointer_stacktrace stacktrace(size_t capture)
	{
		capture = std::max((size_t)((USHORT)-1), capture);
		pointer_stacktrace ret(capture);
		size_t size = CaptureStackBackTrace(0, capture, (void**)ret.data(), nullptr);
		ret.resize(size);
		return ret;
	}

	inline symbol_stacktrace get_traced(const pointer_stacktrace& pointer_stacktrace)
	{
		symbol_stacktrace trace;
		detail::winapi_wrapper state;
		for (uintptr_t ptr : pointer_stacktrace)
			trace.emplace_back(state.get_info(ptr));

		return trace;
	}
}

#undef NOMINMAX
#endif
