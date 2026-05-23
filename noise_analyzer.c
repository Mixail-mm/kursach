#include "stats_lib.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define MKDIR(dir) _mkdir(dir)
#else
#define MKDIR(dir) mkdir(dir, 0755)
#endif

typedef struct {
    float timestamp_ms;
    float lat_rad;
    float lon_rad;
    float alt_m;
    float Ve;
    float Vn;
} record_t;

size_t get_size_and_read(const char *fileName, record_t **arr) {
    FILE *FR = fopen(fileName, "rb");
    if (!FR) {
        fprintf(stderr, "[ОШИБКА] Не удалось открыть файл: %s\n", fileName);
        *arr = NULL;
        return 0;
    }
    fseek(FR, 0, SEEK_END);
    long fileSizeBytes = ftell(FR);
    fseek(FR, 0, SEEK_SET);
    if (fileSizeBytes <= 0) { fclose(FR); *arr = NULL; return 0; }

    size_t numberOfBlock = (size_t)fileSizeBytes / sizeof(record_t);
    record_t *tmp = (record_t *)malloc(numberOfBlock * sizeof(record_t));
    if (!tmp) { fclose(FR); *arr = NULL; return 0; }

    size_t size = fread(tmp, sizeof(record_t), numberOfBlock, FR);
    *arr = tmp;
    fclose(FR);
    return size;
}

float get_lat(const void *rec) { return ((const record_t *)rec)->lat_rad; }
float get_lon(const void *rec) { return ((const record_t *)rec)->lon_rad; }
float get_alt(const void *rec) { return ((const record_t *)rec)->alt_m; }
float get_Ve(const void *rec) { return ((const record_t *)rec)->Ve; }
float get_Vn(const void *rec) { return ((const record_t *)rec)->Vn; }

void get_field(const record_t *data, size_t n, const char *field_name,
               float (*getter)(const void *), FILE *out) {
    float *tmp = (float *)malloc(n * sizeof(float));
    if (!tmp) return;
    for (size_t i = 0; i < n; i++) tmp[i] = getter(&data[i]);
    Stats stats = stats_calculate(tmp, n);
    fprintf(out, "%e %e %e\n", stats.mathExp, stats.stdDev, stats.median);
    free(tmp);
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif

    FILE *OUT_REF   = fopen(OUTPUT_DIR "/outputRef.txt", "w");
    FILE *OUT_NOISE = fopen(OUTPUT_DIR "/outputNoise.txt", "w");
    FILE *OUT_CALC  = fopen(OUTPUT_DIR "/outputCalc.txt", "w");

    if (!OUT_REF || !OUT_NOISE || !OUT_CALC) {
        fprintf(stderr, "[ОШИБКА] Не удалось создать выходные файлы. Проверьте путь: %s\n", OUTPUT_DIR);
        return 1;
    }

    record_t *arrRef = NULL, *arrNoise = NULL;
    size_t countRef   = get_size_and_read("variant_21/ref.bin", &arrRef);
    size_t countNoise = get_size_and_read("variant_21/noisy.bin", &arrNoise);

    if (countRef == 0 || countNoise == 0) {
        fprintf(stderr, "[ОШИБКА] Входные файлы не найдены или пусты. Проверьте папку variant_21/\n");
        fclose(OUT_REF); fclose(OUT_NOISE); fclose(OUT_CALC);
        return 1;
    }

    for (size_t i = 0; i < countRef; i++)
        fprintf(OUT_REF, "%zu %f %f %f %f %f %f\n", i + 1, arrRef[i].timestamp_ms, arrRef[i].lat_rad, arrRef[i].lon_rad, arrRef[i].alt_m, arrRef[i].Ve, arrRef[i].Vn);
    for (size_t i = 0; i < countNoise; i++)
        fprintf(OUT_NOISE, "%zu %f %f %f %f %f %f\n", i + 1, arrNoise[i].timestamp_ms, arrNoise[i].lat_rad, arrNoise[i].lon_rad, arrNoise[i].alt_m, arrNoise[i].Ve, arrNoise[i].Vn);

    get_field(arrNoise, countNoise, "lat_rad", get_lat, OUT_CALC);
    get_field(arrNoise, countNoise, "lon_rad", get_lon, OUT_CALC);
    get_field(arrNoise, countNoise, "alt_m",   get_alt, OUT_CALC);
    get_field(arrNoise, countNoise, "Ve",      get_Ve,  OUT_CALC);
    get_field(arrNoise, countNoise, "Vn",      get_Vn,  OUT_CALC);

    free(arrRef); free(arrNoise);
    fclose(OUT_REF); fclose(OUT_NOISE); fclose(OUT_CALC);

    printf("[ГОТОВО] Анализ завершён. Файлы сохранены в: %s\n", OUTPUT_DIR);
    return 0;
}