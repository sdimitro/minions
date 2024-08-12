#!/usr/bin/python3

def qwerty(input_array):
    sorted_array = sorted(input_array)
    # sorted_array = input_array
    summa = sum(input_array)
    print(summa)
    half = summa / 2

    array1, array2 = [sorted_array.pop()], []
    sum1 = array1[0]
    while len(sorted_array) != 0:
        value = sorted_array.pop()
        if (half - (sum1 + value)) > 0:
            sum1 += value
            array1.append(value)
        else:
            array2.append(value)
    print(abs(sum(array1) - sum(array2)))
    return array1, array2
