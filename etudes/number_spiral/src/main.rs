fn number_from_spiral(x: u64, y: u64) -> u64 {
    if x == 1 && y == 1 {
        return 1;
    }

    let biggest_coord = std::cmp::max(x, y);
    let max_value = biggest_coord * biggest_coord;
    let is_even = (biggest_coord % 2) == 0;
    if is_even {
        max_value - (biggest_coord - x) - (y - 1)
    } else {
        max_value - (biggest_coord - y) - (x - 1)
    }
}

fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();

    let num_tests = line.trim().parse::<u64>().unwrap();
    let mut results = Vec::with_capacity(num_tests as usize);
    for _ in 0..num_tests {
        let mut test_numbers = String::new();
        std::io::stdin().read_line(&mut test_numbers).unwrap();
        let mut parts = test_numbers.trim().split_whitespace().map(|s| s.parse::<u64>());
        let x = parts.next().unwrap().unwrap();
        let y = parts.next().unwrap().unwrap();
        results.push(number_from_spiral(x, y));
    }

    for result in results {
        println!("{result}");
    }
}
