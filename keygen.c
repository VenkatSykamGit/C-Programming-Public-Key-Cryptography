#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <gmp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include "numtheory.h"
#include "randstate.h"
#include "rsa.h"

#define OPTIONS "hb:i:n:d:s:v"

// Prints out the help message as specified by resources binary
void help_message(void) {
    printf("SYNOPSIS\n");
    printf("   Generates an RSA public/private key pair.\n");
    printf("\n");
    printf("USAGE\n");
    printf("   ./keygen [-hv] [-b bits] -n pbfile -d pvfile\n");
    printf("\n");
    printf("OPTIONS\n");
    printf("   -h              Display program help and usage.\n");
    printf("   -v              Display verbose program output.\n");
    printf("   -b bits         Minimum bits needed for public key n (default: 256).\n");
    printf("   -i confidence   Miller-Rabin iterations for testing primes (default: 50).\n");
    printf("   -n pbfile       Public key file (default: rsa.pub).\n");
    printf("   -d pvfile       Private key file (default: rsa.priv).\n");
    printf("   -s seed         Random seed for testing.\n");
    exit(0);
}

// Main program that contains the implementation of the generation of public and private keys
int main(int argc, char **argv) {
    int opt = 0;
    int nbits = 256; // default nbits
    int iters = 50; // specified by asgn6.pdf
    char *public_path = "rsa.pub";
    char *private_path = "rsa.priv";
    uint64_t SEED = time(NULL); // specified by asgn6.pdf
    bool verbose = false;
    FILE *pbfile;
    FILE *pvfile;
    char *user = "USER";
    char *username;

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) { // Loop through arguments
        switch (opt) {
        case 'h': help_message(); return -1;
        case 'b': nbits = atoi(optarg); break; // using atoi to convert optarg to the right type
        case 'i': iters = atoi(optarg); break;
        case 'n': public_path = optarg; break; // if specified, use new path
        case 'd': private_path = optarg; break;
        case 's': SEED = atoi(optarg); break;
        case 'v': verbose = true; break;
        }
    }

    pbfile = fopen(public_path, "w"); // open with "w" so we can write later
    pvfile = fopen(private_path, "w");

    if (pbfile == NULL) { // Check if there was an error opening the file, print error message
        printf("Error opening pbfile.\n");
        return -1;
    }

    if (pvfile == NULL) { // Check if file didn't open as expected, print error message
        printf("Error opening pvfile.\n");
        return -1;
    }

    mpz_t p, q, n, e, d, m, s;
    mpz_inits(p, q, n, e, d, m, s, NULL);
    fchmod(fileno(pvfile), 0600); // Setting permissions as specified by asgn6.pdf

    randstate_init(SEED);

    rsa_make_pub(p, q, n, e, nbits, iters); // make public key
    rsa_make_priv(d, e, p, q); // make private key

    username = getenv(user);

    mpz_set_str(m, username, 62);
    rsa_sign(s, m, d, n);

    rsa_write_pub(n, e, s, username, pbfile); // write out the keys to the specified file
    rsa_write_priv(n, d, pvfile);

    if (verbose == true) { // Print verbose statistics
        printf("user = %s\n", username);
        gmp_printf("s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("p (%d bits) = %Zd\n", mpz_sizeinbase(p, 2), p);
        gmp_printf("q (%d bits) = %Zd\n", mpz_sizeinbase(q, 2), q);
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    fclose(pbfile); // Close all the files we opened
    fclose(pvfile);
    randstate_clear();
    mpz_clears(p, q, n, e, d, m, s, NULL);
}
