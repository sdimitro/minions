use anyhow::Result;
use lazy_static::lazy_static;
use regex::Regex;
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

lazy_static! {
    static ref MUL_REGEX: Regex = Regex::new(r"mul\(([0-9]+),([0-9]+)\)").unwrap();
    static ref MUL_REGEX_2: Regex = Regex::new(r"(mul\(\d{1,3}\,\d{1,3}\)|do\(\)|don\'t\(\))").unwrap();
}

fn mull_it_over(file_path: &str) -> Result<(u32, u32)> {
    let mut res1 = 0;
    let mut res2 = 0;
    let mut accepting = true;
    for line in read_lines(file_path)?.flatten() {
        for (_, [num1, num2]) in MUL_REGEX.captures_iter(&line).map(|c| c.extract()) {
            res1 += num1.parse::<u32>()? * num2.parse::<u32>()?;
        }
        for (_, [m]) in MUL_REGEX_2.captures_iter(&line).map(|c| c.extract()) {
            match m {
                "do()" => accepting = true,
                "don't()" => accepting = false,
                _ => if accepting {
                    let (_, [num1, num2]) = MUL_REGEX.captures(m).unwrap().extract();
                    res2 += num1.parse::<u32>()? * num2.parse::<u32>()?;
                },
            }
        }
    }
    return Ok((res1, res2));
}

fn main() {
    for arg in env::args().skip(1) {
        match mull_it_over(&arg) {
            Ok((res1, res2)) => println!("mull_it_over -> {res1}|{res2}"),
            Err(s) => eprintln!("error: {arg}: {s}"),
        }
    }
}