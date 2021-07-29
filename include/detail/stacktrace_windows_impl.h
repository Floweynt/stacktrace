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

			inline SYMBOL_INFO* get_info_of(uintptr_t ptr)
			{
				if(!is_valid)
					return nullptr;

				if (!SymFromAddr(process, (DWORD64)(ptr), 0, symbol))
					return nullptr;

				return symbol;
			}

			inline IMAGEHLP_LINE* get_line_of(uintptr_t ptr)
			{
				if(!is_valid)
					return nullptr;

				DWORD tmp;
				if (!SymGetLineFromAddr(process, (DWORD64)(ptr), &tmp, &line))
					return nullptr;

				return &line;
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
		{
			SYMBOL_INFO* info = state.get_info_of(ptr);
			IMAGEHLP_LINE* line = state.get_line_of(ptr);

			std::string str(info ? (const char*)info->Name : "UNK");
			if(info)
				detail::demangle(str);

			trace.emplace_back(
				info ? (uintptr_t)info->Address : ptr,
				line ? (size_t)line->LineNumber : 0,
				line ? std::string((const char*)line->FileName) : "UNK",
				info ? str : "UNK"
			);
		}

		return trace;
	}
}

#undef NOMINMAX
#endif
