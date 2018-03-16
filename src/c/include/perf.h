#include <stdint.h>

#ifndef PERF_H
#define PERF_H

#ifdef __cplusplus
extern "C" {
#endif

void pmu_enable();
void pmu_disable();

void pmu_reset_pmn();
void pmu_reset_ccnt();

uint64_t pmu_read_ccnt();
void pmu_enable_ccnt();

uint64_t pmu_read_pmn(uint8_t counter);
void pmu_config_pmn(uint8_t counter, uint32_t event);
void pmu_enable_pmn(uint8_t counter);
void pmu_disable_pmn(uint8_t counter);

#ifdef __cplusplus
};
#endif

#endif