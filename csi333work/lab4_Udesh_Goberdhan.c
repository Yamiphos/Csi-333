#include <stdio.h>
#include <ctype.h>

int main() {
    
    char holder;
    
    FILE *myfile;
    
    myfile = fopen("myfiletest.txt", "r");
    
    if (myfile == NULL) {
        printf("The file did not open correctly :( \n");
        return 1; // Exit program if file can't be opened
    } else {
        printf("All good to run.\n");
    }
     
    printf("File contents:\n");
     
    while ((holder = getc(myfile)) != EOF) {
        if (isspace(holder)) {
            printf("\n");
            continue;
        }
        printf("%c", holder);
    }

    printf("\nAll done.\n");

    fclose(myfile); 

    return 0;
}
