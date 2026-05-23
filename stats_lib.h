#ifndef KURSOVAYA_STATS_LIB_H
#define KURSOVAYA_STATS_LIB_H

#include <stddef.h>

typedef struct {
    float mathExp;
    float stdDev;
    float median;
} Stats;

float stats_mathExp(const float *data, size_t n);
float stats_stdDev(const float *data, size_t n, float mathExp);
float stats_median(const float *data, size_t n);
Stats stats_calculate(const float *data, size_t n);

#endif // KURSOVAYA_STATS_LIB_H