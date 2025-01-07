#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
	//initialization and set ups
    int lower = 0, upper = 99; 
    int randomNumber = 0;
	int sum= 0;
    srand(time(0));
	int numberarray[100] = {0};
	char myfile[50];

	// putting random numbers in the array
	for(int i = 0; i<100; i++){
	randomNumber = (rand() % (upper - lower + 1)) + lower;
	numberarray[i] = randomNumber;
	}
	
	//summing the total of the array
	for(int i = 0; i<100; i++){
	sum += numberarray[i];
	}
	
	//printing sum of the array
	printf("Sum of all random numbers: %d\n", sum);
	
	//making the file
	snprintf(myfile, sizeof(myfile), "numbers.%d", sum);

    // Open the file
    FILE *numbers = fopen(myfile, "w");
    if (numbers == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    // Write to the file
   for (int i = 0; i < 100; i++) {
        fprintf(numbers, "%d\n", numberarray[i]);
    }

    // Close the file
    fclose(numbers);
	
    return 0;
}
