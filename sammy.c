#include <stdint.h>
#include <stdio.h>

typedef uint8_t byte;
typedef byte bool;
typedef uint64_t u64;
typedef int64_t  i64;
typedef void    *pointer;

#define unwrap(ptr) (*ptr)
#define error(name, ...)                                                                                                       \
    ({                                                                                                                         \
        printf("Error: " name "\n" __VA_OPT__(, ) __VA_ARGS__);                                                                \
        powered = 0;                                                                                                           \
        0;                                                                                                                     \
    })

#define p_cat(a, b) a##b
#define cat(a, b)   p_cat(a, b)

#define create_stack(name, type, size)                                                                                         \
    const u64 cat(name, _stack_size)    = size;                                                                                \
    type      cat(name, _stack)[]       = { [0] = 0, [size - 1] = 0 };                                                         \
    u64       cat(name, _stack_pointer) = 0;

create_stack(main, u64, 128 * 1000);
create_stack(call, u64, 16 * 1000);

#define push(stack, value)                                                                                                     \
    (stack[ cat(stack, _pointer)++ >= cat(stack, _size) ? error("Stack overflow") : (cat(stack, _pointer) - 1) ] = value)
#define pop(stack) (stack[ --cat(stack, _pointer) < 0 ? error("Stack underflow") : cat(stack, _pointer) ])
#define pop2(stack, l, r)                                                                                                      \
    __auto_type l = pop(stack);                                                                                                \
    __auto_type r = pop(stack)
#define peek(stack) (stack[ cat(stack, _pointer) - 1 ])
#define item(value, code)                                                                                                      \
    case value: {                                                                                                              \
        code;                                                                                                                  \
    }; break
#define none(code)                                                                                                             \
    default: {                                                                                                                 \
        code;                                                                                                                  \
    } break

byte powered  = 1;
byte exitcode = 0;

#define MAX_SUBROUTINES 2048
u64 subroutines[ MAX_SUBROUTINES ] = { 0 };

void (*aux_function)() = NULL;

// Flags:
// n - add to stack, m - remove from stack
// d - read from instruction data, s - modify subroutines

enum opcode : byte {
    d_halt,
    m_pop,
    n_dup,
    dn_const,
    m_call,
    ret,
    mmnn_swap,
    mmn_add,
    mmn_sub,
    mmn_mul,
    mmn_div,
    mmn_eq,
    m_callz,
    sdm_add_proc,
    d_call_proc,
    d_call_aux
};

struct op {
    enum opcode op;
    u64         data;
};

void cycle(u64 *cur, struct op inst) {
    switch (inst.op) {
        item(d_halt, exitcode = inst.data; powered = 0);
        item(m_pop, (void) pop(main_stack));
        item(n_dup, {
            u64 val = peek(main_stack);
            push(main_stack, val);
        });
        item(dn_const, push(main_stack, inst.data));
        item(m_call, {
            u64 addr = pop(main_stack);
            push(call_stack, *cur);
            *cur += addr;
        });
        item(ret, *cur = pop(call_stack));
        item(mmnn_swap, {
            pop2(main_stack, a, b);
            push(main_stack, a);
            push(main_stack, b);
        });
        item(mmn_add, {
            pop2(main_stack, a, b);
            push(main_stack, a + b);
        });
        item(mmn_sub, {
            pop2(main_stack, a, b);
            push(main_stack, b - a);
        });
        item(mmn_mul, {
            pop2(main_stack, a, b);
            push(main_stack, a * b);
        });
        item(mmn_div, {
            pop2(main_stack, a, b);
            if (a == 0) error("Divide by zero");
            push(main_stack, b / a);
        });
        item(mmn_eq, {
            pop2(main_stack, a, b);
            push(main_stack, a == b);
        });
        item(m_callz, {
            pop2(main_stack, a, addr);
            if (a == 0) {
                push(call_stack, *cur);
                *cur += addr;
            }
        });
        item(sdm_add_proc, {
            u64 addr = pop(main_stack);
            if (inst.data >= MAX_SUBROUTINES) error("Invalid procedure ID");
            subroutines[ inst.data ] = addr;
        });
        item(d_call_proc, {
            if (inst.data >= MAX_SUBROUTINES || subroutines[ inst.data ] == 0) error("Invalid or undefined procedure ID");
            push(call_stack, *cur);
            *cur = subroutines[ inst.data ];
        });
        item(d_call_aux, {
            if (aux_function) aux_function();
            else { error("No auxiliary function set"); }
        });
    }
}

void run(u64 size, struct op *code) {
    powered  = 1;
    exitcode = 0;
    for (u64 i = 0; (i < size) && powered; i++) cycle(&i, code[ i ]);
}

void print_aux() { printf("aux: %lu\n", pop(main_stack)); }

int main() {
    aux_function = print_aux;

    struct op code[] = { { dn_const, 42 }, { d_call_aux, 0 }, { dn_const, 20 }, { d_halt, 0 } };

    run(sizeof(code) / sizeof(struct op), code);

    printf("exit: %i size: %li stack: %li\n", exitcode, main_stack_pointer, peek(main_stack));
}