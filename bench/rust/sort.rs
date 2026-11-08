fn qs(a:&mut [i64],lo:isize,hi:isize){if lo>=hi{return;}let m=((lo+hi)/2) as usize;let p=a[m];a[m]=a[hi as usize];a[hi as usize]=p;let mut s=lo;
 for i in lo..hi{if a[i as usize]<p{a.swap(i as usize,s as usize);s+=1;}}
 a[hi as usize]=a[s as usize];a[s as usize]=p;qs(a,lo,s-1);qs(a,s+1,hi);}
fn main(){let n=100000usize;let mut a=Vec::new();let mut x:i64=12345;for _ in 0..n{x=(x*1103515245+12345)%2147483648;a.push(x);}
 qs(&mut a,0,n as isize-1);let mut c:i64=0;for i in 0..n{c=(c+a[i]%1000003*(i as i64+1))%1000000007;}
 println!("{}\n{}\n{}",a[0],a[n-1],c);}
