Compiler Final Project - Fall 2023
[Arshia Paridari - Pouria Yazdani]

## Running Instructions
```
mkdir build
cd build
cmake ..
make
cd src
./gsm "<the input you want to be compiled>" > gsm.ll
llc --filetype=obj -o=gsm.o gsm.ll
clang -o gsmbin gsm.o ../../rtGSM.c
```
## Sample Input Codes With Their Outputs
input code 1
```
int a;
int b;
int result;
result = a + b;
a = 2;"
```
output IR with implementing dead code elimination
```
; ModuleID = 'calc.expr'
source_filename = "calc.expr"

define i32 @main(i32 %0, i8** %1) {
entry:
  %2 = alloca i32, align 4
  store i32 0, i32* %2, align 4
  %3 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  %4 = alloca i32, align 4
  store i32 0, i32* %4, align 4
  %5 = load i32, i32* %2, align 4
  %6 = load i32, i32* %3, align 4
  %7 = add nsw i32 %5, %6
  %8 = load i32, i32* %4, align 4
  store i32 %7, i32* %4, align 4
  call void @gsm_write(i32 %7)
  %9 = load i32, i32* %2, align 4
  store i32 2, i32* %2, align 4
  call void @gsm_write.1(i32 2)
  ret i32 0
}

declare void @gsm_write(i32)

declare void @gsm_write.1(i32)
```
