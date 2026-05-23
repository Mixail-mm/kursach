#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "stats_lib.h"

#ifdef _WIN32
#include <windows.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

#define PI 3.14159265358979323846f
#define RE 6371000

typedef struct {
    float timestamp_ms;
    float lat_rad;
    float lon_rad;
    float alt_m;
    float Ve;
    float Vn;
} tr_point;

typedef struct {
    float phi;
    float lambda;
    float R;
    float Vn;
    float T;
    float dt;
    float h;
    float alpha0;
} circ_t;

float convert_to_rad(float angle) { return angle * (PI / 180.0f); }

void generate_trajectory(circ_t *parameters, tr_point **tr, int *count) {
    *count = (int)(parameters->T / parameters->dt) + 1;
    *tr = (tr_point *)malloc((*count) * sizeof(tr_point));
    if (!*tr) { *count = 0; return; }

    parameters->phi    = convert_to_rad(parameters->phi);
    parameters->lambda = convert_to_rad(parameters->lambda);
    parameters->alpha0 = convert_to_rad(parameters->alpha0);

    for (int i = 0; i < *count; i++) {
        float ti    = (float)i * parameters->dt;
        float alpha = parameters->alpha0 + (parameters->Vn / parameters->R) * ti;
        float xi    = parameters->R * cosf(alpha);
        float yi    = parameters->R * sinf(alpha);
        float dphi   = yi / RE;
        float phii   = parameters->phi + dphi;
        float dlabda = xi / (RE * cosf(phii));
        float lambdai = parameters->lambda + dlabda;
        float Vei = -parameters->Vn * sinf(alpha);
        float Vni =  parameters->Vn * cosf(alpha);

        (*tr)[i].timestamp_ms = ti;
        (*tr)[i].lat_rad      = phii;
        (*tr)[i].lon_rad      = lambdai;
        (*tr)[i].alt_m        = parameters->h;
        (*tr)[i].Ve           = Vei;
        (*tr)[i].Vn           = Vni;
    }
}

void save_to_file(char *fileName, tr_point *tr, int count, char *format) {
    FILE *file = fopen(fileName, format);
    if (!file) {
        fprintf(stderr, "[ОШИБКА] Не удалось открыть файл для записи: %s\n", fileName);
        return;
    }
    if (strcmp(format, "w") == 0) {
        for (int i = 0; i < count; i++)
            fprintf(file, "%f %f %f %f %f %f\n", tr[i].timestamp_ms, tr[i].lat_rad, tr[i].lon_rad, tr[i].alt_m, tr[i].Ve, tr[i].Vn);
    } else if (strcmp(format, "wb") == 0) {
        fwrite(tr, sizeof(tr_point), count, file);
    }
    fclose(file);
}

float N(float mathExp, float stdDev) {
    float u1 = (float)rand() / RAND_MAX;
    float u2 = (float)rand() / RAND_MAX;
    if (u1 < 1e-10f) u1 = 1e-10f;
    float z = sqrtf(-2.0f * logf(u1)) * cosf(2.0f * PI * u2);
    return z * stdDev + mathExp;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    srand((unsigned int)time(NULL));

    circ_t parameters = {
        .phi = 55.0f, .lambda = 37.0f, .R = 5000.0f, .Vn = 100.0f,
        .h   = 1000.0f, .T = 120.0f, .dt = 1.0f, .alpha0 = 0.0f
    };

    tr_point *tr = NULL;
    int count = 0;
    generate_trajectory(&parameters, &tr, &count);
    if (!tr) {
        fprintf(stderr, "[ОШИБКА] Не удалось выделить память для траектории.\n");
        return 1;
    }

    FILE *fParams = fopen(OUTPUT_DIR "/outputCalc.txt", "r");
    if (!fParams) {
        fprintf(stderr, "[ОШИБКА] Не найден файл outputCalc.txt. Сначала запустите noise_analyzer!\n");
        free(tr); return 1;
    }

    Stats *noise_params = (Stats *)malloc(5 * sizeof(Stats));
    if (!noise_params) { fclose(fParams); free(tr); return 1; }

    for (int i = 0; i < 5; i++) {
        if (fscanf(fParams, "%f %f %f", &noise_params[i].mathExp, &noise_params[i].stdDev, &noise_params[i].median) != 3) {
            fprintf(stderr, "[ОШИБКА] Ошибка чтения параметров шума.\n");
            free(noise_params); fclose(fParams); free(tr); return 1;
        }
    }
    fclose(fParams);

    for (int i = 0; i < count; i++) {
        tr[i].lat_rad += N(noise_params[0].mathExp, noise_params[0].stdDev);
        tr[i].lon_rad += N(noise_params[1].mathExp, noise_params[1].stdDev);
        tr[i].alt_m   += N(noise_params[2].mathExp, noise_params[2].stdDev);
        tr[i].Ve      += N(noise_params[3].mathExp, noise_params[3].stdDev);
        tr[i].Vn      += N(noise_params[4].mathExp, noise_params[4].stdDev);
    }

    save_to_file(OUTPUT_DIR "/trajectory.txt", tr, count, "w");
    save_to_file(OUTPUT_DIR "/trajectory.bin", tr, count, "wb");

    free(tr); free(noise_params);
    printf("[ГОТОВО] Траектория сгенерирована. Файлы сохранены в: %s\n", OUTPUT_DIR);
    return 0;
}