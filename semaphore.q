//This file was generated from (Commercial) UPPAAL 4.0.14 (rev. 5615), May 2014

/*

*/
A[] D.cs imply not (A.cs || B.cs || C.cs)

/*

*/
A[] (not Mutex.overflow) and (not roomEmpty.overflow) and( not turnstile.overflow)

/*

*/
A[]  A.cs imply not (B.cs || C.cs )

/*

*/
A[] not deadlock
