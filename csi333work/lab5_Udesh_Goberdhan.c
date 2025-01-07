#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Define the Node structure
struct Node {
    int data;
    struct Node *next;
};

// Function to create a new node
struct Node* createNode(int data) {
    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Function to print and free the linked list
void printAndFreeList(struct Node *head) {
    struct Node *temp;
    while (head != NULL) {
        printf("%d -> ", head->data);
        temp = head;
        head = head->next;
        free(temp); 
    }
    printf("NULL\n");
}

int main() {
    struct Node *head = NULL, *tail = NULL, *temp = NULL;
    int randomNum;

    srand(time(0));

    while (1) {
        randomNum = rand() % 51; 
        if (randomNum == 49) {
            // If 49, print list and free memory
            printAndFreeList(head);
            break;
        }

        temp = createNode(randomNum);

        if (head == NULL) {
            head = temp;
            tail = head;
        } else {
            tail->next = temp;
            tail = tail->next;
        }
    }

    return 0;
}
