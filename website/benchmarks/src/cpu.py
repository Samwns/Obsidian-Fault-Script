def fibonacci(n: int) -> int:
    if n <= 1:
        return n
    a = 0
    b = 1
    for _ in range(2, n + 1):
        a, b = b, a + b
    return b


total = 0
for run in range(5_000_000):
    total += fibonacci(25 + (run % 8))

print(total)
