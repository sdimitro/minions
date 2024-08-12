fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    let max = line.trim().parse::<usize>().unwrap();
    let mut evens = vec![];
    let mut odds = vec![];
    if max == 1 {
        println!("1");
        return;
    }
    if max == 3 || max == 2 {
        println!("NO SOLUTION");
        return;
    }
    for i in 1..(max + 1) {
        if i % 2 == 0 {
            evens.push(i);
        } else {
            odds.push(i);
        }
    }
    evens.append(&mut odds);
    for i in evens {
        print!("{i} ");
    }
    println!("");
}
