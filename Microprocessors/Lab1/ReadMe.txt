Assembly for:
1.Hash is as long as the lenght of the string
2.For each latin CAPITAL letter add the {number}ASCII character multiplied by 2
3.For each latin lowercase letter add the ^2 of the distance of the {number} ASCII character 
4.for each number we have the following table 
0-->5
1-->12
2-->7
3-->6
4-->4
5-->11
6-->6
7-->3
8-->10
9-->32

5.Any other character is being ignored!

IF HASH is >9
1.Then we add the numbers of the hash and each digit ! .F.e for A9b3 4+130+21+1+6=164 -->1+6+4 
2.We need the mod 7 of the number above until we get a number from {0 to 9}  F.e  11 mod 7 =4
3.Then we write in assembly the following function.In our case n=4

  int fibonacci(int n)
   { 
      if(n==0)
        return 0;
       else if(n==1)
         return 1;
       else 
         return fibonacci(n-1)+fibonacci(n-2)
}

4.Store the final result anywhere :)
