use anyhow::Result;
use std::collections::HashMap;
use std::env;
use std::fs::File;
use std::io::{BufRead, BufReader, Lines};
use std::path::Path;

fn read_lines<P>(filename: P) -> Result<Lines<BufReader<File>>>
where
    P: AsRef<Path>,
{
    let file = File::open(filename)?;
    Ok(BufReader::new(file).lines())
}

fn compute_similarity_score(file_path: &str) -> Result<u32> {
    let mut freq = HashMap::new();
    let mut score = HashMap::new();
    for line in read_lines(file_path)?.flatten() {
        let mut tokens = line.split_whitespace();

        let key = tokens.next().unwrap().parse::<u32>()?;
        *freq.entry(key).or_insert(0) += 1u32;
        let value = tokens.next().unwrap().parse::<u32>()?;
        *score.entry(value).or_insert(0) += value;
    }
    Ok(freq.into_iter().fold(0, |acc, (k, count)| {
        acc.checked_add(count.checked_mul(*score.get(&k).unwrap_or(&0)).unwrap())
            .unwrap()
    }))
}

fn main() {
    for arg in env::args().skip(1) {
        match compute_similarity_score(&arg) {
            Ok(similarity_score) => println!("{arg} - {similarity_score}"),
            Err(s) => eprintln!("error: {arg}: {s}"),
        }
    }
}
