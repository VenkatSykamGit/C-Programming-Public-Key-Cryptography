#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "numtheory.h"
#include "randstate.h"
#include <gmp.h>

// Computes (base ^ exponent) % modulus
// No return value
void pow_mod(mpz_t out, mpz_t base, mpz_t exponent, mpz_t modulus) {
    mpz_t p;
    mpz_t v;
    mpz_t exponent_mod;
    mpz_t exponent_temp;
    mpz_t p_squared;

    mpz_init(p_squared);
    mpz_init_set(exponent_temp, exponent);
    mpz_init_set(exponent_mod, modulus);
    mpz_init_set_ui(v, 1);
    mpz_init_set(p, base);

    while (mpz_cmp_ui(exponent_temp, 0) > 0) { // Check if exponent is greater than 0
        if ((mpz_odd_p(exponent_temp)) != 0) { // Checks if exponent is odd
            mpz_mul(v, v, p);
            mpz_mod(v, v, exponent_mod);
        }
        mpz_mul(p_squared, p, p);
        mpz_mod(p, p_squared, exponent_mod);
        mpz_fdiv_q_ui(exponent_temp, exponent_temp, 2); // Floor division of exponent / 2
    }
    mpz_set(out, v); // Store result in out
    mpz_clear(p);
    mpz_clear(v);
    mpz_clear(exponent_mod);
    mpz_clear(exponent_temp);
    mpz_clear(p_squared);

    return;
}

// Miller-Rabin Test
// Checks if n is prime using Miller Rabin test, returns true if so, or false otherwise
bool is_prime(mpz_t n, uint64_t iters) {

    mpz_t r, s, y, j, a, n_minus_one, r_mod, s_minus_one, temp_exponent, n_minus_three;
    mpz_inits(r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, NULL);

    mpz_init_set_ui(temp_exponent, 2);
    mpz_sub_ui(n_minus_one, n, 1);
    mpz_sub_ui(n_minus_three, n, 3);

    if (mpz_cmp_ui(n, 0) == 0) { // if n = 0, return false
        mpz_clears(
            r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, temp_exponent, NULL);
        return false;
    }

    if (mpz_cmp_ui(n, 1) == 0) { // if n = 1, return false
        mpz_clears(
            r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, temp_exponent, NULL);
        return false;
    }

    if (mpz_cmp_ui(n, 2) == 0) { // if n = 2, return true
        mpz_clears(
            r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, temp_exponent, NULL);
        return true;
    }

    if (mpz_cmp_ui(n, 3) == 0) { // if n = 3, return true
        mpz_clears(
            r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, temp_exponent, NULL);
        return true;
    }

    if ((mpz_even_p(n) != 0)) { // if n is even, return false
        mpz_clears(
            r, y, j, a, s, n_minus_one, s_minus_one, r_mod, n_minus_three, temp_exponent, NULL);
        return false;
    }

    mpz_set(r, n_minus_one);
    while ((mpz_even_p(r) != 0)) { // while r is still even, keep dividing r by two
        mpz_fdiv_q_ui(r, r, 2);
        mpz_add_ui(s, s, 1);
    }

    mpz_sub_ui(s_minus_one, s, 1); // Initializing s - 1

    for (uint64_t i = 1; i <= iters; i++) {
        mpz_urandomm(a, state, n_minus_three);
        mpz_add_ui(a, a, 2); // Sets a to range [2, n-2]
        pow_mod(y, a, r, n);

        if ((mpz_cmp_ui(y, 1) != 0) && (mpz_cmp(y, n_minus_one) != 0)) {
            mpz_set_ui(j, 1);

            while ((mpz_cmp(j, s_minus_one) <= 0) && (mpz_cmp(y, n_minus_one) != 0)) {
                pow_mod(y, y, temp_exponent, n);

                if (mpz_cmp_ui(y, 1) == 0) { // if the pow_mod returns 1, return false
                    mpz_clears(r, s, y, j, a, n_minus_one, r_mod, s_minus_one, temp_exponent,
                        n_minus_three, NULL);
                    return false;
                }
                mpz_add_ui(j, j, 1);
            }

            if (mpz_cmp(y, n_minus_one) != 0) { // if y != n-1, return false
                mpz_clears(r, s, y, j, a, n_minus_one, r_mod, s_minus_one, temp_exponent,
                    n_minus_three, NULL);
                return false;
            }
        }
    }

    mpz_clears(r, s, y, j, a, n_minus_one, r_mod, s_minus_one, temp_exponent, n_minus_three, NULL);

    return true;
}

// Computes a prime p by generating random numbers until one is found
// No return value
void make_prime(mpz_t p, uint64_t bits, uint64_t iters) {
    mpz_t temp_p;
    mpz_init(temp_p);
    mpz_urandomb(temp_p, state, bits); // create an initial random p

    while (!(is_prime(temp_p, iters))
           || (mpz_sizeinbase(temp_p, 2)
               < bits)) { // While p is not prime or p is not at least bits amount of bits long
        mpz_urandomb(temp_p, state, bits);
    }

    mpz_set(p, temp_p);
    mpz_clear(temp_p);
    return;
}

// Computes the greatest common divisor of a and b
// Stores the value in d
void gcd(mpz_t d, mpz_t a, mpz_t b) {
    mpz_t t, a_temp, b_temp;
    mpz_init(t);
    mpz_init(a_temp);
    mpz_init(b_temp);
    mpz_set(a_temp, a);
    mpz_set(b_temp, b);

    while (mpz_cmp_ui(b_temp, 0) != 0) { // While b != 0
        mpz_set(t, b_temp);
        mpz_mod(b_temp, a_temp, t); // b = a % b
        mpz_set(a_temp, t);
    }
    mpz_set(d, a_temp); // Store value of a into d
    mpz_clear(t);
    mpz_clear(a_temp);
    mpz_clear(b_temp);

    return;
}

// An inverse function that finds the inverse i of a modulo n
// Sets i to 0 if no inverse is found
void mod_inverse(mpz_t i, mpz_t a, mpz_t n) {

    mpz_t t, t_prime, r, r_prime, q, x, y, temp, temp_two;

    mpz_init(x);
    mpz_init(y);
    mpz_init(q);
    mpz_init_set(r, n);
    mpz_init_set(r_prime, a);
    mpz_init_set_ui(t, 0);
    mpz_init_set_ui(t_prime, 1);
    mpz_init(temp);
    mpz_init(temp_two);

    while (mpz_cmp_ui(r_prime, 0) != 0) { // While r' != 0
        mpz_fdiv_q(q, r, r_prime);
        mpz_set(x, r);
        mpz_set(r, r_prime);
        mpz_mul(temp, q, r_prime); // Following pemdas order, multiply first, then subtract
        mpz_sub(r_prime, x, temp); // Sets r' = r - q x r'

        mpz_set(y, t);
        mpz_set(t, t_prime);
        mpz_mul(temp_two, q, t_prime);
        mpz_sub(t_prime, y, temp_two); // Sets t' = t - q x t'
    }

    if (mpz_cmp_ui(r, 1) > 0) { // If no inverse found, set i to 0
        mpz_set_ui(i, 0);
        mpz_clears(x, y, q, r, r_prime, t, t_prime, temp, temp_two, NULL);
        return;
    }
    if (mpz_cmp_ui(t, 0) < 0) {
        mpz_add(t, t, n);
    }

    mpz_set(i, t);

    mpz_clear(x);
    mpz_clear(y);
    mpz_clear(q);
    mpz_clear(r);
    mpz_clear(r_prime);
    mpz_clear(t);
    mpz_clear(t_prime);
    mpz_clear(temp);
    mpz_clear(temp_two);

    return;
}
