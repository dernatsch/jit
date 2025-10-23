#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>

#include "bfprograms.h"

typedef int __attribute((preserve_none)) (*jitf)(uint8_t, uint8_t*);

static const uint32_t op_add_code[] = {
    0x11000408, //     	add	w8, w0, #0x1
    0x12001d00, //     	and	w0, w8, #0xff
};

static const uint32_t op_sub_code[] = {
    0x51000408, //     	sub	w8, w0, #0x1
    0x12001d00, //     	and	w0, w8, #0xff
};

static const uint32_t op_left_code[] = {
    0x39000280, //     	strb	w0, [x20]
    0x385ffe80, //     	ldrb	w0, [x20, #-0x1]!
};

static const uint32_t op_right_code[] = {
    0x39000280, //     	strb	w0, [x20]
    0x38401e80, //     	ldrb	w0, [x20, #0x1]!
};

static const uint32_t op_ret_code[] = {
    0xd65f03c0, // ret
};

static const uint32_t op_loop_start_code[] = {
    0x34000000, // cbz w0, <off>
};

// one relocation: offset to loop entry, could also be conditional?
static const uint32_t op_loop_end_code[] = {
    0x14000000, // b <off>
};

// one relocation: offset to putchar_proxy for the bl
static const uint32_t op_out_code[] = {
    0xa9bf7bfd, //     	stp	x29, x30, [sp, #-0x10]!
    0x910003fd, //     	mov	x29, sp
    0xaa0003f3, //     	mov	x19, x0
    0x13001c00, //     	sxtb	w0, w0
    0x94000000, //     	bl	0x40 <_op_out+0x10>
    0xaa1303e0, //     	mov	x0, x19
    0xa8c17bfd, //     	ldp	x29, x30, [sp], #0x10
};

// Is this necessary?
void putchar_proxy(char c) {
    putchar(c);
}

static void dump(void *buf) {
    uint32_t *cur = (uint32_t *)buf;

    while (*cur) {
        printf("%x\n", *cur++);
    }
}

static int emit(void *buf, const uint32_t *code, int len) {
    memcpy(buf, code, len);
    return len;
}

static jitf compile(const char *prg) {
    const int size_guess = 1024*1024;

    void *page = mmap(NULL, size_guess, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (!page) {
        perror("mmap failed");
        exit(1);
    }
    memset(page, 0, size_guess);

    void *cur = page;

    void *loopstack[1000] = {0};
    int loopidx = 0;
    void *loop_start_addr;
    ptrdiff_t jump_len;

    // Generate code

    for (unsigned long i = 0; i < strlen(prg); i++) {
        const char c = prg[i];
        switch (c) {
            case '+':
                cur += emit(cur, op_add_code, 8);
                break;
            case '-':
                cur += emit(cur, op_sub_code, 8);
                break;
            case '<':
                cur += emit(cur, op_left_code, 8);
                break;
            case '>':
                cur += emit(cur, op_right_code, 8);
                break;
            case '[':
                loopstack[loopidx] = cur;
                loopidx += 1;
                cur += emit(cur, op_loop_start_code, 4);
                break;
            case ']':
                // loop end
                // Here, we need to patch the start and end.
                loopidx -= 1;
                loop_start_addr = loopstack[loopidx];
                jump_len = cur - loop_start_addr;

                // patch start
                *(uint32_t *)loop_start_addr |= (jump_len+4) << 3;

                // patch end
                cur += emit(cur, op_loop_end_code, 4);
                *(uint32_t *)(cur - 4) |= (((-jump_len) >> 2) & 0x3FFFFFF);

                break;
            case '.':
                cur += emit(cur, op_out_code, 7*4);
                jump_len = (void *)&putchar_proxy - (cur - 12);

                // relocation
                *(uint32_t *)(cur - 12) |= ((jump_len >> 2) & 0x3FFFFFF);
                break;
        }
    }
    emit(cur, op_ret_code, 4);

    assert(loopidx == 0);

    // Make executable
    mprotect(page, size_guess, PROT_READ | PROT_EXEC);
    return page;
}

int main(int argc, char *argv[]) {
    int dorun = 1;

    if (argc > 1 && !strcmp(argv[1], "dump")) {
        dorun = 0;
    }

    jitf f = compile(mb);

    if (!dorun) {
        dump(f);
        return 0;
    }

    uint8_t *tape = malloc(30000);
    f(0, tape);

    return 0;
}
