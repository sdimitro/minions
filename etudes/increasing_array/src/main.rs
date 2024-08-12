fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();

    let mut total_moves = 0u64;
    let mut previous = None;
    line = String:: new();
    std::io::stdin().read_line(&mut line).unwrap();
    for token in line.trim().split(" ") {
        match previous {
            Some(prev) => {
                let val = token.parse::<u64>().unwrap();
                if val < prev {
                    total_moves +=  prev - val;
                } else {
                    previous = Some(val);
                }
            } ,
            None => previous = Some(token.parse::<u64>().unwrap()),
        }
    }
    println!("{total_moves}");
}
