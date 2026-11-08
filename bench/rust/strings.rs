fn main(){
 let mut chunk=String::new();for _ in 0..64{chunk=chunk.clone()+"abcdefghijklmnop";}
 let mut s=String::new();for _ in 0..1024{s=s.clone()+&chunk;}
 let mut h:i64=0;for b in s.bytes(){h=(h*31+b as i64)%1000003;}
 println!("{}\n{}",s.len(),h);}
