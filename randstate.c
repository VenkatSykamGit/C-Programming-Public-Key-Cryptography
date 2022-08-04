#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "randstate.h"
#include <gmp.h>

gmp_randstate_t state;

// Initializes the global state and uses the seed parameter for random seed
// No return value
void randstate_init(uint64_t seed) {
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, seed); // setting seed for random seed
}
// Clears and frees all the memory associated with the initialization of the state variable
void randstate_clear(void) {
    gmp_randclear(state);
}
