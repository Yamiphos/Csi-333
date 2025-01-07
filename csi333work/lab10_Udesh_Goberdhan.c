#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define ARRAY_SIZE 1000000
#define NUM_THREADS 2

// Global variables
int *array;
int global_sum = 0; // Shared global variable for threads

void *sum_array(void *arg) {
    // Thread arguments
    int start = *(int *)arg;
    int end = start + ARRAY_SIZE / NUM_THREADS;

    for (int i = start; i < end; i++) {
        global_sum += array[i]; // No synchronization, race condition occurs
    }

    pthread_exit(NULL);
}

int main() {
    // Allocate memory for the array
    array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (!array) {
        perror("Failed to allocate memory");
        return 1;
    }

    // Seed random number generator and fill array
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = rand() % 10;
    }

    // Compute the sum in the main thread
    int real_sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        real_sum += array[i];
    }
    printf("Real sum: %d\n", real_sum);

    // Create threads
    pthread_t threads[NUM_THREADS];
    int start_indices[NUM_THREADS] = {0, ARRAY_SIZE / 2};

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, sum_array, &start_indices[i]) != 0) {
            perror("Failed to create thread");
            free(array);
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Global sum (with threads and race condition): %d\n", global_sum);

    // Clean up
    free(array);

    return 0;
}
