#include <stdio.h>
#include <string.h>

int main() {
    char String[100];
    int stringlength = 0;
    
    printf("Enter a string: ");
    
    fgets(String, sizeof(String), stdin);
    
    while(String[stringlength] != '\0'){
        stringlength++;
    }
    
    printf("the calculated string length is %d\n", stringlength);
    printf("the string length is %ld\n", strlen(String)); 
    
    return 0;
}
