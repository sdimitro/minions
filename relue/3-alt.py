import math

def lpf(n):
    lastfactor = 1
    if (n % 2 == 0):
        lastfactor = 2
        n /= 2
        while (n % 2 == 0):
            n /= 2
    factor = 3
    limit = math.sqrt(n)
    while n > 1 and factor <= limit:
        if (n % factor == 0):
            n /= factor
            lastfactor = factor
            while (n % factor == 0):
                n /= factor
            limit = math.sqrt(n)
        factor += 2
    if n == 1:
        return lastfactor
    else:
        return n

print(lpf(600851475143))
