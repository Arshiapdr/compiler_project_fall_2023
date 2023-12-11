# compiler_phase1 (Fall 2023)

How to run?
mkdir build
cd build
cmake ..
make
cd src
./gsm "<the input you want to be compiled>" > gsm.ll
llc --filetype=obj -o=gsm.o gsm.ll
clang -o gsmbin gsm.o ../../rtGSM.c

Sample inputs
int a;
int a = 3 * 9;
int a = 4;
int b = 4 * 9;
int c;
c = a * b;
