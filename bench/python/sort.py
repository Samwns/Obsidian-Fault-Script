import sys
sys.setrecursionlimit(10000)
def qs(a, lo, hi):
    if lo >= hi: return
    m = (lo+hi)//2; p = a[m]; a[m] = a[hi]; a[hi] = p; s = lo
    for i in range(lo, hi):
        if a[i] < p:
            a[i], a[s] = a[s], a[i]; s += 1
    a[hi] = a[s]; a[s] = p
    qs(a, lo, s-1); qs(a, s+1, hi)
n = 100000; x = 12345; a = []
for _ in range(n):
    x = (x*1103515245 + 12345) % 2147483648; a.append(x)
qs(a, 0, n-1)
c = 0
for i in range(n): c = (c + a[i] % 1000003 * (i+1)) % 1000000007
print(a[0]); print(a[n-1]); print(c)
