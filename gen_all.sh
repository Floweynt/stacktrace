#!/bin/bash 

win_stacktrace_opts=("winapi" "libbacktrace")
win_decode_opts=("noop" "winapi" "libbacktrace")

linux_stacktrace_opts=("execinfo" "libbacktrace" "libunwind")
linux_decode_opts=("noop" "libbfd" "libbacktrace")

for a in winapi libbacktrace; do 
    for b in noop winapi libbacktrace; do 
        ./gen_prebuilt.sh $a $b noop 
    done 
done

for a in execinfo libbacktrace libunwind; do 
    for b in noop libbfd libbacktrace; do 
        ./gen_prebuilt.sh $a $b cxxabi 
    done 
done

