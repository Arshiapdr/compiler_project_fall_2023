## Compiler Final Project - Fall 2023

## Running Instructions
```
mkdir build
cd build
cmake ..
make
cd src
./gsm "<the input you want to be compiled>"
```
## Sample Input Codes With Their Outputs
Input Code 1
```
int a;
int b;
int result;
result = a + b;
a = 2;"
```
Output IR 1 With Implementing Dead Code Elimination
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
<Note that in the above code, there is no dead variable to eliminate>

Input Code 2
```
int result;
int a = 4;
int b = 2;
b += a;
int c = 12;
int d;
d = c;
result *= ((a * b) - 34 * 5 + 12);
```
Output IR 2 With Implementing Dead Code Elimination
```
variable 'c' is dead.
variable 'd' is dead.
; ModuleID = 'calc.expr'
source_filename = "calc.expr"

define i32 @main(i32 %0, i8** %1) {
entry:
  %2 = alloca i32, align 4
  store i32 0, i32* %2, align 4
  %3 = alloca i32, align 4
  store i32 4, i32* %3, align 4
  %4 = alloca i32, align 4
  store i32 2, i32* %4, align 4
  %5 = load i32, i32* %3, align 4
  %6 = load i32, i32* %4, align 4
  %7 = add nsw i32 %6, %5
  store i32 %7, i32* %4, align 4
  call void @gsm_write(i32 %7)
  %8 = load i32, i32* %3, align 4
  %9 = load i32, i32* %4, align 4
  %10 = mul nsw i32 %8, %9
  %11 = sub nsw i32 %10, 170
  %12 = add nsw i32 %11, 12
  %13 = load i32, i32* %2, align 4
  %14 = mul nsw i32 %13, %12
  store i32 %14, i32* %2, align 4
  call void @gsm_write.1(i32 %14)
  ret i32 0
}

declare void @gsm_write(i32)

declare void @gsm_write.1(i32)
```
<Two variables c and d are dead as it is printed in the output, and the corresponding IR is optimized due to them>
