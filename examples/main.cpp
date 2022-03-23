#include <cstdio>
#include <iostream>
#include <iomanip>
#include <stacktrace.h>
#include <stacktrace_exception.h>
#include <code_position_exception.h>

void recursion(int i);
void func_0(int i);
void func_1(int i);
void method_0();
void method_1();
void method_2();
void method_3();
void some_handler();

void recursion(int i)
{
    if (i == 5)
        func_0(0);
    else
        recursion(i + 1);
}

void func_0(int i)
{
    if (i == 2)
        some_handler();
    else
        func_1(i + 1);
}

void func_1(int i)
{
    func_0(i);
}

void method_0()
{
    method_1();
}

void method_1()
{
    method_2();
}

void method_2()
{
    method_3();
}

void method_3()
{
    recursion(0);
}

void some_handler()
{
    throw stacktrace::stacktrace_exception("test_");
}

void some_buggy_function()
{
    // use the throw_dbg macro to make throwing easier
    throw stacktrace::stacktrace_exception("something is wrong!");
}

int main(int argc, char** argv)
{
    std::cout << "starting example print\n";
    if (argc == 1)
    {
        stacktrace::dump_stacktrace();
        stacktrace::dump_stacktrace();
    }
    else if (argc == 2)
    {
        std::cout << "argc == 2\n";
        char c = argv[1][0];
        stacktrace::stack_printer printer = [](const stacktrace::entry& e, std::ostream& os, size_t)
        {
            os << "AT: " << e.file << ':' << e.line << " (" << e.function << ')';
        };

        switch (c)
        {
        case '0':
            try
            {
                method_0();
            }
            catch (stacktrace::stacktrace_exception& e)
            {
                std::cout << stacktrace::stacktrace << e << std::endl;
            }
            break;
        case '1':
            stacktrace::dump_stacktrace();
            std::cout << std::endl;
            break;
        case '2':
            try
            {
                some_buggy_function();
            }
            catch (stacktrace::stacktrace_exception& e)
            {
                std::cout << stacktrace::stacktrace << e << std::endl;
            }
            break;
        case '3':
            stacktrace::dump_stacktrace();
            std::cout << std::endl;
            break;
        case '4':
            std::cout << "here!\n";
            std::cout << stacktrace::longexcept << stacktrace::code_position_exception("test") << std::flush;
            break;
        default:
            break;
        }
    }

    return 0;
}
