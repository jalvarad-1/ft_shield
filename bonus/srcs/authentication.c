#include "../includes/ft_shield.h"


int validar_otp(const char *clave_secreta_bin, size_t clave_secreta_longitud, const char *codigo_otp) {
    time_t tiempo_actual = time(NULL);
    size_t window = 1;
    unsigned int time_step_size = OATH_TOTP_DEFAULT_TIME_STEP_SIZE;
    time_t start_offset = 0;

    int resultado = oath_totp_validate(clave_secreta_bin, clave_secreta_longitud, tiempo_actual, time_step_size, start_offset, window, codigo_otp);

    return (resultado >= 0);
}


bool authenticate(char *codigo_otp) {
    if (oath_init() != OATH_OK) {
        fprintf(stderr, "Error initialising liboath\n");
        return false;
    }

    const char *clave_secreta_base32 = SECRET;

    // Decode
    char *clave_secreta_bin = NULL;
    size_t clave_secreta_longitud;

    int resultado = oath_base32_decode(clave_secreta_base32, strlen(clave_secreta_base32), &clave_secreta_bin, &clave_secreta_longitud);
    if (resultado != OATH_OK) {
        fprintf(stderr, "Error decoding the secret key\n");
        oath_done();
        return false;
    }

    // Validar el código OTP
    if (validar_otp(clave_secreta_bin, clave_secreta_longitud, codigo_otp)) {
        printf("Successful authentication\n");
        free(clave_secreta_bin);
        oath_done();
        return true;
    } else {
        printf("Invalid OTP code\n");
    }

    free(clave_secreta_bin);

    // Finalise the liboath library
    oath_done();

    return false;
}

// something like this to generate the qr
//qrencode -t ANSI256 -s 1.5 "otpauth://totp/MiServicio:usuario@example.com?secret=PATATA&issuer=MiServicio"
