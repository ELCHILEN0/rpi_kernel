#include "include/peripheral.h"

// Memory-Mapped I/O output
inline void mmio_write(uint32_t reg, uint32_t data)
{
	*(volatile uint32_t*)reg = data;
}

// Memory-Mapped I/O input
inline uint32_t mmio_read(uint32_t reg)
{
	return *(volatile uint32_t*)reg;
}

// Memory-Mapped I/O output
inline void mmio_write64(uint64_t reg, uint32_t data)
{
	*(volatile uint64_t*)reg = data;
}

// Memory-Mapped I/O input
inline uint32_t mmio_read64(uint64_t reg)
{
	return *(volatile uint64_t*)reg;
}

// Loop <delay> times in a way that the compiler won't optimize away
static void delay(int32_t count)
{
	asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
		 : "=r"(count): [count]"0"(count) : "cc");
}
