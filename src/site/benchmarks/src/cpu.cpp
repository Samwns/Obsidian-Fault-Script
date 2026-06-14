#include <cstdint>
#include <iostream>

static std::int64_t fibonacci(std::int64_t n) {
    if (n <= 1) return n;
    std::int64_t a = 0;
    std::int64_t b = 1;
    for (std::int64_t i = 2; i <= n; ++i) {
        const auto next = a + b;
        a = b;
        b = next;
    }
    return b;
}

int main() {
    std::int64_t total = 0;
    for (std::int64_t run = 0; run < 5000000; ++run) {
        total += fibonacci(25 + (run % 8));
    }
    std::cout << total << '\n';
}
