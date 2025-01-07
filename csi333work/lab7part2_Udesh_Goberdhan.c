#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


void processFile(char *myfile);

int main(){
	DIR *dir;
	struct dirent *entry;
	
	//opening directory
	dir = opendir(".");
    if (!dir) {
        perror("Failed to open directory");
        return 1;
    }
	
	//looping through directory
	while((entry=readdir(dir)) != NULL){
		//checking the name to compare to numbers.
		if (strncmp(entry->d_name, "numbers.", 8) == 0){
			printf("Found file: %s\n", entry->d_name);
            processFile(entry->d_name);
		}
	}
	
	// Closing the directory
	closedir(dir); 
    return 0;
}

void processFile(char *myfile){
	int sum=0;
	int numbers[100]={0};
	int count=0;
	
	//opening the file
	FILE *myfile1 = fopen(myfile, "r");
    if (myfile1 == NULL) {
        printf("Error opening file!\n");
        return;
    }
	
	//looping through and setting the array up as well as sum
	while (count < 100 && fscanf(myfile1, "%d", &numbers[count]) == 1) {
        sum += numbers[count]; 
        count++;
    }
	
	//closing the file
	fclose(myfile1); 

    // Output the numbers and their sum
    printf("Numbers read from the file:\n");
    for (int i = 0; i < count; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\nSum: %d\n", sum);
	
}