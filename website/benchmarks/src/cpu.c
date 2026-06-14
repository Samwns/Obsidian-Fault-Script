#include <stdint.h>
#include <stdio.h>

static int64_t fibonacci(int64_t n) {
    if (n <= 1) return n;
    int64_t a = 0;
    int64_t b = 1;
    for (int64_t i = 2; i <= n; ++i) {
        int64_t next = a + b;
        a = b;
        b = next;
    }
    return b;
}

int main(void) {
    int64_t total = 0;
    for (int64_t run = 0; run < 5000000; ++run) {
        total += fibonacci(25 + (run % 8));
    }
    printf("%lld\n", (long long)total);
    return 0;
}
