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
#define N 2 

void cons_routine() {

    //printf("\nI'm  consumer");
    int fp;
    pthread_mutex_t mutex;
    pthread_cond_t items, spaces;

    // Get the shared memory segments we created earlier
    int id = shmget(SHM_BUF_KEY, 0, 0);
    char *shmBufSeg = shmat(id, NULL, 0666);
    if(shmBufSeg == (void*)-1) {
        perror("\nConsumer : shmat buf failed");
        exit(0);
    }

    id = shmget(SHM_COUNT_KEY, 0, 0);
    int *shmCountSeg = shmat(id, NULL, 0666);
    if(shmCountSeg == (void*)-1) {
        perror("\nConsumer : shmat count failed");
        exit(0);
    }

    id = shmget(SHM_MUTEX_KEY, 0, 0);
    pthread_mutex_t *shmMutexSeg = shmat(id, NULL, 0666);
    if(shmMutexSeg == (void*)-1) {
        perror("\nConsumer : shmat mutex failed");
        exit(0);
    }
    
     id = shmget(SHM_COND_ITEMS_KEY, 0, 0);
    pthread_cond_t *shmCondItemsSeg = shmat(id, NULL, 0666);
    if(shmCondItemsSeg == (void*)-1) {
        perror("\nConsumer : shmat cond items failed");
        exit(0);
    }

    id = shmget(SHM_COND_SPACES_KEY, 0, 0);
    pthread_cond_t *shmCondSpacesSeg = shmat(id, NULL, 0666);
    if(shmCondSpacesSeg == (void*)-1) {
        perror("\nConsumer : shmat cond spaces failed");
        exit(0);
    }

    fp = open("Consumer.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
    int i;
    char buf[50];

    for (i=0; i<3000; i++) {

        pthread_mutex_lock(shmMutexSeg);  /* Enter critical section  */
        while (shmCountSeg[0]==0)          /* Make sure that buffer is NOT empty   */
            while (pthread_cond_wait(shmCondItemsSeg, shmMutexSeg) != 0) ; /* Sleep using a cond variable */
        
        if (shmCountSeg[0] > 0) {
            strcpy(buf, &shmBufSeg[shmCountSeg[2]*50]);  /* Remove item form the buffer using "out" */
            write(fp, buf, strlen(buf));
            shmCountSeg[2] = (shmCountSeg[2] + 1)%N; /* out = (out + 1)%N */ 
            shmCountSeg[0]--;           /* Decrement the count of items in the buffer */
            //printf( "Removed %d \n", i);
        }
        pthread_mutex_unlock (shmMutexSeg);   /* exit critical seciton  */
        pthread_cond_signal(shmCondSpacesSeg);  /* Wakeup prodcuer, if waiting */
    }
    
    // Detach the shared memory regions  
    shmdt((void*)shmBufSeg);
    shmdt((void*)shmCountSeg);
    shmdt((void*)shmMutexSeg);
    shmdt((void*)shmCondItemsSeg);
    shmdt((void*)shmCondSpacesSeg);

    // Closing output file 
    close(fp);
}

