#define SIO_BASE 0xD0000000
#define SIO_GPIO_OUT_XOR 0xD000001C
#define SIO_GPIO_OE_SET 0xD0000024

__asm__(".section .boot2, \"ax\"\n.space 256, 0\n.section .vectors, \"ax\"\n.global _vectors\n_vectors:\n.word 0x20042000\n.word _reset + 1\n.word 0\n.word 0\n.text\n.global _reset\n.type _reset, %function\n_reset:\ncpsid i\nldr r0, =0x20042000\nmov sp, r0\nbl main\n1: b 1b\n");

void delay(int n) {
    while (n) {
        n = n - 1;
    }
}

int calculate_blinks(int a, int b) {
    int sum = a + b;
    int result = 0;
    if (sum > 2) {
        result = 2;
    } else {
        result = sum;
    }
    
    int x = result * 2;
    return x;
}

int main() {
    volatile unsigned int *gpio_oe = (volatile unsigned int *)SIO_GPIO_OE_SET;
    volatile unsigned int *gpio_xor = (volatile unsigned int *)SIO_GPIO_OUT_XOR;
    
    *gpio_oe = (1 << 25);
    
    int blinks = calculate_blinks(1, 2);
    
    while (1) {
        for (int i = 0; i < blinks; i = i + 1) {
            *gpio_xor = (1 << 25);
            delay(1000000);
            *gpio_xor = (1 << 25);
            delay(1000000);
        }
        delay(4000000);
    }
    
    return 0;
}
