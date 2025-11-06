#pragma once

#include <stdint.h>

#define PORT_KEYBOARD_DATA 0x60
#define PORT_KEYBOARD_STATUS 0x64

static inline uint8_t inb(uint16_t port)
{
	uint8_t result;
	__asm__ volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
	return result;
}

uint8_t keyboard_read_scancode(void)
{
	// Wait until bit 0 of status register is set
	while (!(inb(PORT_KEYBOARD_STATUS) & 1)) {
		// do nothing (just wait)
	}

	// Now there’s data ready — read it
	return inb(PORT_KEYBOARD_DATA);
}
