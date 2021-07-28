# stacktrace
C++ stacktraces, without needing to use boost.

## Features
Generating stacktraces with:
```cpp
#include "stacktrace.h"

int main()
{
  stacktrace::pointer_stacktrace st = stacktrace::stacktrace(10); // captures max 10 stack frames
                                                                  // this generates a std::vector<uintptr_t>
                                                                  // use stacktrace::get_traced to get the version with symbol
  stacktrace::symbol_stacktrace sym = stacktrace::get_traced(st);
  
  // print
  for(auto entry : sym)
    std::cout << entry.file << ":" << entry.line << " at " << entry.func << '\n';
}
```
Use/extend the `stack_aware_exception` class
```cpp
#include "stacktrace.h"

void some_buggy_function()
{
  // use the throw_dbg macro to make throwing easier
  throw_dbg(stacktrace::stack_aware_exception, "something is wrong!");
}

int main()
{
  try 
  {
    some_buggy_function();
  }
  catch(stack_aware_exception& e)
  {
    // use stream manipulators to set how to print exception
    // stacktrace::shortexcept: prints e.what()
    // stacktrace::longexcept: prints e.what(), and where it was thrown
    // stacktrace::stacktrace: prints e.what(), where it was thrown, and a stacktrace
    // some stacktrace elements from the constructor + stacktrace-generating functions will be printed
    std::cout << stacktrace::stacktrace << e;
  }
```
