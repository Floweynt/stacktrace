#!/bin/sh 

mkdir -p temp 

# this script generates prebuilt headers
cat include/stacktrace_fwd.h \
    include/detail/demangle/$3.h \
    include/detail/stack/$1.h \
    include/detail/decode/$2.h \
    include/stacktrace.h \
    include/code_position_exception.h \
    include/stacktrace_exception.h > temp/out_1.h 

sed -i -e 's/#include ".\+"//g' temp/out_1.h
sed -i -e 's/#include \w\+//g' temp/out_1.h
sed -i -e 's/#ifndef .\+//g' temp/out_1.h
sed -i -e 's/#endif//g' temp/out_1.h
sed -i -e 's/#define _.\+//g' temp/out_1.h

# build actual output 
echo -e '#ifndef __STACKTRACE_SINGLE_HEADER__\n' > temp/out_2.h 
echo -e '#define __STACKTRACE_SINGLE_HEADER__\n' >> temp/out_2.h 
echo -e '#define MAX_CAPTURE_FRAMES 100\n' >> temp/out_2.h 
cat temp/out_1.h >> temp/out_2.h 
echo -e '#endif' >> temp/out_2.h

clang-format -i temp/out_2.h
dos2unix temp/out_2.h 

cp temp/out_2.h prebuilt/$1_$2_$3.h
rm -rf temp 
