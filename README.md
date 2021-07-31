# stacktrace [![build](https://github.com/FloweyTheFlower420/stacktrace/actions/workflows/cmake.yml/badge.svg)](https://github.com/FloweyTheFlower420/stacktrace/actions/workflows/cmake.yml) 
Header-only cross-platform C++ stacktraces, without needing to use boost.

## Features
Simple usage (one line!):
```cpp
#include "stacktrace.h"

int main()
{
  stacktrace::dump_stacktrace(10, std::cout); // dumps a max of 10 stack frames, then prints it to cout
}
```
This will print out (built in debug mode for symbols):
```
AT: [0000562c78b24530] /home/runner/work/stacktrace/stacktrace/examples/../include/detail/stacktrace_execinfo_impl.h:11 (stacktrace::stacktrace(unsigned long))
AT: [0000562c78b25947] /home/runner/work/stacktrace/stacktrace/examples/../include/stacktrace.h:61 (stacktrace::dump_stacktrace(unsigned long, std::ostream&))
AT: [0000562c78b23d12] /home/runner/work/stacktrace/stacktrace/examples/main.cpp:108 (main)
AT: [00007f1831dda0b3] UNK:0 (UNK)
AT: [0000562c78b2390e] UNK:0 (_start)
```
Advanced usages [here!](https://github.com/FloweyTheFlower420/stacktrace/wiki/Advanced-Usage)
## Supported Versions
Supported compilers:
- MSCV
- g++
- clang

Supported operating systems:
- windows
- linux
- macos (not tested)
## Building
Check out the [wiki page](https://github.com/FloweyTheFlower420/stacktrace/wiki/Building)
