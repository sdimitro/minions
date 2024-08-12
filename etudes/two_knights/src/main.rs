fn two_knights_combinations(k: u64) -> u64 {
    if k == 1 {
        return 0;
    }

    let k_squared = k * k;
    let piece_combinations = (k_squared * (k_squared - 1)) >> 1;
    let attacking_positions = 4 * (k - 1) * (k - 2);
    piece_combinations - attacking_positions
}

fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    let n = line.trim().parse::<u64>().expect("cannot parse integer value");
    for k in 1..=n {
        println!("{}", two_knights_combinations(k));
    }
}
