#include <stacktrace.h>
#include <csignal>

int main()
{
    std::cout << "Waiting for signal...\n";
    signal(SIGINT, +[](int) {
        stacktrace::signal_safe_stacktrace([](uintptr_t ptr) {
            // bad practice, IO should not be performed
            std::cout << std::hex << "0x" << ptr << '\n';
        });       
    });
    while(1){}
}
