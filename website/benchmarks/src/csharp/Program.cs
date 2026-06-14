static long Fibonacci(long n)
{
    if (n <= 1) return n;
    long a = 0;
    long b = 1;
    for (long i = 2; i <= n; i++)
    {
        long next = a + b;
        a = b;
        b = next;
    }
    return b;
}

long total = 0;
for (long run = 0; run < 5_000_000; run++)
{
    total += Fibonacci(25 + (run % 8));
}
Console.WriteLine(total);
