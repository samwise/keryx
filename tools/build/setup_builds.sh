#/bin/bash


DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
mkdir -p $DIR/../../build/
cd $DIR/../../build/

rm -rf clang_debug 
mkdir clang_debug
cd clang_debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=`realpath ../../tools/build/rtags.sh` ../..
cd ..

rm -rf gcc_debug
mkdir gcc_debug
cd gcc_debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=/usr/lib/ccache/g++ ../..
cd ..

rm -rf gcc_release
mkdir gcc_release
cd gcc_release
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_CXX_COMPILER=g++ ../..
cd ..


