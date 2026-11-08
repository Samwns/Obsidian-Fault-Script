use std::hint::black_box;
fn main(){let mut s:i64=0;for i in 0..1000000i64{let b=black_box(Box::new([i,i+1]));s=(s+b[1])%1000000007;}println!("{}",s);}
