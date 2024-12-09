function is_ordered_in_range(A, len, min, max, skip, next_i, diff) {
    for (i = 1; i < len; i++) {
        next_i = i + 1
        if (i == skip) {
            continue
        }
        if (next_i == skip) {
            next_i = i + 2
        }
        if (next_i > NF) {
            break;
        }
        diff = A[next_i] - A[i]
        if (diff < min || diff > max) {
            return 0
        }
    }
    return 1
}

function is_increasing_skip(A, len, skip) {
    return is_ordered_in_range(A, len, 1, 3, skip)
}

function is_decreasing_skip(A, len, skip) {
    return is_ordered_in_range(A, len, -3, -1, skip)
}

function is_increasing(A, len) {
    return is_increasing_skip(A, len, -1)
}

function is_decreasing(A, len) {
    return is_decreasing_skip(A, len, -1)
}

{
    for (c = 1; c <= NF; c++) {
        nums[c] = $c
    }

    if (is_increasing(nums, NF) || is_decreasing(nums, NF)) {
        count++
    } else {
        for (q = 1; q <= NF; q++) {
            if (is_increasing_skip(nums, NF, q) || is_decreasing_skip(nums, NF, q)) {
                tolerance_count++
                break
            }
        }
    }
}

END {
    print("safe count:", count)
    print("tolerance count:", tolerance_count)
    print("total:", count + tolerance_count)
}
