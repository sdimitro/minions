main = print(sum([x | x <- [1..999], mod x 3 == 0 || mod x 5 == 0]))
-- sum [3,6..999]  + sum [5,10..999] - sum [15,30..999] is also fun
