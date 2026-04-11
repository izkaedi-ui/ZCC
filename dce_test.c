#include <stdio.h>

int main() {
    int x = 100;

    if (0) {
        x = 999;
    } else {
        x = 5;
    }

    while (0) {
        x = 5000;
    }

    return x;
}
