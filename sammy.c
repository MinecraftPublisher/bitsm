#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef uint8_t  byte;
typedef uint64_t u64;
typedef uint32_t u32;
typedef int64_t  i64;
typedef int32_t  i32;
typedef char    *string;
#define null 0

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
    i64       cat(name, _stack_pointer) = 0;

create_stack(main, u64, 128 * 1000);
create_stack(call, u64, 16 * 1000);

#define push(stack, value)                                                                                                     \
    (stack[ cat(stack, _pointer)++ >= cat(stack, _size) ? error("Stack overflow (" #stack ")") : (cat(stack, _pointer) - 1) ]  \
     = value)
#define pop(stack) (stack[ --cat(stack, _pointer) < 0 ? error("Stack underflow (" #stack ")") : cat(stack, _pointer) ])
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

void (*aux_function)(i32) = NULL;

// Flags:
// n - add to stack, m - remove from stack
// d - read from instruction data, s - modify subroutines

enum opcode : byte {
    d_halt,
    m_pop,
    n_dup,
    dn_const,
    m_call,
    d_call,
    ret,
    mmnn_swap,
    mmn_add,
    mmn_sub,
    mmn_mul,
    mmn_div,
    mmn_eq,
    m_callnz,
    d_callnz,
    sdm_add_proc,
    d_call_proc,
    d_call_aux
};
const string names[]
    = { "d_halt",  "m_pop",   "n_dup",   "dn_const", "m_call",  "d_call",  "ret",          "mmnn_swap",   "mmn_add",
        "mmn_sub", "mmn_mul", "mmn_div", "mmn_eq",   "m_callz", "d_callz", "sdm_add_proc", "d_call_proc", "d_call_aux" };

struct op {
    enum opcode op;
    i32         data;
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
            *cur += addr - 1;
        });
        item(d_call, {
            push(call_stack, *cur);
            *cur += inst.data - 1;
        });
        item(ret, {
            if (call_stack_pointer == 1) powered = 0;
            *cur = pop(call_stack);
        });
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
        item(m_callnz, {
            pop2(main_stack, a, addr);
            if (a == 0) {
                push(call_stack, *cur);
                *cur += addr - 1;
            }
        });
        item(d_callnz, {
            u64 a = pop(main_stack);
            if (a != 0) {
                push(call_stack, *cur);
                *cur += inst.data - 1;
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
            if (aux_function) aux_function(inst.data);
            else { error("No auxiliary function set"); }
        });
    }
}

void run(u64 size, struct op *code) {
    exitcode = 0;
    for (u64 i = 0; (i < size) && powered; i++) cycle(&i, code[ i ]);
}

void print_aux(i32 data) { putchar(data == 0 ? pop(main_stack) : data); }

struct sammycode {
    u64       size;
    struct op code[ 64000 ];
};

struct assembler_data {
    u32 labels_count;
    struct label {
        char name[ 256 ];
        u64  position;
    } labels[ 1024 ];

    u32 label_delegation_count;
    struct label_delegated {
        u64  target_position;
        char name[ 256 ];
    } label_delegations[ 4096 ];
};

#define eq(a, b) (strcmp(a, b) == 0)

#define get_keyword()                                                                                                          \
    ({                                                                                                                         \
        char name[ 256 ];                                                                                                      \
        byte size      = 0;                                                                                                    \
        name[ size++ ] = c;                                                                                                    \
        while (((((c = next()) >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '-') || (c == '_'))               \
               && (size <= 255)) {                                                                                             \
            name[ size++ ] = c;                                                                                                \
        }                                                                                                                      \
        text--;                                                                                                                \
        name[ size ] = 0;                                                                                                      \
        name;                                                                                                                  \
    })

#define add_code(operator, data)                                                                                               \
    result.code[ result.size++ ] = (struct op) { operator, data }
#define next() (*(text++))
struct sammycode assemble(char *text, struct assembler_data storage) {
    struct sammycode result = { .size = 0, .code = { [0] = 0, [64000 - 1] = 0 } };

    char c;
    while ((c = next()) != null && powered) {
        switch (c) {
            item(' ', {});
            item('\n', {});
            item('\t', {});

            item('\'', add_code(dn_const, next()));

            item('"', {
                char str[ 4096 ];
                u32  size = 0;

                while ((c = next()) != null && size < 4096) {
                    if (c == '"') break;
                    str[ size++ ] = c;
                }

                add_code(dn_const, 0);
                for (i32 i = size - 1; i >= 0; i--) add_code(dn_const, str[ i ]);
            });

            item('0' ... '9', {
                u64 number = c - '0';

                while ((c = next()) >= '0' && (c <= '9')) number = (number * 10) + (c - '0');

                text--;
                add_code(dn_const, number);
            });

            item('a' ... 'z', {
                item('A' ... 'Z', {
                    item('-', {
                        item('_', {
                            string name = get_keyword();

                            // process keyword

                            char c = next();
                            if (c == ':') { // create label
                                storage.labels[ storage.labels_count ].position = result.size;
                                memcpy(storage.labels[ storage.labels_count++ ].name, name, strlen(name));
                            }

                            else if (eq(name, "dup")) {
                                add_code(n_dup, 0);
                            }

                            else if (eq(name, "aux")) {
                                add_code(d_call_aux, 0);
                            }

                            else if (eq(name, "ret")) {
                                add_code(ret, 0);
                            }

                            else {
                                error("Unknown function '%s'", name);
                            }
                        });
                    });
                });
            });

            item('@', {
                c               = next();
                string name     = get_keyword();
                u64    position = -1;

                // look for label
                for (u64 i = 0; i < storage.labels_count; i++) {
                    if (eq(storage.labels[ i ].name, name)) {
                        position = storage.labels[ i ].position;
                        goto FOUND;
                    }
                }

                // Delegate label resolution to after-assembly
                storage.label_delegations[ storage.label_delegation_count ].target_position = result.size;
                add_code(d_call, -1);
                memcpy(storage.label_delegations[ storage.label_delegation_count++ ].name, name, strlen(name));
                break;

            FOUND:;
                // calculate offset
                i64 offset = position - result.size;

                add_code(d_call, offset);
            });

            item('#', {
                c               = next();
                string name     = get_keyword();
                u64    position = -1;

                // look for label
                for (u64 i = 0; i < storage.labels_count; i++) {
                    if (eq(storage.labels[ i ].name, name)) {
                        position = storage.labels[ i ].position;
                        goto FOUND2;
                    }
                }

                // Delegate label resolution to after-assembly
                storage.label_delegations[ storage.label_delegation_count ].target_position = result.size;
                add_code(d_call, -1);
                memcpy(storage.label_delegations[ storage.label_delegation_count++ ].name, name, strlen(name));
                break;

            FOUND2:;
                // calculate offset
                i64 offset = position - result.size;

                add_code(d_callnz, offset);
            });

            item('$', { add_code(d_call_aux, 0); });
        }
    }

    if (powered) {
        // resolve delegated labels
        for (u64 i = 0; i < storage.label_delegation_count; i++) {
            struct label_delegated label = storage.label_delegations[ i ];
            for (u64 j = 0; j < storage.labels_count; j++) {
                if (eq(storage.labels[ j ].name, label.name)) {
                    result.code[ label.target_position ].data = storage.labels[ j ].position;
                    goto TOP_CONTINUE;
                }
            }

            error("Unknown label '%s'", storage.label_delegations[ i ].name);
        TOP_CONTINUE:;
        }
    }

    return result;
}

#include <stdio.h>
#include <stdlib.h>

string read_file(string file_name) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        powered = 0;
        perror("Could not open file");
        return null;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size <= 0) {
        error("Error determining file size, or empty file\n");
        fclose(file);
        return null;
    }

    char *buffer = (char *) malloc(size + 1);
    if (buffer == NULL) {
        exit(1);
        return null;
    }

    fread(buffer, 1, size, file);
    buffer[ size ] = '\0';

    fclose(file);
    return buffer;
}

int main() {
    aux_function = print_aux;

    struct sammycode output = assemble(read_file("input.sam"), (struct assembler_data) {});
    // for (u64 i = 0; i < output.size; i++) { printf("{ %s, %i }\n", names[ output.code[ i ].op ], output.code[ i ].data); }

    run(output.size, output.code);
}