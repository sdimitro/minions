def lpf(n):
    divisor = 2
    rem = n
    while rem != 1:
        if (rem % divisor == 0):
            rem /= divisor
        else:
            divisor += 1
    return divisor

print(lpf(600851475143))
