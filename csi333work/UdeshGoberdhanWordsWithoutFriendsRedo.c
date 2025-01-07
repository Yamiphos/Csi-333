#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//function prototypes
int initialization();
void gameLoop();
void teardown();
void displayWorld();
void acceptInput();
bool formatInput(char *str);
bool isDone();
bool compareCounts(char *guess, char *masterword);
void getLetterDistribution(char *str, int letterCount[]);
struct wordListNode* getRandomWord(int wordCount);
void findwords(struct wordListNode *masterword);

struct wordListNode{
    char word[30];
	struct wordListNode *next;
};
struct wordListNode *wordroot =NULL;


struct gameListNode{
    char word[30];
	bool found;
	struct gameListNode *next;
};
struct gameListNode *gameroot =NULL;
//main

//testing variable remove after making sure it works
int testingvariable = 0;


int main() {
    initialization();
    gameLoop();
    teardown();
    return 0;
}


//functions
int initialization(){
	
	int wordcount=0;
	FILE *f = fopen("testing2of12.txt", "r");
	char buffer[100];
	if( f== NULL){
		exit(1);
	}
	
	struct wordListNode *end = NULL;
	while(fgets(buffer,100,f) != NULL){
		
		buffer[strcspn(buffer, "\n")] = '\0';
		
		struct wordListNode *newNode = malloc(sizeof(struct wordListNode));
		if (newNode == NULL) {
			fclose(f);
			exit(1);
		}
	
		strcpy(newNode->word,buffer);
		newNode->next = NULL;
		if(wordroot ==NULL){
			wordroot = newNode;
			end = wordroot;
		}
		else{
			end->next = newNode;
			end = newNode;
		}
		wordcount++;
	}
	
	struct wordListNode *test = wordroot; //for testing
    while (test != NULL) {
        printf("added word: %s\n", test->word);
        test = test->next;
    }
	fclose(f);
	
	//finding and printing the masterword
	
	struct wordListNode *masterword = getRandomWord(wordcount);
	if(masterword != NULL){
		printf("Master word selected: %s\n", masterword->word);
		findwords(masterword);
	}
	
	
	
    srand(time(NULL));
	
    return wordcount; //changed from return 0;
}

void gameLoop(){
    while(isDone() == false){ 
        displayWorld();
        acceptInput();
    }
}

void displayWorld(){
    printf("------------------\n");
}

void acceptInput(){
    char buf[100];
	char availableletters[] = "ADEFNORRSSW";	//testing array for compareCounts
    
    printf("Enter a guess: ");
    fgets(buf, sizeof(buf), stdin);
    
    if(formatInput(buf)){  // Fix this condition
        printf("you made a proper guess and it was: %s", buf); //for debugging
		compareCounts(buf, availableletters); //testing compareCounts
	
        testingvariable++;
    }
    else{ 
        printf("you did not make a proper guess and it was: %s", buf); //for debugging will get removed
        testingvariable++;
    }
}

bool formatInput(char *str){
    bool formatted = true;
    int i=0;
    while (str[i]) {
        if(str[i] == '\n' || str[i] == '\0' || str[i] == '\r'){
            break;
        }
        if (!isalpha((unsigned char)str[i])) {
            formatted = false;  
        }
        str[i] = toupper(str[i]);
        i++;
    }
    return formatted;
}

bool isDone(){
    if(testingvariable == 1){ //testing  
        return true;
    }
    return false;
}

void teardown(){
    printf("all done\n");
}


void getLetterDistribution(char *str, int letterCount[]){
	
	for(int i=0; i<26; i++){
		letterCount[i] = 0;
	}
	
	for (int i =0; str[i] != '\0'; i++){
		char ch = str[i];
        if (isalpha(ch)) {  // Check if the character is a letter
            letterCount[ch - 'A']++;  // Increment the corresponding index
        }
	}
	
	
	
}

bool compareCounts(char *guess, char *masterword){
	int guessDistribution[26]= {0};
	int masterWordDistribution[26]={0};
	
	getLetterDistribution(guess, guessDistribution);
	getLetterDistribution(masterword, masterWordDistribution);
	
	/*
	//both prints are for testing
	printf("guess: ");
	for(int i=0;i<26;i++){
	printf("%d, ", guessDistribution[i]); // for testing
	}
	printf("\n");
	
	printf("masterword: ");
	for(int i=0;i<26;i++){
	printf("%d, ", masterWordDistribution[i]); //for testing
	}
	printf("\n");
	*/
	
	
	for (int i = 0; i < 26; i++) {
        if (guessDistribution[i] > masterWordDistribution[i]) {
            return false;  
        }
	}
		
	return true;
}


struct wordListNode* getRandomWord(int wordCount){
	
	int randomindex = rand() % wordCount;
	
	struct wordListNode *current = wordroot;
	struct wordListNode *masterWord = NULL;
	
	for(int i=0;i<randomindex;i++){
		if(current != NULL){
			current = current->next;
		}
		else{
			break;
		}
		
	}
	int pass=0;
	
	while(pass<2){
		
		while (current !=NULL){
			
			if(strlen(current->word) > 6){
			masterWord = current;
			printf("Master word found: %s\n", masterWord->word);
			return masterWord;
			}
			else{
			current = current->next;
			}
		}
		current = wordroot;
		pass++;
		
	}
	return NULL;
} 

void findwords(struct wordListNode *masterword){
	struct wordListNode *current = wordroot;
	struct gameListNode *gameListEnd = NULL;
	
	while(current != NULL){
	
		if(compareCounts(current->word,masterword->word)){
			struct gameListNode *newgamelistnode =  malloc(sizeof(struct gameListNode));
			
			if(newgamelistnode == NULL){
				exit(1);
			}
			strcpy(newgamelistnode->word, current->word);
			newgamelistnode->next = NULL;
			newgamelistnode->found = true;
			
			if (gameroot == NULL) {
                gameroot = newgamelistnode;
                gameListEnd = gameroot;
            } else {
                gameListEnd->next = newgamelistnode;
                gameListEnd = newgamelistnode;
            }
			printf("word added to the game list: %s" , newgamelistnode->word);
			
		}
		current =  current->next;
		
	}
}