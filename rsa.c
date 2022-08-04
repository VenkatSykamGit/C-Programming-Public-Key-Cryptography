#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <gmp.h>
#include "rsa.h"
#include "randstate.h"
#include "numtheory.h"

// Creates all the necessary components of a public key
// Creates two primes, p and q, n = p*q, and also computes a fitting public exponent e
void rsa_make_pub(mpz_t p, mpz_t q, mpz_t n, mpz_t e, uint64_t nbits, uint64_t iters) {
    uint64_t remaining_bits;
    uint64_t p_bits;
    uint64_t q_bits;
    mpz_t totient;
    mpz_t p_minus_one;
    mpz_t q_minus_one;
    mpz_t curr_e;
    mpz_t curr_gcd;

    mpz_init(curr_e);
    mpz_init(curr_gcd);
    mpz_init(totient);
    mpz_init(p_minus_one);
    mpz_init(q_minus_one);

    p_bits = (random() % (nbits / 2)) + (nbits / 4);
    p_bits += 1;
    remaining_bits = nbits - p_bits; // q_bits is given the remaining bits of nbits
    q_bits = remaining_bits;
    q_bits += 1;

    make_prime(p, p_bits, iters);
    make_prime(q, q_bits, iters);

    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);
    mpz_mul(totient, p_minus_one, q_minus_one); // Setting totient to (p-1)(q-1)

    mpz_mul(n, p, q);

    while (mpz_cmp_ui(curr_gcd, 1) != 0) { // stop the loop when we find coprime with totient
        mpz_urandomb(curr_e, state, nbits);
        gcd(curr_gcd, curr_e, totient);
    }

    mpz_set(e, curr_e); // Set e

    mpz_clear(curr_e);
    mpz_clear(curr_gcd);
    mpz_clear(totient);
    mpz_clear(p_minus_one);
    mpz_clear(q_minus_one);

    return;
}

// Writes a public key to a specified pbfile
// Writes all components, including n, e, s, and username, as hexstrings
void rsa_write_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {

    gmp_fprintf(pbfile, "%Zx\n", n); // Prints out n with a trailing newline
    gmp_fprintf(pbfile, "%Zx\n", e); // Prints out e with a trailing newline
    gmp_fprintf(pbfile, "%Zx\n", s); // Prints out s with a trailing newline

    fprintf(pbfile, "%s\n", username);

    return;
}

// Reads a public key from a specified pbfile
// Scans in n, e, s, and username
void rsa_read_pub(mpz_t n, mpz_t e, mpz_t s, char username[], FILE *pbfile) {
    gmp_fscanf(pbfile, "%Zx\n", n); // Reads n into pbfile
    gmp_fscanf(pbfile, "%Zx\n", e); // Reads e into pbfile
    gmp_fscanf(pbfile, "%Zx\n", s); // Reads s into pbfile

    fscanf(pbfile, "%s\n", username);

    return;
}

// Makes a new private key and stores it in d
// Takes in two primes, p and q, and a public exponent e to compute the private key
void rsa_make_priv(mpz_t d, mpz_t e, mpz_t p, mpz_t q) {
    mpz_t p_minus_one;
    mpz_t q_minus_one;
    mpz_t totient;

    mpz_init(p_minus_one);
    mpz_init(q_minus_one);
    mpz_init(totient);

    mpz_sub_ui(p_minus_one, p, 1);
    mpz_sub_ui(q_minus_one, q, 1);
    mpz_mul(totient, p_minus_one, q_minus_one); // Setting totient to (p-1)(q-1)

    mod_inverse(d, e, totient); // Computes inverse of e modulo totient

    mpz_clears(q_minus_one, p_minus_one, totient, NULL);

    return;
}

// Writes the private key to a specified pvfile
// n and d are both written out as hexstrings
void rsa_write_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fprintf(pvfile, "%Zx\n", n); // Writes n as hexstring to private file
    gmp_fprintf(pvfile, "%Zx\n", d); // Writes d as hexstring to private file

    return;
}

// Reads a private key from a specified pvfile
// n and d are both read in as hexstrings using gmp_fscanf
void rsa_read_priv(mpz_t n, mpz_t d, FILE *pvfile) {
    gmp_fscanf(pvfile, "%Zx\n", n); // Reads n into pvfile
    gmp_fscanf(pvfile, "%Zx\n", d); // Reads d into pvfile

    return;
}

// Does RSA encryption by encrypting m using e and n
// Stores the ciphertext in c
// Computes the equation c = (m ^ e) (mod n)
void rsa_encrypt(mpz_t c, mpz_t m, mpz_t e, mpz_t n) {
    pow_mod(c, m, e, n); // Computes c = m^e (mod n)
    return;
}

// Encrypts the specified infile and writes out the encryption contents to the specified outfile
// Takes in a modulo n and a public exponent e
void rsa_encrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t e) {
    mpz_t m;
    mpz_t c;
    mpz_init(m);
    mpz_init(c);

    size_t k = (mpz_sizeinbase(n, 2) - 1) / 8; // calculate block size = (log2(n) - 1) / 8
    size_t j = 0;

    uint8_t *buffer = (uint8_t *) calloc(k, sizeof(uint8_t)); // Creates a buffer for the blocks
    buffer[0] = 0xFF;

    while (
        (j = fread(&buffer[1], sizeof(uint8_t), k - 1, infile))
        > 0) { // we need to know when there's 0 bytes left so we can't make read_bytes = 0 or else the loop wouldn't stop when needed

        mpz_import(m, j + 1, 1, sizeof(uint8_t), 1, 0, buffer);

        rsa_encrypt(c, m, e, n);

        gmp_fprintf(outfile, "%Zx\n", c); // Prints out the encrypted c to outfile
    }
    free(buffer);
    mpz_clears(m, c, NULL);
    return;
}

// Performs decryption by decrypting the ciphertext c using the private key d and modulus n
// Computes the equation D(c) = m = c^d (mod n)
void rsa_decrypt(mpz_t m, mpz_t c, mpz_t d, mpz_t n) {
    pow_mod(m, c, d, n); // Computes m = c^d (mod n)
    return;
}

// Decrypts the content of a specified infile and writes to a specified outfile
// Takes in n and d as parameters
void rsa_decrypt_file(FILE *infile, FILE *outfile, mpz_t n, mpz_t d) {
    mpz_t m;
    mpz_t c;
    mpz_init(m);
    mpz_init(c);

    size_t k = (mpz_sizeinbase(n, 2) - 1) / 8; // Computes block size using (log2(n) - 1) / 8
    size_t j = 0;

    uint8_t *buffer = (uint8_t *) calloc(k, sizeof(uint8_t)); // Create a buffer for the blocks

    while (gmp_fscanf(infile, "%Zx\n", c) > 0) {
        rsa_decrypt(m, c, d, n);
        mpz_export(buffer, &j, 1, sizeof(uint8_t), 1, 0, m);
        fwrite(&buffer[1], sizeof(uint8_t), j - 1, outfile); // Write out j-1 bytes to outfile
    }
    free(buffer);
    mpz_clears(m, c, NULL);
    return;
}

// Produces a signature by signing m using d and n
// Computes the equation S(m) = s = m^d (mod n)
void rsa_sign(mpz_t s, mpz_t m, mpz_t d, mpz_t n) {
    pow_mod(s, m, d, n); // Calculates the equation stated above
    return;
}

// Verifies the signature s, Returns true if s is verified (if t is same as m) and false otherwise
// Computes the equation t = V(S) = s^e (mod n)
bool rsa_verify(mpz_t m, mpz_t s, mpz_t e, mpz_t n) {
    mpz_t t;
    mpz_init(t);

    pow_mod(t, s, e, n); // Calculates the equation stated above

    if (mpz_cmp(t, m) == 0) { // Returns true only if t is the same as expected message
        mpz_clear(t);
        return true;
    }

    mpz_clear(t);
    return false;
}
