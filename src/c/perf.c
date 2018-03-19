#include "include/perf.h"

// https://developer.arm.com/products/software-development-tools/ds-5-development-studio/resources/tutorials/using-the-pmu-event-counters-in-ds-5
// https://developer.arm.com/docs/ddi0500/latest/performance-monitor-unit/events

void pmu_enable() {
    uint64_t reg;
    asm("MRS %0, PMCR_EL0" : "=r" (reg));
    asm("MSR PMCR_EL0, %0" :: "r" (reg | 0x1));
}

void pmu_disable() {
    uint64_t reg;
    asm("MRS %0, PMCR_EL0" : "=r" (reg));
    asm("MSR PMCR_EL0, %0" :: "r" (reg & 0x1));
}

void pmu_reset_pmn() {
    uint64_t reg;
    asm("MRS %0, PMCR_EL0" : "=r" (reg));
    asm("MSR PMCR_EL0, %0" :: "r" (reg | 0x2)); // Set P Bit (Event counter reset)
}

void pmu_reset_ccnt() {
    uint64_t reg;
    asm("MRS %0, PMCR_EL0" : "=r" (reg));
    asm("MSR PMCR_EL0, %0" :: "r" (reg | 0x4)); // Set C Bit (Clock counter reset)
}

uint64_t pmu_read_ccnt() {
    uint64_t reg;
    asm volatile("MRS %0, PMCCNTR_EL0" : "=r" (reg));
    return reg;
}

void pmu_enable_ccnt() {
    uint64_t reg = 0x80000000;
    asm("MSR PMCNTENSET_EL0, %0" :: "r" (reg));
}

uint64_t pmu_read_pmn(uint8_t counter) {
    uint64_t reg = counter & 0x1F;
    asm("MSR PMSELR_EL0, %0" :: "r" (reg));
    asm("ISB");
    uint64_t val;
    asm("MRS %0, PMXEVCNTR_EL0" : "=r" (val));
    return val;
}

void pmu_config_pmn(uint8_t counter, uint32_t event) {
    uint64_t reg = counter & 0x1F;
    asm("MSR PMSELR_EL0, %0" :: "r" (reg));
    asm("ISB");
    asm("MSR PMXEVTYPER_EL0, %0" :: "r" (event));
}

void pmu_enable_pmn(uint8_t counter) {
    uint64_t reg = 1 << counter;
    asm("MSR PMCNTENSET_EL0, %0" :: "r" (reg));
}

void pmu_disable_pmn(uint8_t counter) {
    uint64_t reg = 1 << counter;
    asm("MSR PMCNTENCLR_EL0, %0" :: "r" (reg));
}