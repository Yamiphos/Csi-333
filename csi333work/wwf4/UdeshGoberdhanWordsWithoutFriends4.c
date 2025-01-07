//web server
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 

//words without friends "includes"
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

//words without friends stuff
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
void handleGame(int sockfd, char *url);
int initialization();
void gameLoop();
void teardown();
void displayWorld(int client_sock);
void acceptInput(char *UrlwMove);
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
void finishGame(int sockFD);


//web server stuff
#define PORT 8000
#define BUFFER_SIZE 1024

char* base_dir;

void *handle_request(void *arg);
void serve_file(int newsockfd, const char *file_path);

//webserver code

int main(int argc, char *argv[]) {
	if (argc != 2) {
        printf("Please run with base directory as argument. \n");
        exit(EXIT_FAILURE);
    }
	
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    base_dir = argv[1];
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        int *new_sock_ptr = malloc(sizeof(int));
        *new_sock_ptr = new_socket;
        
        pthread_t thread;
        pthread_create(&thread, NULL, handle_request, new_sock_ptr);
        pthread_detach(thread);
    }

    if (new_socket < 0) {
        perror("Accept failed");
    }

    close(server_fd);
    return 0;
}

void *handle_request(void *arg) {
    int newsockfd = *(int *)arg;
    free(arg);
    
    char buffer[1024];
    ssize_t n = recv(newsockfd, buffer, sizeof(buffer), 0);
    if (n < 0) {
        perror("Error reading from socket");
        return NULL;
    }

    if (strncmp(buffer, "GET", 3) == 0) {
        char *file_path_start = buffer + 4;
        char *file_path_end = strchr(file_path_start, ' ');
        if (file_path_end != NULL) {
            *file_path_end = '\0';
        }

        // Added for Words Without Friends game logic
        if (strncmp(file_path_start, "/words", 6) == 0) {
			
            handleGame(newsockfd, file_path_start);
			
        } else {
            // Default behavior: Serve static file (HTML, etc.)
            serve_file(newsockfd, file_path_start);
        }
    }

    close(newsockfd);
    return NULL;
}


void serve_file(int newsockfd, const char *file_path) {
    struct stat file_stat;
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", base_dir, file_path);
    
    if (stat(full_path, &file_stat) == -1) {
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found";
        send(newsockfd, not_found_response, strlen(not_found_response), 0);
        return;
    }
    
    int file_fd = open(full_path, O_RDONLY);
    if (file_fd == -1) {
        const char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\nFile Not Found";
        send(newsockfd, not_found_response, strlen(not_found_response), 0);
    } else {
        // Send 200 OK response with file content
        char header[BUFFER_SIZE];
        snprintf(header, sizeof(header), 
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: text/html\r\n"
                 "Content-Length: %ld\r\n\r\n", file_stat.st_size);
        send(newsockfd, header, strlen(header), 0);

        // Read and send file content
        char file_buffer[1024];
        ssize_t n;
        while ((n = read(file_fd, file_buffer, sizeof(file_buffer))) > 0) {
            send(newsockfd, file_buffer, n, 0);
        }
        
        close(file_fd);
    }
}




//words without friends code below:


//dont need main
/*
int main() {
    initialization();
    gameLoop();
    teardown();
    return 0;
}*/

//handle game function

//sockfd to pass to display world, url to pass to accept input
void handleGame(int sockfd, char *url) {
    // If the URL only has "words", initialize the game
    if (strncmp(url, "/words", 6) == 0 && (url[6] == '\0')) {
        initialization();  // Initialize the game only if the URL is "/words"
    }

    // Continue processing the game if it's not over
    if (!isDone()) {
        

        // Check for "?move=" in the URL before calling acceptInput
        if (strstr(url, "?move=") != NULL) {
            acceptInput(url);  // Pass the original URL to acceptInput
        } else {
            printf("No valid move found in URL. Skipping acceptInput.\n");
        }
		displayWorld(sockfd);  // Show the current game state
		
    } else {
        // Game is over, handle the end of the game (e.g., show winner or lose message)
        finishGame(sockfd);
    }

}


void finishGame(int sockFD) {
    // Teardown the game (cleanup resources, reset state, etc.)
    teardown();

    // Prepare the HTML content
    const char *htmlResponse = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body>"
        "Congratulations! You solved it! "
        "<a href=\"/words\">Another?</a>"
        "</body></html>";

    // Send the HTML response to the client
    write(sockFD, htmlResponse, strlen(htmlResponse));
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

	
	masterword =getRandomWord(wordcount);
	if(masterword != NULL){
		printf("Master word selected(initialization): %s\n", masterword->word);
		findwords(masterword);
	}
	
	
    return wordcount; //changed from return 0;
}

//un-used
void gameLoop(int sock_fd, char *url_move){
    while(isDone() == false){ 
		displayWorld(sock_fd);
        acceptInput(url_move);
    }
}

//needs to be called with client sock to be able to write back
void displayWorld(int client_sock) {
    struct gameListNode *gmrp = gameroot; 
    sortList(gameroot);
    
    // Buffer for HTML response
    char buffer[30000];
	
	//master word checking
	if (masterword == NULL) {
    fprintf(stderr, "Error: Master word is NULL.\n");
    return;
	}

	//checking game list nodes
	if (gameroot == NULL) {
    fprintf(stderr, "Error: Game list is empty.\n");
    return;
	}

    // Start building the HTML response
    snprintf(buffer, sizeof(buffer),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html>\n"
        "<head>\n"
        "<title>Word Game</title>\n"
        "</head>\n"
        "<body>\n"
        "<h1>Word Game</h1>\n");
		

    // Display the master word
    strncat(buffer, "<p>Master Word: ", sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, masterword->word, sizeof(buffer) - strlen(buffer) - 1);
    strncat(buffer, "</p>\n", sizeof(buffer) - strlen(buffer) - 1);
	
    // List all words and their status (found/not found)
    strncat(buffer, "<h2>Words:</h2>\n<ul>\n", sizeof(buffer) - strlen(buffer) - 1);
    while (gmrp != NULL) {
        strncat(buffer, "<li>", sizeof(buffer) - strlen(buffer) - 1);
        if (gmrp->found) {
            strncat(buffer, gmrp->word, sizeof(buffer) - strlen(buffer) - 1);
        } else {
            for (size_t i = 0; i < strlen(gmrp->word); i++) {
                strncat(buffer, "-", sizeof(buffer) - strlen(buffer) - 1);
            }
        }
        strncat(buffer, "</li>\n", sizeof(buffer) - strlen(buffer) - 1);
        gmrp = gmrp->next;
    }
    strncat(buffer, "</ul>\n", sizeof(buffer) - strlen(buffer) - 1);
	

    // Add the form for submitting a move
    strncat(buffer, 
        "<form action=\"/words\" method=\"GET\">\n"
        "  <label for=\"move\">Enter your word:</label>\n"
        "  <input type=\"text\" id=\"move\" name=\"move\" autofocus required>\n"
        "  <button type=\"submit\">Submit</button>\n"
        "</form>\n", 
        sizeof(buffer) - strlen(buffer) - 1);
		
		

    // End HTML
    strncat(buffer, 
        "</body>\n"
        "</html>\n", 
        sizeof(buffer) - strlen(buffer) - 1);
		

    // Send the HTML response to the client
    write(client_sock, buffer, strlen(buffer));
}


		
	//old display world missing top bracket
	/*
	struct gameListNode *gmrp = gameroot; 
    sortList(gameroot);
	
	
    
    while (gmrp != NULL) {
        //printf("debugging print: %s\n", gmrp->word); 
		if(gmrp->found == true){
			printf("FOUND: %s\n", gmrp->word);
		// }
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
	}*/

//changed to take a url(char array) as a parameter, will parse to get the guess.
//url will always have "?move=" followed by the guess will just move past that and then get the guess
void acceptInput(char *UrlwMove) {
	printf("got to accept input before doing anything in there");
    char buf[100];               // Buffer to store the extracted guess
    char moveparse[] = "?move="; // Identifier to locate the guess in the URL
	
	printf("got to accept input before looking for move, after making arrays");
    // Find the substring "?move=" in the URL
    char *moveStart = strstr(UrlwMove, moveparse);
	//checking if movestart is null
	if (moveStart == NULL) {
    fprintf(stderr, "Error: Invalid URL, '?move=' not found.\n");
    return;
	}

	//checking game list nodes
	if (gameroot == NULL) {
    fprintf(stderr, "Error: Game list is empty.\n");
    return;
	}

    if (moveStart) {
        moveStart += strlen(moveparse); // Move the pointer past "?move="
        
        // Copy the guess into the buffer
        strncpy(buf, moveStart, sizeof(buf) - 1);
		buf[sizeof(buf) - 1] = '\0'; //Ensure null termination
		formatInput(buf); //formatting guess
		
		//rest of old logic, checking if word can be made and saying if it was correct or not
		if(compareCounts(buf, masterword->word)){
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
}
	
	
	
	//old accept input
	/*
    printf("Enter a guess: ");
    fgets(buf, sizeof(buf), stdin);
    formatInput(buf);
	
	if (strlen(buf) == 1) {
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
}*/

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
		dgmr = dgmr->next;
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