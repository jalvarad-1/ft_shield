#include "../includes/ft_shield.h"


int validar_otp(const char *clave_secreta_bin, size_t clave_secreta_longitud, const char *codigo_otp) {
    time_t tiempo_actual = time(NULL);
    size_t window = 1; // Tolerancia de intervalos
    unsigned int time_step_size = OATH_TOTP_DEFAULT_TIME_STEP_SIZE; // Normalmente 30 segundos
    time_t start_offset = 0;

    // Validar el código OTP
    int resultado = oath_totp_validate(clave_secreta_bin, clave_secreta_longitud, tiempo_actual, time_step_size, start_offset, window, codigo_otp);

    // Si el resultado es mayor o igual a 0, el código es válido
    return (resultado >= 0);
}


bool authenticate(char *codigo_otp) {
    // Inicializar la biblioteca liboath
    if (oath_init() != OATH_OK) {
        fprintf(stderr, "Error al inicializar liboath\n");
        return false;
    }

    // Clave secreta en Base32 (definida por la macro SECRET)
    const char *clave_secreta_base32 = SECRET;

    // Decodificar la clave secreta de Base32 a binario
    char *clave_secreta_bin = NULL;
    size_t clave_secreta_longitud;

    int resultado = oath_base32_decode(clave_secreta_base32, strlen(clave_secreta_base32), &clave_secreta_bin, &clave_secreta_longitud);
    if (resultado != OATH_OK) {
        fprintf(stderr, "Error al decodificar la clave secreta\n");
        oath_done();
        return false;
    }

    // Validar el código OTP
    if (validar_otp(clave_secreta_bin, clave_secreta_longitud, codigo_otp)) {
        printf("Autenticación exitosa\n");
        free(clave_secreta_bin);
        oath_done();
        return true;
    } else {
        printf("Código OTP inválido\n");
    }

    // Liberar la memoria asignada
    free(clave_secreta_bin);

    // Finalizar la biblioteca liboath
    oath_done();

    return false;
}

// algo como esto para generar el qr
//qrencode -t ANSI256 -s 1.5 "otpauth://totp/MiServicio:usuario@example.com?secret=PATATA&issuer=MiServicio"