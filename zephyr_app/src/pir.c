#include "pir.h"

static int pir_state = 0; // default no motion

int pir_init(void) {
    return 0;   // satisfy test 1
}

int pir_read(void) {
    return pir_state;
}

void pir_set_state(int new_state) {
    pir_state = new_state;
}