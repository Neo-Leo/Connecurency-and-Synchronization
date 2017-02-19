// Name : Nilanshu Sharma
// sharm398
// Assignment 4 - Programming Problem 2 

// Header files 
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<sys/time.h>

#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<pthread.h>
#include<string.h>

#include "producer.h"
#include "consumer.h"

/* Constants */

#define SHM_BUF_KEY 1498 
#define SHM_COUNT_KEY 5454 
#define SHM_MUTEX_KEY 1500
#define SHM_COND_ITEMS_KEY 1501
#define SHM_COND_SPACES_KEY 1502

/* Globals */ 

pthread_mutexattr_t mutexattr; 
pthread_condattr_t condattr;
pthread_mutex_t mutex;  
pthread_cond_t items, spaces; 


int main() {

    int shmBufID, shmCountID, count=0, shmMutexID, shmCondItemsID, shmCondSpacesID;
    pid_t pids[4]; 
     
    // Make a shared memory region of size 100 bytes which 
    // will be used as the buffer. This will hold 2 items.  
    // First 50 bytes will hold first item, and next 50 bytes
    // will hold next item. 
    shmBufID  = shmget(SHM_BUF_KEY, 100, 0666 | IPC_CREAT);
    if(shmBufID < 0) {
        perror("\nshmget Buffer Failed!");
        exit(1);
    }

    // Make a shared memory region of size 12 bytes which 
    // will contain count of items written to shared memory region.
    // The next 4 bytes will act as "in" variable and next 4 bytes 
    // as "out" variable.  
    shmCountID = shmget(SHM_COUNT_KEY, 12, 0666 | IPC_CREAT);
    if(shmCountID < 0) {
        perror("\nshmget Count Failed!");
        exit(2);
    }

    // Make a shared memory region which to store the mutex
    shmMutexID = shmget(SHM_MUTEX_KEY, sizeof(pthread_mutex_t), 0666 | IPC_CREAT);
    if(shmMutexID < 0) {
        perror("\nshmget Mutex Failed!");
        exit(3);
    }
    
    // Make a shared memory region to store the condition 
    // variable for items 
    shmCondItemsID = shmget(SHM_COND_ITEMS_KEY, sizeof(pthread_cond_t), 0666 | IPC_CREAT);
    if(shmMutexID < 0) {
        perror("\nshmget Items Condition Variable Failed!");
        exit(4);
    }

    // Make a shared memory region to store the condition  
    // variable for spaces 
    shmCondSpacesID = shmget(SHM_COND_SPACES_KEY, sizeof(pthread_cond_t), 0666 | IPC_CREAT);
    if(shmMutexID < 0) {
        perror("\nshmget Spaces Condition Variable Failed!");
        exit(5);
    }

    // Now attach those memory addresses to this process        
    int id = shmget(SHM_BUF_KEY, 0, 0);
    char *shmBufSeg = shmat(id, NULL, 0666);
    if(shmBufSeg == (void*)-1) {
        perror("\nMain : shmat buf failed");
        exit(0);
    }

    id = shmget(SHM_COUNT_KEY, 0, 0);
    int *shmCountSeg = shmat(id, NULL, 0666);
    if(shmCountSeg == (void*)-1) {
        perror("\nMain : shmat count failed");
        exit(0);
    }

    id = shmget(SHM_MUTEX_KEY, 0, 0);
    pthread_mutex_t *shmMutexSeg = shmat(id, NULL, 0666);
    if(shmMutexSeg == (void*)-1) {
        perror("\nMain : shmat mutex failed");
        exit(0);
    }

    id = shmget(SHM_COND_ITEMS_KEY, 0, 0);
    pthread_cond_t *shmCondItemsSeg = shmat(id, NULL, 0666);
    if(shmCondItemsSeg == (void*)-1) {
        perror("\nMain : shmat cond items failed");
        exit(0);
    }

    id = shmget(SHM_COND_SPACES_KEY, 0, 0);
    pthread_cond_t *shmCondSpacesSeg = shmat(id, NULL, 0666);
    if(shmCondSpacesSeg == (void*)-1) {
        perror("\nMain : shmat cond spaces failed");
        exit(0);
    }
    
    // Initialize the shared memory regions 
    shmCountSeg[0] = shmCountSeg[1] = shmCountSeg[2] = 0;  
    // Set the mutex attribute to be process shared, and then init the mutex 
    pthread_mutexattr_setpshared(&mutexattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(shmMutexSeg, &mutexattr);     

    // Set the condition variable attribute to be process shared, 
    // and init the condition variables - items and spaces 
    pthread_condattr_setpshared(&condattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(shmCondItemsSeg, &condattr); 
    pthread_cond_init(shmCondSpacesSeg, &condattr);    
   
    // Spawn 4 child processes - 3 Producers and 1 Consumer
    for (int i = 0; i < 4; i++) {
        if ((pids[i] = fork()) < 0) {
            perror("\nfork failed!");
            exit(6);
        } else if (pids[i] == 0) {
            if(i == 0) { 
                prod_routine("RED");
            } else if(i == 1) {
                prod_routine("BLACK");
            } else if(i == 2) {
                prod_routine("WHITE"); 
            } else {
                cons_routine(); 
            }
 	    exit(7);
        }
    }

    int status, i=4;
    pid_t pid;
    
    // Wait for children to terminate  
    while (i-- > 0) {
        pid = wait(&status);
        //printf("\nChild with PID %ld exited with status 0x%x.\n", (long)pid, status);
    }
    
    printf("\n\nProgram terminated Successfully\n\n"); 

    return 0;
}
