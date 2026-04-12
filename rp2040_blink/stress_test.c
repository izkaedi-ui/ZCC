#include <stdint.h>

// RP2040 MMIO Map
#define SIO_BASE       0xD0000000
#define GPIO_OUT       (*(volatile uint32_t*)(SIO_BASE + 0x010))
#define GPIO_OUT_SET   (*(volatile uint32_t*)(SIO_BASE + 0x014))
#define GPIO_OUT_CLR   (*(volatile uint32_t*)(SIO_BASE + 0x018))
#define GPIO_OE_SET    (*(volatile uint32_t*)(SIO_BASE + 0x024))

#define IO_BANK0_BASE  0x40014000
#define GPIO25_CTRL    (*(volatile uint32_t*)(IO_BANK0_BASE + 0x0CC))

#define RESETS_BASE    0x4000C000
#define RESET          (*(volatile uint32_t*)(RESETS_BASE + 0x000))
#define RESET_DONE     (*(volatile uint32_t*)(RESETS_BASE + 0x008))

void delay(volatile uint32_t count) {
    while (count--) {
        // Spin
    }
}

// Function 1: Array and memory stress
int compute_sum() {
    int arr[5];
    int i = 0;
    while (i < 5) {
        arr[i] = i + 1;
        i++;
    }
    int sum = 0;
    i = 0;
    while (i < 5) {
        sum = sum + arr[i]; // 1 + 2 + 3 + 4 + 5 = 15
        i++;
    }
    return sum;
}

// Function 2: Conditionals, ternary, and arithmetic test
int get_blink_count() {
    int sum = compute_sum();
    // ternary condition
    int t = (sum == 15) ? 10 : 0;
    // returns 5
    return sum - t; 
}

// Function 3: Hardware abstraction
void blink_n(int count) {
    int i = 0;
    while (i < count) {
        GPIO_OUT_SET = (1 << 25);
        delay(200000); // short blink
        GPIO_OUT_CLR = (1 << 25);
        delay(400000); 
        i++;
    }
    delay(2000000); // Long pause before restart
}

int main() {
    // 1. Release reset on IO_BANK0
    RESET &= ~(1 << 5);

    // Wait for the reset to complete
    while ((RESET_DONE & (1 << 5)) == 0) {
        // Block until subsystem is online
    }

    // 2. Configure GPIO25 for Software Controlled IO (SIO)
    GPIO25_CTRL = 5;

    // 3. Enable Output Drive for GPIO25
    GPIO_OE_SET = (1 << 25);

    int n = get_blink_count();

    // 4. Infinite Blink Loop using computed value
    while (1) {
        blink_n(n);
    }

    return 0; // Unreachable
}
