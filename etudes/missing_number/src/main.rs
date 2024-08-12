fn get_set_bit(n: u64) -> u64 {
    let mut n = n;
    let mut inner_idx = 0;
    while n != 1 {
        n /= 2;
        inner_idx += 1;
    }
    return inner_idx + 1;
}

struct Bitmap {
    map: Vec<u64>,
    limit: u64,
}

impl Bitmap {
    fn new(size: usize) -> Self {
        let vec_size = (size / 64) + 1;
        Bitmap { map: vec![0; vec_size], limit: size as u64 }
    }

    fn insert(&mut self, n: u64) {
        let outer_idx = (n - 1) / 64;
        let inner_idx = (n - 1) % 64;
        let c = &mut self.map[outer_idx as usize];
        *c += 1 << inner_idx;
    }

    fn get_missing(&self) -> Option<u64> {
        let last_idx = (self.limit - 1) / 64;
        let last_inner_idx = ((self.limit - 1) % 64) as u32;
        let last_max = if last_inner_idx == 63 { u64::MAX } else { (1 << (last_inner_idx + 1)) - 1 };
        for (idx, &chunk) in self.map.iter().enumerate() {
            if idx == last_idx as usize {
                let xored = last_max ^ chunk;
                return Some((idx as u64 * 64) + get_set_bit(xored));
            }
            if chunk != u64::MAX {
                let xored = u64::MAX ^ chunk;
                return Some((idx as u64 * 64) + get_set_bit(xored));
            }
        }
        return None;
    }
}

fn main() {
    let mut line = String::new();
    std::io::stdin().read_line(&mut line).unwrap();
    let mut map = Bitmap::new(line.trim().parse().unwrap());
    // let mut map = Bitmap::new(200000);

    line = String:: new();
    std::io::stdin().read_line(&mut line).unwrap();
    for token in line.trim().split(" ") {
        //println!("got {token}");
        map.insert(token.parse().unwrap());
    }
    // for i in 1..200000 {
    //     map.insert(i);
    // }
    println!("{}", map.get_missing().unwrap());
}
