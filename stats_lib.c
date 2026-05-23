#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "stats_lib.h"

int comp(const void *a, const void *b) {
    float fa = *(const float *)a;
    float fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

float stats_mathExp(const float *data, size_t n) {
    if (n == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < n; i++) sum += data[i];
    return sum / (float)n;
}

float stats_stdDev(const float *data, size_t n, float mathExp) {
    if (n == 0) return 0.0f;
    float sum = 0.0f;
    for (size_t i = 0; i < n; i++) sum += powf(data[i] - mathExp, 2);
    return sqrtf(sum / (float)n);
}

float stats_median(const float *data, size_t n) {
    if (n == 0) return 0.0f;
    float *sorted_copy = (float *)malloc(n * sizeof(float));
    if (!sorted_copy) return 0.0f;
    for (size_t i = 0; i < n; i++) sorted_copy[i] = data[i];

    qsort(sorted_copy, n, sizeof(float), comp);

    float result;
    if (n % 2 == 1) {
        result = sorted_copy[(n + 1) / 2 - 1];
    } else {
        result = (sorted_copy[n / 2 - 1] + sorted_copy[n / 2]) / 2.0f;
    }
    free(sorted_copy);
    return result;
}

Stats stats_calculate(const float *data, size_t n) {
    Stats result = {0.0f, 0.0f, 0.0f};
    result.mathExp = stats_mathExp(data, n);
    result.stdDev  = stats_stdDev(data, n, result.mathExp);
    result.median  = stats_median(data, n);
    return result;
}