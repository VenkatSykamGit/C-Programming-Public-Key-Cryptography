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

// Print out the help message when called in the getopt() loop
void help_message(void) {
    printf("SYNOPSIS\n");
    printf("   Encrypts data using RSA encryption.\n");
    printf("   Encrypted data is decrypted by the decrypt program.\n");
    printf("\n");
    printf("USAGE\n");
    printf("   ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey\n");
    printf("\n");
    printf("OPTIONS\n");
    printf("   -h              Display program help and usage.\n");
    printf("   -v              Display verbose program output.\n");
    printf("   -i infile       Input file of data to encrypt (default: stdin).\n");
    printf("   -o outfile      Output file for encrypted data (default: stdout).\n");
    printf("   -n pbfile       Public key file (default: rsa.pub).\n");
    exit(0);
}

// Main function that contains the implementation to encrypt a file
int main(int argc, char **argv) {
    int opt = 0;
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *pbfile;
    char *pbfile_path = "rsa.pub";
    bool verbose = false;
    bool verify;
    char username[32]; // initialize username array to call rsa_read_pub later

    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'h': help_message(); return -1;
        case 'i': infile = fopen(optarg, "r"); break; // open if specified
        case 'o':
            outfile = fopen(optarg, "w");
            break; // open with "w" to be able to write encryption to outfile
        case 'n': pbfile_path = optarg; break;
        case 'v': verbose = true; break;
        }
    }

    pbfile = fopen(pbfile_path, "r"); // open the public key file

    if (pbfile == NULL) {
        printf("Error opening pbfile.\n");
        return -1;
    }

    mpz_t n, e, s, m;
    mpz_inits(n, e, s, m, NULL);

    rsa_read_pub(n, e, s, username, pbfile); // Reads in n, e, s, and username from pbfile

    if (verbose == true) {
        printf("user = %s\n", username);
        gmp_printf("s (%d bits) = %Zd\n", mpz_sizeinbase(s, 2), s);
        gmp_printf("n (%d bits) = %Zd\n", mpz_sizeinbase(n, 2), n);
        gmp_printf("e (%d bits) = %Zd\n", mpz_sizeinbase(e, 2), e);
    }

    mpz_set_str(m, username, 62); // Converting username
    verify = rsa_verify(m, s, e, n);
    if (verify == false) { // Error handling of verify
        printf("Error while verifying signature.\n");
        return -1;
    }

    rsa_encrypt_file(infile, outfile, n, e);

    fclose(infile); // Close all the opened files
    fclose(outfile);
    fclose(pbfile);

    mpz_clears(n, e, s, m, NULL);
}
