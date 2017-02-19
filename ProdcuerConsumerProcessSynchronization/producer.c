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


void prod_routine(char* prod_name) {

    //printf("\nI'm producer : %s", prod_name); 
    int fp; 
    char name[10], filename[25]; 

    // Copy the producer name in a temporary buffer
    strcpy(name,prod_name);

    // Create the file name
    strcpy(filename,"Producer_"); 
    strcat(filename,prod_name); 
    strcat(filename,".txt"); 
 
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
    
    fp = open(filename, O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
    int i;
    char buf[50];
    char tsbuf[30]; 
    struct timeval curr_time;
    unsigned long int curr_usec;

    for (i=0; i<1000; i++) {

        pthread_mutex_lock(shmMutexSeg);  /* Enter critical section  */

        while (shmCountSeg[0]==N)   /* Make sure that buffer is NOT full   */
            while (pthread_cond_wait(shmCondSpacesSeg, shmMutexSeg) != 0) ; /* Sleep using a cond variable */
        /*  printf( "Producer %d \n", i);  */
        gettimeofday(&curr_time, NULL);
        curr_usec = curr_time.tv_sec*1000000 + curr_time.tv_usec;
        strcpy(buf, name);
        if(strcmp(name, "RED") == 0){
            strcat(buf, "    "); 
        } else {
            strcat(buf, "  "); 
        }
        sprintf(tsbuf, "%lu\n", curr_usec);
        strcat(buf,tsbuf);
        write(fp, buf, strlen(buf));
        /* count must be less than N */
        strcpy(&shmBufSeg[shmCountSeg[1]*50],buf);    /* Put item in the buffer using "in"  */
        shmCountSeg[1] = (shmCountSeg[1] + 1) % N;
        shmCountSeg[0]++;                    /* Increment the count of items in the buffer */
        //printf("Deposited item = %d\n", i);
        pthread_mutex_unlock(shmMutexSeg);
        pthread_cond_signal(shmCondItemsSeg); /* Wakeup consumer, if waiting */
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
