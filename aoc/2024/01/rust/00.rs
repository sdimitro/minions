use std::collections::BinaryHeap;
use std::env;
use std::fs::File;
use std::io::{self, BufRead};
use std::path::Path;

// The output is wrapped in a Result to allow matching on errors.
// Returns an Iterator to the Reader of the lines of the file.
fn read_lines<P>(filename: P) -> io::Result<io::Lines<io::BufReader<File>>>
where P: AsRef<Path>, {
    let file = File::open(filename)?;
    Ok(io::BufReader::new(file).lines())
}

fn compute_absdiff_sum(file_path: &str) -> io::Result<u32> {
    let mut sorted_column_0 = BinaryHeap::new();
    let mut sorted_column_1 = BinaryHeap::new();
    for line in read_lines(file_path)?.flatten() {
        let mut tokens = line.split_whitespace();
        sorted_column_0.push(tokens.next().unwrap().parse::<u32>().unwrap());
        sorted_column_1.push(tokens.next().unwrap().parse::<u32>().unwrap());
    }

    let mut sum = 0u32;
    while !sorted_column_0.is_empty() {
        sum += sorted_column_0.pop().unwrap().abs_diff(sorted_column_1.pop().unwrap());
    }
    return Ok(sum);
}

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();
    for arg in args {
        match compute_absdiff_sum(&arg) {
            Ok(sum) => println!("{arg} - {sum}"),
            Err(s) => println!("error: {arg}: {s}"),
        }
    }
}
