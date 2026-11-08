s = 0
for i in range(100000000):
    s = (s + i*i) % 1000000007
print(s)
