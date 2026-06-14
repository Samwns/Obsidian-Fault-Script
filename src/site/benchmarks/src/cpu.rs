fn fibonacci(n: i64) -> i64 {
    if n <= 1 {
        return n;
    }
    let mut a = 0;
    let mut b = 1;
    for _ in 2..=n {
        let next = a + b;
        a = b;
        b = next;
    }
    b
}

fn main() {
    let mut total: i64 = 0;
    for run in 0..5_000_000 {
        total += fibonacci(25 + (run % 8));
    }
    println!("{total}");
}
