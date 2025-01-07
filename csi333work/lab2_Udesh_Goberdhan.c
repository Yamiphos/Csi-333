#include <stdio.h>

void increment(int x) {
    x++;
}

void incrementByReference(int *x) {
    
    (*x)++;
}

int main() {
    int a = 0;
    int b = 0;
    int *pa;
    int *pb;

    pa = &a;
    pb = &b;

    *pa = 5;
    *pb = 6;

    printf("a = %d, b = %d, pa = %p, pb = %p\n", a, b, pa, pb);

    increment(b);  //this does not actually increase the value of b, probably because its passing the value or a copy of b and not b itself.
    
    printf(" a = %d, b = %d, pa = %p, pb = %p\n", a, b, pa, pb);

    incrementByReference(pb);  //this does increase b because it is passing the pointer of b and then dereferencing it to directly change the value at that address
    
    printf(" a = %d, b = %d, pa = %p, pb = %p\n", a, b, pa, pb);

    return 0;
}
