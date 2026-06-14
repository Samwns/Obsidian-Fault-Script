public final class CpuBench {
    private static long fibonacci(long n) {
        if (n <= 1) return n;
        long a = 0;
        long b = 1;
        for (long i = 2; i <= n; i++) {
            long next = a + b;
            a = b;
            b = next;
        }
        return b;
    }

    public static void main(String[] args) {
        long total = 0;
        for (long run = 0; run < 5_000_000; run++) {
            total += fibonacci(25 + (run % 8));
        }
        System.out.println(total);
    }
}
