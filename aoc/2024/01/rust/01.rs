use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::BufRead;
use std::path::Path;

fn read_lines<P>(filename: P) -> std::io::Result<std::io::Lines<std::io::BufReader<File>>>
where P: AsRef<Path>, {
    let file = File::open(filename)?;
    Ok(std::io::BufReader::new(file).lines())
}

fn compute_similarity_score(file_path: &str) -> std::io::Result<u32> {
    let mut freq = HashMap::new();
    let mut score = HashMap::new();

    for line in read_lines(file_path)?.flatten() {
        let mut tokens = line.split_whitespace();

        let key = tokens.next().unwrap().parse::<u32>().unwrap();
        *freq.entry(key).or_insert(0) += 1;
        let value = tokens.next().unwrap().parse::<u32>().unwrap();
        *score.entry(value).or_insert(0) += value;
    }
    
    let mut similarity = 0u32;
    for (k, count) in freq.drain() {
            similarity += count * score.get(&k).unwrap_or(&0);
    }
    return Ok(similarity);
}

fn main() {
    let args: Vec<String> = env::args().skip(1).collect();

    for arg in args {
        match compute_similarity_score(&arg) {
            Ok(similarity_score) => println!("{arg} - {similarity_score}"),
            Err(s) => println!("error: {arg}: {s}"),
        }
    }
}
