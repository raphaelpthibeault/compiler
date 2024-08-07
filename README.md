This project is a compiler for a custom language defined in a compiler class. The language supports multiple inheritance and has custom Rust-style syntax.

The compiler uses a table-driven recursive descent parser.
The chosen intermediate representation for the syntax analysis phase was an abstract syntax tree (AST).
The semantic analysis phase was split into two stages: generating symbol tables for the AST vertices and performing the semantic checkings.
The code generation phase outputs the mooncode for the Moon Virtual Machine.

The visitor design pattern was used to traverse the AST and perform the semantic analysis and the code generation.

An example program for bubblesort:
```c
/* sort the array */
func bubbleSort(arr: integer[], size: integer) -> void
{
    let n: integer;
    let i: integer;
    let j: integer;
    let temp: integer;
    n = size;
    i = 0;
    j = 0;
    temp = 0;
    while (i < n-1) {
        while (j < n-i-1) {
            if (arr[j] > arr[j+1])
                then {
                // swap temp and arr[i]
                temp = arr[j];
                arr[j] = arr[j+1];
                arr[j+1] = temp;
            } else ;
            
            j = j+1;
        };
        
        i = i+1;
    };
}

func printArray(arr: integer[], size: integer) -> void
{
    let n: integer;
    let i: integer;
    n = size;
    i = 0;
    while (i<n) {
        write(arr[i]);
        i = i+1;
    };
}

func main() -> void
{
    let arr: integer[7];
    arr[0] = 64;
    arr[1] = 34;
    arr[2] = 25;
    arr[3] = 12;
    arr[4] = 22;
    arr[5] = 11;
    arr[6] = 90;
    printarray(arr, 7);
    bubbleSort(arr, 7);
    printarray(arr, 7);
}
```
