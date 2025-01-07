#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>


int initialization();
void gameLoop();
void teardown();
void displayWorld();
void acceptInput();
bool isDone();
bool formatInput(char *str);
void getLetterDistribution(char *str, int letter_count[]);
bool compareCounts(char *guess, char *availableletters);
//find words
struct wordListNode* getRandomWord(int wordCount);  
void findWords(struct wordListNode *masterWord);

//linked list for words
struct wordListNode {
    char guessedWord[30];
    struct wordListNode *next;
};
struct wordListNode *root = NULL;

struct gameListNode {
    char guessedWord[30];
	bool found;
    struct gameListNode *next;
};
struct gameListNode *gameRoot = NULL;


int main(int argc, char *argv[]) {
    initialization();
    gameLoop();
    teardown();
    return 0;
}

//main game functions
int initialization() {
    srand(time(NULL));
    
	 FILE *file = fopen("2of12.txt", "r");
    if (file == NULL) {
        printf("The file did not open correctly :( \n");
        return 1; // Exit program if file can't be opened
    }
	
	int wordcount =0;
	char buffer[31];
	struct wordListNode *current = NULL;
	
	while( fgets(buffer, sizeof(buffer), file)){
		buffer[strcspn(buffer, "\n")] = 0;
		//printf("Loaded word: %s\n", buffer);  // Addedto check if words are being read correctly
		
		struct wordListNode *newNode = (struct wordListNode *)malloc(sizeof(struct wordListNode));
		if (newNode == NULL) {
            printf("Error: Memory allocation failed\n");
            return -1;
        }
		
		strcpy(newNode->guessedWord, buffer);
		newNode->next = NULL;
		
		if (root == NULL) {
            root = newNode;  // First node becomes the root else current points to new node
        } else {
            current->next = newNode;
        }
		current = newNode;  
        wordcount++;
	}
	fclose(file);
	
	//call find and random word
	
	struct wordListNode *masterWord = getRandomWord(wordcount);
	if (masterWord != NULL) {
        printf("The master word is: %s\n", masterWord->guessedWord);
        
        // Find words that can be made from the master word
        //findWords(masterWord);
    }
	
	return wordcount;
}

void gameLoop() {
    do {
    displayWorld();
    acceptInput();

    if(isDone()){
        break;
        }
    }while(true);
}

void teardown(){
    printf("All done");
}

//gameloop functions
void displayWorld(){
    printf("---------------------\n");
}

void acceptInput() {
    char buffer[100];
    bool validInput = false;
	int letterChoicesDistribution[26] = {0};
    

    while (!validInput) {
        printf("Enter a guess:");
        fgets(buffer, sizeof(buffer), stdin);
        
        // Only proceed if formatInput returns true (valid input)
        if (formatInput(buffer)) {
            validInput = true;
            printf("You entered: %s\n", buffer);
			getLetterDistribution(buffer, letterChoicesDistribution);
			
			/* for testing
			for (int i = 0; i < 26; i++) {
            printf("%d, ", letterChoicesDistribution[i]);
			}*/
         
        } else {
            printf("Invalid input! Please enter letters only.\n");
        }
    }
}

bool isDone() {
    return true;
}

// other functions
bool formatInput(char *str) {
    int i = 0;
    bool isAlphabetic = true;

    while (str[i]) {
        if (str[i] == '\r' || str[i] == '\n') {
            str[i] = '\0';  // Null-terminate the string
            break;
        }
        
        // Check if character is a letter and Set to false if any non-letter is found
        if (!isalpha((unsigned char)str[i])) {
            isAlphabetic = false;  
        }

        str[i] = toupper((unsigned char)str[i]);  
        i++;
    }

    return isAlphabetic;
}

void getLetterDistribution(char *str, int letter_count[]) {
    // Initialize the letter count array
    for (int i = 0; i < 26; i++) {
        letter_count[i] = 0;
    }

    for (int i = 0; str[i] != '\0'; i++) {
        char ch = str[i];
        if (isalpha(ch)) {  // Check if the character is a letter
            letter_count[ch - 'A']++;  // Increment the corresponding index
        }
    }
}

bool compareCounts(char *guess, char *availableletters) {
    int wordDistribution[26] = {0};  // To hold the distribution of the guess
    int letterChoicesDistribution[26] = {0};  // To hold the distribution of available letters

    // Get the distributions for both the guess and available letters
    getLetterDistribution(guess, wordDistribution);
    getLetterDistribution(availableletters, letterChoicesDistribution);
    
    /*
    printf("Guess Distribution: ");
    for (int i = 0; i < 26; i++) {
            printf("%d, ", wordDistribution[i]);
    }
    printf("\n");

    printf("Available Letters Distribution: ");
    for (int i = 0; i < 26; i++) {
            printf("%d, ", letterChoicesDistribution[i]);
    }
    printf("\n");*/

    // Compare the distributions
    for (int i = 0; i < 26; i++) {
        if (wordDistribution[i] > letterChoicesDistribution[i]) {
            return false;  // If the word needs more of any letter than available, return false
        }
    }

    return true;  // Return true if all letters in wordDistribution can be made from letterChoicesDistribution
}


struct wordListNode* getRandomWord(int wordCount){
	if(wordCount == 0 || root == NULL){
		printf("the word list is empty\n");
		return NULL;
	}
	
	struct wordListNode *current = root;
    struct wordListNode *selectedWord = NULL;
	

	//current = root;
	int randomIndex = rand() % wordCount;
	int index = 0;
	
	while (current != NULL && index < randomIndex) {
        current = current->next;
        index++;
    }
	
    int pass = 0;
    while (pass < 2) {  // Ensure that we search the list a maximum of two times
        while (current != NULL) {
            if (strlen(current->guessedWord) > 6) {
                selectedWord = current;
                printf("Master word found: %s\n", selectedWord->guessedWord);  // Debug statement
                return selectedWord;
            }
            current = current->next;
        }
        // If we reached the end of the list, start again from the beginning
        current = root;
        pass++;
    }
	
	return NULL;
}
/*
void findWords(struct wordListNode *masterWord) {
    if (masterWord == NULL || root == NULL) {
        printf("No master word or empty dictionary.\n");
        return;
    }

    struct wordListNode *current = root;
    struct gameListNode *currentGame = NULL;

    while (current != NULL) {
        if (compareCounts(current->guessedWord, masterWord->guessedWord)) {
            // Create a new gameListNode for the found word
            struct gameListNode *newGameNode = (struct gameListNode *)malloc(sizeof(struct gameListNode));
            if (newGameNode == NULL) {
                printf("Error: Memory allocation failed\n");
                return;
            }

            // Initialize the gameListNode
            strcpy(newGameNode->guessedWord, current->guessedWord);
            newGameNode->found = false;
            newGameNode->next = NULL;

            // Add to the gameList linked list
            if (gameRoot == NULL) {
                gameRoot = newGameNode;
                currentGame = newGameNode;  // Initialize currentGame
            } else {
                currentGame->next = newGameNode;
                currentGame = newGameNode;  // Move the pointer to the new node
            }
            
            printf("Added word to game list: %s\n", newGameNode->guessedWord); // Debug statement
        }

        current = current->next;  // Move to the next word in the word list
    }

    if (gameRoot == NULL) {
        printf("No words found that can be made from the master word.\n");
    } else {
        printf("Words found and added to game list.\n");
    }
} */
