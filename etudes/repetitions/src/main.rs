fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    let mut current_char = 'Q';
    let mut current_counter = 0u64;
    let mut max_count = 0;

    for c in line.chars() {
        if c == current_char {
            current_counter += 1;
        } else if c == '\n' {
            break;
        } else {
            max_count = std::cmp::max(max_count, current_counter);
            current_counter = 1;
            current_char = c;
        }
    }
    max_count = std::cmp::max(max_count, current_counter);
    println!("{max_count}");
}
