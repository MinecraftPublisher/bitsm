# Sammy Stack Machine

This is a work-in-progress JIT compiler and stack machine that supports fast and efficient stack operations to develop turing-complete programs.

### Code example:
```
@main

print: # print a string to console
    putchar dup ^print pop ret

main:
    "Hello world!" @print
```

### Step-by-step breakdown:
```
# skips the code to the main function
@main

# creates a label called "print"
print:
    # removes the top character from the stack and writes it to console
    putchar
    # duplicates the top value of the stack
    dup
    # pops the top value of the stack, and then jumps back to the start of the "print" label if the value is not zero.
    ^print
    # finally, after it reaches zero, we remove the zero from the stack.
    pop
    # and then we return to wherever the print label was called from.
    ret

# the code for the main body of the program
main:
    # writes "Hello world!" to the stack, reversed (because it's a stack.)
    "Hello world!"
    # calls the label named print
    @print
```

This program reads a string from the stack and hands it over to the putchar function one character at a time, until it reaches a zero.