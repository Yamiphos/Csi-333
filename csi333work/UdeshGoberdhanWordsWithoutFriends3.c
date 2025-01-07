#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

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
struct wordListNode *masterword = NULL;

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
void swap(struct gameListNode* a, struct gameListNode* b);
void sortList(struct gameListNode* head);
void cleanupWordListNodes();
void cleanupGameListNodes();

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
	srand(time(NULL));
	
	int wordcount=0;
	FILE *f = fopen("2of12.txt", "r");
	char buffer[31];
	if( f== NULL){
		exit(1);
	}
	
	struct wordListNode *end = NULL;
	while(fgets(buffer,sizeof(buffer),f) != NULL){
		//buffer[strcspn(buffer, "\n")] = '\0';
		
		struct wordListNode *newNode;
		struct wordListNode *temp = malloc(sizeof(struct wordListNode));
		if (temp != NULL) {
			newNode = temp;
		}
		else{
			fclose(f);
			printf("memory allocation failed in initialization");
			return(-1);
		}
	
		formatInput(buffer);
		strcpy(newNode->word,buffer);
		//printf("%s \n", newNode->word);
		newNode->next = NULL;
		if(wordroot ==NULL){
			wordroot = newNode;
			end=wordroot;
		}
		else{
			end->next = newNode;
			end= newNode;
		}
	
		wordcount++;
	}
	fclose(f);

	//struct wordListNode *
	masterword =getRandomWord(wordcount);
	if(masterword != NULL){
		printf("Master word selected: %s\n", masterword->word);
		findwords(masterword);
	}
	
    return wordcount; //changed from return 0;
}

void gameLoop(){
    while(isDone() == false){ 
		displayWorld();
        acceptInput();
    }
}

void displayWorld(){
	struct gameListNode *gmrp = gameroot; 
    sortList(gameroot);
	
	
    
    while (gmrp != NULL) {
        //printf("debugging print: %s\n", gmrp->word); 
		if(gmrp->found == true){
			printf("FOUND: %s\n", gmrp->word);
		}
		else{
			size_t dashes = strlen(gmrp->word);
			for(int i=0; i<dashes; i++){
			printf("-");
			}
			printf("\n");
		}	
        gmrp = gmrp->next;
    }
	
    //printf("------------------\n");
}

void acceptInput(){
    char buf[100];
	
    
    printf("Enter a guess: ");
    fgets(buf, sizeof(buf), stdin);
    formatInput(buf);
	
	if (strlen(buf) == 1) {
		printf("single letter");
        buf[1] = '\0';
    }
	
    if(compareCounts(buf, masterword->word)){
        printf("you correctly guessed: %s", buf); 
		struct gameListNode *gmn = gameroot;
		//printf("%s \n", gmn->word);
		while(gmn !=NULL){
			//printf("current word: %s \n", gmn->word);
			//printf("buffer: %s \n", buf);
			if(strcmp(gmn->word,buf)==0)
			{
				gmn->found =true;
			}
		
			//printf("%s \n", gmn->word);
			gmn = gmn->next;
		}
        
    }
    else{ 
        printf("you did not make a proper guess and it was: %s", buf); //for debugging will get removed
        
    }
}

bool formatInput(char *str){
    bool formatted = true;
    int i=0;
	//&& str[i] != '\n' && str[i] != '\r'
    while (str[i]) {
    if (!isalpha((unsigned char)str[i])) {
        formatted = false;
		str[i] = '\0';
    }
    str[i] = toupper((unsigned char)str[i]); // cast to unsigned for safety
    i++;
}
    return formatted;
}

bool isDone(){
	struct gameListNode *dgmr =  gameroot;
	while(dgmr != NULL){
		if(dgmr->found == false){
			return false;
		}
	}
	return true;
	
	/*
    if(){  
        return true;
    }
    return false;*/
}

void teardown(){
	cleanupGameListNodes();
	cleanupWordListNodes();
    printf("all done\n");
}


void getLetterDistribution(char *str, int letterCount[]){
	
	for(int i=0; i<26; i++){
		letterCount[i] = 0;
	}
	
	for (int i =0; str[i] != '\0'; i++){
		char ch = str[i];
        if (isalpha(ch)) {  // Check if the character is a letter
			ch = toupper(ch);
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
	if(wordCount == 0 || wordroot == NULL){
		printf("the word list is empty\n");
		return NULL;
	}
	
	struct wordListNode *current = wordroot;
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
            if (strlen(current->word) > 6) {
                selectedWord = current;
                return selectedWord;
            }
            current = current->next;
        }
        // If we reached the end of the list, start again from the beginning
        current = wordroot;
        pass++;
    }
	
	return NULL;

} 

void findwords(struct wordListNode *masterword){
	struct wordListNode *current = wordroot;
	struct gameListNode *gameListEnd = NULL;
	while(current != NULL){
		if (compareCounts(current->word, masterword->word)){
			
			struct gameListNode *newNode = (struct gameListNode*)malloc(sizeof(struct gameListNode));
			if (newNode == NULL) {
			printf("memory allocation failed in initialization");
			}
	
			//printf("current word %s \n" ,current->word);
			strcpy(newNode->word, current->word);
			newNode->next = NULL;
			newNode->found = false;
			
			if(gameroot ==NULL){
			gameroot = newNode;
			gameListEnd = gameroot;
			}
			else{
			gameListEnd->next = newNode;
			gameListEnd = gameListEnd->next;
			}
			//printf("gamelistnodeword %s \n" ,newNode->word);
		}
		
		current = current->next;
		
	}
}

//following 2 for displayworld
void swap(struct gameListNode* a, struct gameListNode* b) {
    char temp[30];
    strcpy(temp, a->word);
    strcpy(a->word, b->word);
    strcpy(b->word, temp);
}

void sortList(struct gameListNode* head) {
    if (head == NULL) return;

    int swapped;
    struct gameListNode* ptr1;
    struct gameListNode* lptr = NULL;

    do {
        swapped = 0;
        ptr1 = head;

        while (ptr1->next != lptr) {
            if (strcmp(ptr1->word, ptr1->next->word) > 0) {
                swap(ptr1, ptr1->next);
                swapped = 1;
            }
            ptr1 = ptr1->next;
        }
        lptr = ptr1;
    } while (swapped);
}

//cleanups
void cleanupGameListNodes() {
    struct gameListNode *current = gameroot;
    struct gameListNode *temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);  
    }
    gameroot = NULL;  
}

void cleanupWordListNodes() {
    struct wordListNode *current = wordroot;
    struct wordListNode *temp;

    while (current != NULL) {
        temp = current;
        current = current->next;
        free(temp);  
    }
    wordroot = NULL; 
}