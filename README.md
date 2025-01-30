# Sammy Stack Machine

This is a work-in-progress JIT compiler and stack machine that supports fast and efficient stack operations to develop turing-complete programs.

**Note**: Look below the (very professionally written) documentation for code examples and more.

## >>> Must-Read Documentation!! <<<
This is an introduction to my girl, Sammy. Sammy is a virtual machine. What she does is she reads instructions one by one, and runs what's said in them in order.

For Sammy to work properly, she needs two stacks:
1. A data stack
2. A call stack

This way, Sammy can jump back and forth through input machine code with speed and efficiency. Sammy's stacks are set to a fixed size, so infinite recursion is just not possible.

Sammy has no registers and no addressable memory (yet), just a good old stack. Sammy's data stack is composed of 128,000 integers, each with a size of 64 bits (8 bytes). Sammy also has a pointer head which she can move up and down on this stack, but she mostly keeps it at the very top of the stack.

Sammy is accompanied by her girlfriend Juniper. Juniper isn't perfect, but she tries her best. Juniper is an assembler/transpiler/compiler which takes in human-readable text (kinda) and crafts it into Sammy's bytecode with love and care. Juniper supports most of the features that Sammy has, but she's still got a long way to go.

Here is what Sammy can do:
1. Halt the program (stops running)
2. Remove the top value from the stack (eg. `1 2` -> `1`)
3. Duplicate the top value on the stack (eg. `1` -> `1 1`)
4. Duplicate the top two values on the stack (eg. `1 2` -> `1 2 1 2`)
5. Push a number onto the stack (eg. `1` -> `1 3`)
6. Call an address in the code (this means Sammy will remember where the call came from and then jumps right back to it after the responder is done!)
7. Jump to an address in the code (this means Sammy won't have a care in the world for where the call came from, useful for loops.)
8. Return from a call (Sammy tries her best to find out where the responder was called from, and goes right back to there!)
9. Swap two values (eg. `1 2` -> `2 1`)
10. Add two values (eg. `1 2` -> `3`)
11. Subtract two values (eg. `2 1` -> `1`)
12. Multiply two values (eg. `2 3` -> `6`)
13. Divide two values (eg. `6 3` -> `2`)
14. Get the modulo of a value (eg. `10 3` -> `1`)
15. Check equality/less-than/greater-than (eg. `1 1` -> `1` and `2 4` -> `0`)
16. Invert (not operator) (eg. `1` -> `0` and `2 -> 0` and `0 -> 1`)
17. Call a function only if the top of the stack is true (just like the call operator but conditional)
18. Jump to a place only if the top of the stack is true (this is the beating heart of loops!)
19. Add and call procedures (honestly, this is just a redundant feature. it could be likened to an appendix. i'll find a use for it someday!)
20. Call the auxillary function (this is like Sammy's smartphone! she can use it to do so so many things!)

Here's what Juniper can do:
1. Skip whitespace (eg. `       ` == ``)
2. Push a character onto stack (code: `'c` or `'3` or `'*` and so on)
3. Push a string onto stack (reversed. code: `"hello"` or `"HAII!!!!!IOJO@I#!J"`)
4. Push a number onto stack (code: `23` or `00023987` or `110011020`)
5. Create a label (an underlying mechanism that lets you jump to code parts without having to deal with the addresses yourself!) (code: `main:` or `HelloWorld:` or `label_name:`)
6. Use Sammy functionality (code: `dup` or `ddup` or `putchar` or `log` or `trace` or `ret` or `swap` or `pop`)
7. Call label defined in the code (code: `@main` or `@HelloWorld` or `@this_is_a_really_long_labelName`)
8. Jump to a label defined in the code (it's like call but the responder doesn't know where to go back!) (code: `&main` or `&loop_body`)
9. Call a label only if the top of stack is true (code: `?destroy_the_world` or `?establish_peace_across_the_non_steven_universe`)
10. Jump to a label only if the top of stack is true (code: `^loop_body` or `^LetMeContinue`)
11. Write a comment! (code: `# :3` or `# wow!! so cool`)
12. Call the auxillary function! (code: `$`) (don't ever do this. let Juniper take care of it for you.)
13. Invert the top value on the stack (code: `!`)
14. Less than (code: `<`), Greater than (code: `>`), Add (code: `+`), Subtract (code: `-`), Multiply (code: `*`), Divide (code: `/`), Modulo (code: `%`)

And, what's more, Juniper is always by Sammy's side, ready to take her hand and guide her through the input that the user gives her to run :3

Thanks for reading, and have a great day!

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