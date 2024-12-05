function qsort(A, left, right, i, last) {
        if (left >= right)
                return
        swap(A, left, left+int((right-left+1)*rand()))
        last = left
        for (i = left+1; i <= right; i++)
                if (A[i] < A[left])
                        swap(A, ++last, i)
        swap(A, left, last)
        qsort(A, left, last-1)
        qsort(A, last+1, right)
}

function swap(A, i, j, t) {
        t = A[i]; A[i] = A[j]; A[j] = t
}

function abs(v) {
        return v < 0 ? -v : v;
}

{
        lcol[NR] = $1 + 0
        rcol[NR] = $2 + 0
        line++

        freq[$1]++
        score[$2] += $2 + 0
}
END {
        qsort(lcol, 1, NR);
        qsort(rcol, 1, NR);
        for (i = 1; i <= NR; i++) {
                sum += abs(lcol[i] - rcol[i])
        }
        print("sum: ", sum)
        for (key in freq) {
                similarity += freq[key] * (score[key])
        }
        print("similarity score:", similarity)
}
