#include <csignal>
#include <cstdint>
#include <iostream>
#include <stacktrace.h>

auto main() -> int
{
    std::cout << "Waiting for signal...\n";
    (void)signal(
        SIGINT,
        +[](int) {
            stacktrace::signal_safe_stacktrace([](uintptr_t ptr) {
                // bad practice, IO should not be performed
                std::cout << std::hex << "0x" << ptr << '\n';
            });
        }
    );

    while (true) {}
}
