#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <oath.h>
#include <time.h>

int main() {
    // Initialise liboath
    if (oath_init() != OATH_OK) {
        fprintf(stderr, "Error initialising liboath\n");
        return EXIT_FAILURE;
    }

    // Generate a random 10-byte secret key
    uint8_t secret_bin[10];
    srand((unsigned int)time(NULL));
    for (int i = 0; i < 10; i++) {
        secret_bin[i] = rand() % 256;
    }

    // Encrypt the secret key in Base32
    char *secret_base32 = NULL;
    size_t secret_base32_len;
    if (oath_base32_encode((const char *)secret_bin, sizeof(secret_bin), \
        &secret_base32, &secret_base32_len) != OATH_OK) {
        fprintf(stderr, "Error when encoding the secret key in Base32\n");
        oath_done();
        return EXIT_FAILURE;
    }

    secret_base32[secret_base32_len] = '\0';
    printf("%s", secret_base32);
    free(secret_base32);
    oath_done();

    return EXIT_SUCCESS;
}