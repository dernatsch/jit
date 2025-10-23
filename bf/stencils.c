#include "jit.h"

extern void char_out(char);

extern STENCIL(NEXT);

STENCIL(op_add) {
    cell++;
    return NEXT(cell, tape);
}

STENCIL(op_sub) {
    cell--;
    return NEXT(cell, tape);
}

STENCIL(op_left) {
    *tape = cell;
    tape--;
    cell = *tape;
    return NEXT(cell, tape);
}

STENCIL(op_right) {
    *tape = cell;
    tape++;
    cell = *tape;
    return NEXT(cell, tape);
}

STENCIL(op_out) {
    char_out(cell);
    return NEXT(cell, tape);
}

void op_ret(void) {
    return;
}
