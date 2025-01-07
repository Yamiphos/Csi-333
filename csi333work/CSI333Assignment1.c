#include <stdio.h>
#include <math.h>

int main() {
    // 1st assignment
    

   double number1, number2, quotient, remainder;
	

   scanf("%lf",&number1);
   scanf("%lf",&number2);

   quotient = number1 / number2;
   remainder = fmod(number1,number2);

   printf("the quotient is: %f\n", quotient);
   printf("the remainder is: %f\n", remainder);


   return 0;
}