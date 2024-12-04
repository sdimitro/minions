#![feature(binary_heap_into_iter_sorted)]

use std::collections::BinaryHeap;
use std::env;
use std::fs::File;
use std::io::{Lines, BufReader, BufRead};
use std::path::Path;

use anyhow::Result;

// Returns an Iterator to the Reader of the lines of the file.
fn read_lines<P>(filename: P) -> Result<Lines<BufReader<File>>>
where
    P: AsRef<Path>,
{
    let file = File::open(filename)?;
    Ok(BufReader::new(file).lines())
}

fn compute_absdiff_sum(file_path: &str) -> Result<u32> {
    let mut sorted_left_column = BinaryHeap::new();
    let mut sorted_right_column = BinaryHeap::new();
    for line in read_lines(file_path)?.flatten() {
        let mut tokens = line.split_whitespace();
        sorted_left_column.push(tokens.next().unwrap().parse::<u32>()?);
        sorted_right_column.push(tokens.next().unwrap().parse()?);
    }
    Ok(sorted_left_column
        .into_iter_sorted()
        .zip(sorted_right_column.into_iter_sorted())
        .fold(0, |acc, (l, r)| acc.checked_add(l.abs_diff(r)).unwrap()))
}

fn main() {
    for arg in env::args().skip(1) {
        match compute_absdiff_sum(&arg) {
            Ok(sum) => println!("{arg} - {sum}"),
            Err(s) => eprintln!("error: {arg}: {s}"),
        }
    }
}
