s = 0
for i in range(1000000):
    a = [i, i+1]
    s = (s + a[1]) % 1000000007
print(s)
