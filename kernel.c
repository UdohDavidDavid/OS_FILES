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


int strcmp(const char *s1, const char *s2)
{
	while (*s1 && (*s1 == *s2)) {
		s1++;
		s2++;
	}
	return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int split(const char *str, char split, char (*result)[50])
{
	int word_index = 0;
	int char_index = 0;

	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] == split) {
			if (char_index > 0) {
				result[word_index][char_index] = '\0';
				word_index++;
				char_index = 0;
			}
		} else {
			result[word_index][char_index++] = str[i];
		}
	}
	/*
	if (strcmp(result[word_index], " ")) {
		result[word_index][char_index] = '\0';
		return word_index + 1;
	}*/

    // only add the last word if it’s not empty
    if (char_index > 0) {
        result[word_index][char_index] = '\0';
        word_index++;
    }

	return word_index;
}

/*
int strtoken(const char *str, const char split, char (*result)[50])
{
	//char *argv[200];
	char value[200];
	int count = 0;
	int value_index = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] != ' ') {
			value[value_index++] = str[i];
		} else {
			value[value_index] = '\0';
			for (int y = 0; value[y] != '\0'; y++) {
				result[count][y] = value[y];
			}
			result[count][value_index] = '\0';
			count++;
			value_index = 0;
		}
	}
	return count;
}
*/


#define VGA_WIDTH   80
#define VGA_HEIGHT  25
#define VGA_MEMORY  0xB8000

size_t terminal_row;
size_t terminal_column;
uint8_t terminal_color;
uint16_t* terminal_buffer = (uint16_t*)VGA_MEMORY;
int column;

char *argslist[80];
char argsstr[2000];

char command_result[20][50];

char *shell_term = "#shell >> ";

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
		terminal_column = (strlen(shell_term) - 1);
		terminal_row++;
	} else if (c == '\b') {
        // Do nothing
    } else {
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
		} else {
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
		for (int x = 0; x < VGA_WIDTH; ++x) {
			terminal_buffer[y * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
		}
	}
}

unsigned char get_char_from_scancode(uint8_t scancode)
{
	if (scancode > 58) return 0;
	return scancode_table[scancode];
}

void update_cursor(int x, int y)
{
	uint16_t pos = y * VGA_WIDTH + x;

	outb(0x3D4, 0x0F);
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

char get_char_from_pos(int x, int y)
{
	uint16_t cell = terminal_buffer[y * VGA_WIDTH + x];
	return (char)(cell & 0x00FF);
}

void get_string_of_char(int x, char key)
{
	/*int good_x = x - strlen(shell_term) - 1;
	if (++x == VGA_WIDTH) {
		goo
	}
	argsstr[good_x] = key;*/
	argsstr[x] = key;
}

void backspace_argstr( int *column)
{
	*column -= 1;
	int i;
	for (i = *column; argsstr[i] != '\0'; ++i) {
		argsstr[i] = ' ';
	}
}

void show_current_command(int y)
{
	int prev_y = y - 1;
    //int index = prev_y * VGA_WIDTH + VGA_WIDTH;
    char command[80];
	int count = 0;

    for (int x = strlen(shell_term); x < VGA_WIDTH; ++x) {
		char c = get_char_from_pos(x, prev_y);
		if (c == ' ' || c == '\0')
			break;
		command[count++] = c;
    }
	command[count] = '\0';

	if (strcmp(command, "clear") == 0) {
		clear_screen();
	}

	//terminal_writestring(command);
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
	clear_screen();

    /* Programming the shell now */
	terminal_writestring(shell_term);
    update_cursor(terminal_column, terminal_row);
	while (1) {
		uint8_t scancode = keyboard_read_scancode();
		char key = get_char_from_scancode(scancode);
		if (key) {
			if (key == '\n') {
				column = 0;
				terminal_column = 0;
                terminal_row++;
				show_current_command(terminal_row);
				int count = split(argsstr, ' ', command_result);
				for (int i = 0; i < count; i++) {
					terminal_writestring("[");
					terminal_writestring(command_result[i]);
					terminal_writestring("]");
					terminal_writestring("\n");
				}
				terminal_writestring(shell_term);
				for (int i = 0; argsstr[i] != '\0'; ++i) {
					argsstr[i] = '\0';
				}
                update_cursor(terminal_column, terminal_row);
			} else if (scancode == 0x01) {
                clear_screen();
                terminal_writestring(shell_term);
                update_cursor(terminal_column, terminal_row);
			} else if (key == '\b') {
				backspace_argstr(&column);
                if (!(terminal_buffer[terminal_row * VGA_WIDTH] == vga_entry('#', terminal_color) && terminal_column <= strlen(shell_term))) {
                    if (terminal_column == 0) {
                        terminal_column = VGA_WIDTH;
                        terminal_row -= 1;
                        update_cursor(terminal_column, terminal_row);
                    } else {
                        terminal_column--;
                        terminal_buffer[terminal_row * VGA_WIDTH + terminal_column] = vga_entry(' ', terminal_color);
                        update_cursor(terminal_column, terminal_row);
                    }
                }
            } else {
                terminal_putchar(key);
				get_string_of_char(column, key);
				column++;
                update_cursor(terminal_column, terminal_row);
            }
		}
	}
}
