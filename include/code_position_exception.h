//
// Created by flowey on 12/27/21.
//

#ifndef __CODE_POSITION_EXCEPTION_H__
#define __CODE_POSITION_EXCEPTION_H__
#include <stacktrace.h>

namespace stacktrace
{
    class code_position_exception : std::runtime_error
    {

        const char* file;
        const char* func;
        int line;

    public:
        inline explicit code_position_exception(const char* what, const char* file = __builtin_FILE(),
                                                const char* func = __builtin_FUNCTION(), int line = __builtin_LINE())
            : runtime_error(what), file(file), func(func), line(line)
        {
        }

        friend std::ostream& operator<<(std::ostream&, const code_position_exception&);
        inline const char* get_file() const
        {
            return file;
        }
        inline const char* get_func() const
        {
            return func;
        }
        inline int get_line() const
        {
            return line;
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const code_position_exception& e)
    {
        int i = os.iword(detail::geti());
        switch (i)
        {
        case 0:
            os << e.what();
            break;
        case 2:
        case 1:
            os << "what: " << e.what() << '\n';
            os << "at: " << e.file << ":" << e.line << " " << e.func;
        }

        return os;
    }
} // namespace stacktrace

#endif //__CODE_POSITION_EXCEPTION_H__
