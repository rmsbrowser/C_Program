
#include <stdio.h>
int main()
{
   int n, t, sum = 0, remainder;

   //system("clear");

   printf("Enter an integer..: ");
   scanf("%d", &n);
   printf("\n");

   t = n;

   while (t != 0)
   {

      remainder = t % 10;
	  //printf("r -> %d ",remainder);
      
	  sum = sum + remainder;
	  printf(" %d \t",sum);
	  
      t = t / 10;
	  printf("<-\t %d \n",t);

   }

   printf("\n");
   printf("Sum of digits of %d = %d\n", n, sum);

   return 0;
}

