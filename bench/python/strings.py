chunk = ""
for _ in range(64): chunk = chunk + "abcdefghijklmnop"
s = ""
for _ in range(1024): s = s + chunk
h = 0
for b in s.encode(): h = (h*31 + b) % 1000003
print(len(s)); print(h)
