#include "idt.h"

/* VGA Text Framebuffer metrics & buffer mapping */
#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_ATTRIB 0x0F /* White text on Black background */

static char *vga_buffer = (char *)0xB8000;
static int cursor_row = 0;
static int cursor_col = 0;

/* Monotonic tick counter */
volatile unsigned long long tick_count = 0;

/* Global IDT table carrying 256 gates */
struct idt_entry_64 idt[256];

/* Helper to clear visual screen */
void kclear_screen(void) {
    int i;
    for (i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        vga_buffer[i * 2] = ' ';
        vga_buffer[i * 2 + 1] = VGA_ATTRIB;
    }
    cursor_row = 0;
    cursor_col = 0;
}

/* Helper to scroll screen up by 1 row */
static void kscroll(void) {
    int r, c;
    for (r = 1; r < VGA_ROWS; r++) {
        for (c = 0; c < VGA_COLS; c++) {
            vga_buffer[((r - 1) * VGA_COLS + c) * 2] = vga_buffer[(r * VGA_COLS + c) * 2];
            vga_buffer[((r - 1) * VGA_COLS + c) * 2 + 1] = vga_buffer[(r * VGA_COLS + c) * 2 + 1];
        }
    }
    /* Clear last row */
    for (c = 0; c < VGA_COLS; c++) {
        vga_buffer[((VGA_ROWS - 1) * VGA_COLS + c) * 2] = ' ';
        vga_buffer[((VGA_ROWS - 1) * VGA_COLS + c) * 2 + 1] = VGA_ATTRIB;
    }
    cursor_row = VGA_ROWS - 1;
}

/* Print a single character to VGA monitor */
void kprint_char(char c) {
    if (c == '\n') {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_ROWS) {
            kscroll();
        }
        return;
    }
    if (c == '\r') {
        cursor_col = 0;
        return;
    }
    if (c == '\b') {
        if (cursor_col > 0) {
            cursor_col--;
            vga_buffer[(cursor_row * VGA_COLS + cursor_col) * 2] = ' ';
        }
        return;
    }
    if (c == '\t') {
        cursor_col = (cursor_col + 8) & ~(8 - 1);
        if (cursor_col >= VGA_COLS) {
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= VGA_ROWS) {
                kscroll();
            }
        }
        return;
    }

    vga_buffer[(cursor_row * VGA_COLS + cursor_col) * 2] = c;
    vga_buffer[(cursor_row * VGA_COLS + cursor_col) * 2 + 1] = VGA_ATTRIB;
    cursor_col++;
    if (cursor_col >= VGA_COLS) {
        cursor_col = 0;
        cursor_row++;
        if (cursor_row >= VGA_ROWS) {
            kscroll();
        }
    }
}

/* Print a null-terminated string */
void kprint_string(const char *s) {
    int i;
    for (i = 0; s[i] != '\0'; i++) {
        kprint_char(s[i]);
    }
}

/* Print hex formatting value deterministically */
void kprint_hex(unsigned long long val) {
    char buf[32];
    int i = 0;
    if (val == 0) {
        kprint_char('0');
        return;
    }
    while (val > 0) {
        int rem = val % 16;
        if (rem < 10) {
            buf[i++] = '0' + rem;
        } else {
            buf[i++] = 'A' + (rem - 10);
        }
        val /= 16;
    }
    /* print in reverse */
    while (i > 0) {
        kprint_char(buf[--i]);
    }
}

/* Print decimal formatting value deterministically */
void kprint_dec(long long val) {
    char buf[32];
    int i = 0;
    int is_neg = 0;
    if (val == 0) {
        kprint_char('0');
        return;
    }
    if (val < 0) {
        is_neg = 1;
        val = -val;
    }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    if (is_neg) {
        kprint_char('-');
    }
    while (i > 0) {
        kprint_char(buf[--i]);
    }
}

/* IDT Descriptor Wiring */
void idt_set_gate(int num, unsigned long base, unsigned short sel, unsigned char flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].offset_middle = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].reserved = 0;
}

/* Remap PIC vectors to bypass CPU exception ports */
void pic_remap(void) {
    outb(0x20, 0x11); /* ICW1: init master */
    outb(0xA0, 0x11); /* ICW1: init slave */
    outb(0x21, 0x20); /* ICW2: master offset = 32 */
    outb(0xA1, 0x28); /* ICW2: slave offset = 40 */
    outb(0x21, 0x04); /* ICW3: master master-slave link */
    outb(0xA1, 0x02); /* ICW3: slave cascade identity */
    outb(0x21, 0x01); /* ICW4: 8086 mode */
    outb(0xA1, 0x01); /* ICW4: 8086 mode */
    outb(0x21, 0xFC); /* Enable only Timer (IRQ0) and Keyboard (IRQ1) */
    outb(0xA1, 0xFF); /* Disable all slave IRQs */
}

/* Master C Interrupt Dispatcher called by isr_common_stub */
void handle_interrupt(unsigned long vector) {
    if (vector == 32) {
        /* Timer Interrupt */
        tick_count++;
        if (tick_count % 100 == 0) {
            kprint_string(" [HEARTBEAT] ");
        }
        outb(0x20, 0x20); /* EOI to PIC Master */
    } else if (vector == 33) {
        /* Keyboard Interrupt */
        unsigned char scancode = inb(0x60);
        if (scancode < 0x80) {
            static const char keymap[128] = {
                0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
                '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
                0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
                '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
            };
            char c = keymap[scancode];
            if (c != 0) {
                kprint_char(c);
            }
        }
        outb(0x20, 0x20); /* EOI to PIC Master (IRQ1) to prevent locks */
    } else {
        /* CPU Exception */
        kprint_string("\n*** EXCEPTION VECTOR DETECTED: ");
        kprint_dec(vector);
        kprint_string(" ***\n");
        while (1) {
            cli_native();
            /* halt processor */
        }
    }
}

void kwrite_serial(const char *msg) {
    int i;
    for (i = 0; msg[i] != '\0'; i++) {
        while ((inb(0x3F8 + 5) & 0x20) == 0) {
            /* busy wait */
        }
        outb(0x3F8, (unsigned char)msg[i]);
    }
}

void kmain(void) {
    /* 1. Initialize COM1 port */
    outb(0x3F8 + 1, 0x00);    /* Disable all interrupts */
    outb(0x3F8 + 3, 0x80);    /* Enable DLAB */
    outb(0x3F8 + 0, 0x01);    /* Divisor 1 (lo) */
    outb(0x3F8 + 1, 0x00);    /*         1 (hi) */
    outb(0x3F8 + 3, 0x03);    /* 8N1 protocol */
    outb(0x3F8 + 2, 0xC7);    /* FIFO init */
    outb(0x3F8 + 4, 0x0B);    /* IRQ enabled */

    /* 2. Visual Framebuffer Setup */
    kclear_screen();
    kprint_string("================================================================================\n");
    kprint_string("                  ZKAEDI BARE-METAL KERNEL v2 OPERATIONAL                       \n");
    kprint_string("================================================================================\n\n");
    kprint_string("[SYSTEM] VGA Monitor identity mapped at 0xB8000.\n");
    kprint_string("[SYSTEM] Display Engine loaded cleanly. No varargs overhead.\n");

    /* 3. Interrupt Descriptors Setup */
    pic_remap();
    kprint_string("[SYSTEM] 8259 PIC remapped to vectors 32-47.\n");

    /* Clear IDT entries */
    {
        int i;
        for (i = 0; i < 256; i++) {
            idt_set_gate(i, 0, 0, 0);
        }
    }

    /* Set exception and hardware interrupt gates */
    idt_set_gate(0, (unsigned long)isr0, 0x08, 0x8E);
    idt_set_gate(8, (unsigned long)isr8, 0x08, 0x8E);
    idt_set_gate(13, (unsigned long)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned long)isr14, 0x08, 0x8E);
    idt_set_gate(32, (unsigned long)isr32, 0x08, 0x8E);
    idt_set_gate(33, (unsigned long)isr33, 0x08, 0x8E);

    /* Load IDT Register */
    {
        struct idt_ptr_64 idt_p;
        idt_p.limit = (sizeof(struct idt_entry_64) * 256) - 1;
        idt_p.base = (unsigned long)&idt;
        lidt_native(&idt_p);
    }
    kprint_string("[SYSTEM] 64-bit IDT descriptors registered.\n");

    /* 4. Programmable Interval Timer Setup (100 Hz) */
    outb(0x43, 0x36);         /* Mode 3: Square Wave */
    outb(0x40, 0x9C);         /* Divisor low: 0x9C */
    outb(0x40, 0x2E);         /* Divisor high: 0x2E */
    kprint_string("[SYSTEM] PIT scheduled for 100 Hz clock ticks.\n");

    /* 5. Enable CPU Interrupts & Run Idle Scheduler Tick Loop */
    sti_native();
    kwrite_serial("[ZKAEDI_V2_BOOT_SUCCESS]\n");
    kprint_string("[SYSTEM] interrupts active. Clock loop online. Boot successful.\n\n");
    kprint_string("kmain console input echo > ");

    while (1) {
        __asm__ __volatile__("hlt");
    }
}
