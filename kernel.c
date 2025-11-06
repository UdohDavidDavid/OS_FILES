#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "io.h"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

const char scancode_table[] = {
	0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
	'\t', 'q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0, 'a','s','d','f','g','h','j','k','l',';','\'','`', 0,
	'\\','z','x','c','v','b','n','m',',','.','/', 0, '*',
	0, ' ', 0, // and so on...
};

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg)
{
	return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color)
{
	return (uint16_t) uc | (uint16_t) color << 8;
}

size_t strlen(const char* str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;


void terminal_initialize(void)
{
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_CYAN, VGA_COLOR_BLACK);

	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void scroll_up(void) {
	for (size_t y = 1; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			terminal_buffer[ (y-1) * VGA_WIDTH + x] = terminal_buffer[y * VGA_WIDTH + x];
		}
	}

	for (size_t x = 0; x < VGA_WIDTH; x++) {
		terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
	}
	terminal_row = VGA_HEIGHT - 1;
	terminal_column = 0;
}

void terminal_setcolor(uint8_t color)
{
	terminal_color = color;
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	const size_t index = y * VGA_WIDTH + x;
	if (c == '\n') {
		terminal_column = -1;
		terminal_row++;
	}
	else if (c == '\b') {
		terminal_buffer[index - 1] = vga_entry(' ', color);
		terminal_column -= 2;
	}
	else {
		terminal_buffer[index] = vga_entry(c, color);
	}
}

void terminal_putchar(char c)
{
	if (terminal_row >= VGA_HEIGHT) {
		scroll_up();
	}
	terminal_putentryat(c, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		terminal_row++;
	}
}

void terminal_write(const char* data, size_t size)
{
	for (size_t i = 0; i < size; i++)
		if (data[i] == '\n') {
			terminal_row++;
			terminal_column = 0;
			continue;
		}
		else {
			terminal_putchar(data[i]);
		}
}

void terminal_writestring(const char* data)
{
	terminal_write(data, strlen(data));
}

void clear_screen()
{
	terminal_row = 0;
	terminal_column = 0;
	for (int y = 0; y < VGA_HEIGHT; ++y) {
		for (int x = 0; x < VGA_HEIGHT; ++x) {
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
		}
	}
}

unsigned char get_char_from_scancode(uint8_t scancode)
{
	if (scancode > 58) return 0;
	return scancode_table[scancode];
}

void kernel_main(void)
{
	/* Initialize terminal interface */
	terminal_initialize();

	/* Newline support is left as an exercise. */
	terminal_writestring("Hello, kernel World!\n");
	terminal_writestring("New Line by me\n");
	terminal_writestring("This is going to be a very long line hahahahahahahahhahahahahahahaahahahahahahahhahahahahahhahaahahahahahahahahah\n");
	for (size_t i = 0; i < 100; i++) {
		terminal_writestring("Character\n");
	}
	terminal_writestring("Another line by me\n");
	terminal_writestring("New Line by me\n");
	clear_screen();
	while (1) {
		uint8_t scancode = keyboard_read_scancode();
		char key = get_char_from_scancode(scancode);
		if (key) {
			terminal_putchar(key);
		}
	}
}
