fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    let n = line.trim().parse::<i64>().expect("invalud number");
    let sum = (n * (n + 1)) / 2;
    if sum % 2 != 0 {
        println!("NO");
    } else {
        let mut midpoint = sum / 2;
        let mut a = vec![];
        let mut b = vec![];
        for i in (1..=n).rev() {
            if midpoint - i >= 0 {
                a.push(i);
                midpoint -= i;
            } else {
                b.push(i);
            }
        }
        println!("YES");
        println!("{}", a.len());
        for i in a.iter().rev() {
            print!("{i} ");
        }
        println!("\n{}", b.len());
        for i in b.iter().rev() {
            print!("{i} ");
        }
        println!();
    }
}
