# Assignment 6 - Public Key Cryptography

This is a C program that aims at diving into the world of cryptography and implementing public and private key cryptography, whilst making use of the RSA algorithm. The program has three executable files, keygen, encrypt, and decrypt. The keygen will create two files, default rsa.pub and rsa.priv. Rsa.pub holds the public key while rsa.priv holds the private key. A call to encrypt will encrypt a given file and will output the encryption to another file (stdin and stdout are default), using rsa.pub as the default public key file. Decrypt will decrypt a given input file and will output the decryption to an output file (stdin and stdout are default for this procecss), using rsa.priv as the default private key file. 

## Building

Build the program using the Makefile which contians various targets, such as all, keygen (builds keygen only), encrypt (builds encrypt only), decrypt (build decrypts only), %.o:%.c (building of all the object and c files), clean (removes all executable and object files), and lastly format (formats the files using clang-format).

Run:
```
$make
```
or:
```
$make <specific target>
```

# Running

Run keygen program with:
```
$ ./keygen [-hv] [-b bits] -n pbfile -d pvfile
```

Run encrypt program with:
```
$ ./encrypt [-hv] [-i infile] [-o outfile] -n pubkey
```

Run decrypt program with:
```
$ ./decrypt [-hv] [-i infile] [-o outfile] -n privkey
```

Use `./program -h` on the programs above for more information on each OPTION above


