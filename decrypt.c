#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <gmp.h>
#include <unistd.h>
#include "numtheory.h"
#include "randstate.h"
#include "rsa.h"

#define OPTIONS "hi:o:n:v"

// Prints out help message when called for in the getopt() loop
void help_message(void) {
    printf("SYNOPSIS\n");
    printf("   Decrypts data using RSA decryption.\n");
    printf("   Encrypted data is encrypted by the encrypt program.\n");
    printf("\n");
    printf("USAGE\n");
    printf("   ./decrypt [-hv] [-i infile] [-o outfile] -n privkey\n");
    printf("\n");
    printf("OPTIONS\n");
    printf("   -h              Display program help and usage.\n");
    printf("   -v              Display verbose program output.\n");
    printf("   -i infile       Input file of data to decrypt (default: stdin).\n");
    printf("   -o outfile      Output file for decrypted data (default: stdout).\n");
    printf("   -n pvfile       Private key file (default: rsa.priv).\n");
    exit(0);
}

// Main function that holds the implementation of decrypting files
int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = stdin; // specified in asgn6.pdf
    FILE *outfile = stdout;
    FILE *pvfile;
    char *pvfile_path = "rsa.priv";
    bool verbose = false; // Set to false, only true if user does "-v"

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': help_message(); return -1;
        case 'i': infile = fopen(optarg, "r"); break;
        case 'o':
            outfile = fopen(optarg, "w");
            break; // Open with "w" so we can write decryption to outfile later in the program
        case 'n': pvfile_path = optarg; break;
        case 'v': verbose = true; break;
        }
    }

    pvfile = fopen(pvfile_path, "r"); // Open private key file

    if (pvfile == NULL) {
        printf("Error opening pvfile.\n");
        return -1;
    }

    mpz_t n, d;
    mpz_inits(n, d, NULL);

    rsa_read_priv(n, d, pvfile); // Read in n and d from pvfile

    if (verbose == true) {
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("d (%d bits) = %Zd\n", mpz_sizeinbase(d, 2), d);
    }

    rsa_decrypt_file(infile, outfile, n, d); // Decrypt infile and write it to outfile

    fclose(infile); // Close all the opened files
    fclose(outfile);
    fclose(pvfile);

    mpz_clears(n, d, NULL);
}
