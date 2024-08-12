use std::collections::BinaryHeap;
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::io::Lines;
use std::io::Result;
use std::path::Path;

fn read_lines<P>(filename: P) -> Result<Lines<BufReader<File>>>
where P: AsRef<Path>, {
    let file = File::open(filename)?;
    Ok(BufReader::new(file).lines())
}

fn main() {
    let k = 3;
    let mut heap = BinaryHeap::with_capacity(k);
    if let Ok(lines) = read_lines("./data/10-samples.txt") {
        for line in lines.map_while(Result::ok) {
            let number = line.parse::<u32>().unwrap();

            if heap.len() == k && heap.peek().unwrap() > &number {
            }

            if heap.len() < k {
                heap.push(number);
            }
        }
    }

    while let Some(number) = heap.pop() {
        println!("{number:?}");
    }
}
