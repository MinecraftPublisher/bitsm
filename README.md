# Sammy Stack Machine

This is a work-in-progress JIT compiler and stack machine that supports fast and efficient stack operations to develop turing-complete programs.

Code example:
```
@main

print:
    aux dup #print ret

main:
    "Hello world!" @print
    ret
```

This program reads a string from the stack and hands it over to the auxillary function (which in this case is putchar) one character at a time, until it reaches a zero.