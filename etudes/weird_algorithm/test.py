def weird_algorithm(n):
    print(f"{int(n)} ")
    while n != 1:
        if n % 2 == 0:
            n /= 2
            print(f"{int(n)} ")
        else:
            n = (3 * n) + 1
            print(f"{int(n)} ")

y = input()
weird_algorithm(int(y))