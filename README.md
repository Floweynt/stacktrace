# stacktrace
C++ stacktraces, without needing to use boost.

## Features
Simple usage (one line!):
```cpp
#include "stacktrace.h"

int main()
{
  stacktrace::dump_stacktrace(10, std::cout); // dumps a max of 10 stack frames, then prints it to cout
}
```
Advanced usages [here!](https://github.com/FloweyTheFlower420/stacktrace/wiki/Advanced-Usage)
## Supported Versions
Supported compilers:
- MSCV
- g++  

Supported operating systems:
- windows
- linux
- macos (not tested)
## Building
Check out the [wiki page](https://github.com/FloweyTheFlower420/stacktrace/wiki/Building)
