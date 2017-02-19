// Name : Nilanshu Sharma

/* This code demostrates producer-consumer synchronization using  
 * condition variables. There are three producers - White, Red, 
 * Black each of which produce 1000 items. The items are simple  
 * strings which contain the producer name - While, Red or Black 
 * and the unix timestamp at which was generated. The consumer 
 * hence, reads a total of 3000 items. Since the producers and   
 * consumers share variable "count", it is protected inside the 
 * critical section. The mutual exclusion and synchronization 
 * happens via a shared mutex and two condition variables - 
 * items and spaces. The producers waits on spaces and consumer
 * waits on items. Also since the mutex and condition variables 
 * are globals they are shared by all the threads. 
 */ 

// Header files 
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/types.h>


#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<pthread.h>
#include<string.h>
#include<sys/time.h>

// Globals

const int N = 2;
char Buffer[2][50];
int in = 0;
int out = 0;
int count = 0;

pthread_mutex_t lock; 
pthread_cond_t items, spaces; /* Producers wait on spaces, Consumer waits on items */

void* red_prod_routine(void* arg) { 
    int fp = open("Producer_RED.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666); 
    int i;
    char buf[50]; 
    struct timeval curr_time;
    unsigned long int curr_usec; 
 
    for (i=0; i<1000; i++) {

        pthread_mutex_lock (&lock);  /* Enter critical section  */  

        while (count==N)          /* Make sure that buffer is NOT full   */
            while (pthread_cond_wait(&spaces, &lock) != 0) ; /* Sleep using a cond variable */
        /*  printf( "Producer %d \n", i);  */
        gettimeofday(&curr_time, NULL);  
        curr_usec = curr_time.tv_sec*1000000 + curr_time.tv_usec; 
        sprintf(buf, "RED   %lu\n", curr_usec); 
        write(fp, buf, strlen(buf)); 
        /* count must be less than N    */
        strcpy(Buffer[in],buf);             /* Put item in the buffer using "in"  */
        in = (in + 1) % N;
        count++;                    /* Increment the count of items in the buffer */
        //printf("Deposited item = %d\n", i);
        pthread_mutex_unlock (&lock);  
        pthread_cond_signal(&items); /* Wakeup consumer, if waiting */        	
    }
    close(fp); 
    return NULL; 
}

void* black_prod_routine(void* arg) {
    int fp = open("Producer_BLACK.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
    int i;
    char buf[50];
    struct timeval curr_time;
    unsigned long int curr_usec;

    for (i=0; i<1000; i++) {

        pthread_mutex_lock (&lock);  /* Enter critical section  */

        while (count==N)          /* Make sure that buffer is NOT full   */
            while (pthread_cond_wait(&spaces, &lock) != 0) ; /* Sleep using a cond variable */
        /*  printf( "Producer %d \n", i);  */
        gettimeofday(&curr_time, NULL);
        curr_usec = curr_time.tv_sec*1000000 + curr_time.tv_usec;
        sprintf(buf, "BLACK %lu\n", curr_usec);
        write(fp, buf, strlen(buf));
        /* count must be less than N    */
        strcpy(Buffer[in],buf);             /* Put item in the buffer using "in"  */
        in = (in + 1) % N;
        count++;                    /* Increment the count of items in the buffer */
        //printf("Deposited item = %d\n", i);
        pthread_mutex_unlock (&lock);
        pthread_cond_signal(&items); /* Wakeup consumer, if waiting */
    }
    close(fp);
    return NULL;
}

void* white_prod_routine(void* arg) {
    int fp = open("Producer_WHITE.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
    int i;
    char buf[50];
    struct timeval curr_time;
    unsigned long int curr_usec;

    for (i=0; i<1000; i++) {

        pthread_mutex_lock (&lock);  /* Enter critical section  */

        while (count==N)          /* Make sure that buffer is NOT full   */
            while (pthread_cond_wait(&spaces, &lock) != 0) ; /* Sleep using a cond variable */
        /*  printf( "Producer %d \n", i);  */
        gettimeofday(&curr_time, NULL);
        curr_usec = curr_time.tv_sec*1000000 + curr_time.tv_usec;
        sprintf(buf, "WHITE %lu\n", curr_usec);
        write(fp, buf, strlen(buf));
        /* count must be less than N    */
        strcpy(Buffer[in],buf);             /* Put item in the buffer using "in"  */
        in = (in + 1) % N;
        count++;                    /* Increment the count of items in the buffer */
        //printf("Deposited item = %d\n", i);
        pthread_mutex_unlock (&lock);
        pthread_cond_signal(&items); /* Wakeup consumer, if waiting */
    }
    close(fp);
    return NULL;
}

void* cons_routine(void* arg) {
    int i;
    int fp = open("Consumer.txt", O_CREAT | O_APPEND | O_RDWR | O_TRUNC, 0666);
    char buf[50]; 
    
    for(i=0; i<3000;i++) {
        pthread_mutex_lock (&lock);    /* Enter critical section  */
        while(count == 0)     /* Make sure that buffer is NOT empty */
            while (pthread_cond_wait(&items, &lock) != 0) ; /* Sleep using a cond variable *

        //  count must be > 0    */
        if (count > 0) {
            strcpy(buf, Buffer[out]);  /* Remove item form the buffer using "out"  */
            write(fp, buf, strlen(buf));
            out = (out + 1)%N; 
            count--;           /* Decrement the count of items in the buffer */
            //printf( "Removed %d \n", i);
        }
        pthread_mutex_unlock (&lock);   /* exit critical seciton  */
        pthread_cond_signal(&spaces);  /* Wakeup prodcuer, if waiting */
    }
    close(fp);
    return NULL;
}


int main(int argc, char* argv[]) {

    pthread_t prod_red, prod_black, prod_white, cons;   /* Thread Variables */
    
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&items, NULL);
    pthread_cond_init(&spaces, NULL); 

    // Create red producer
    int ret = pthread_create(&prod_red, NULL, red_prod_routine, NULL); 
    if(ret != 0){
        perror("\nRed producer thread creation failed");  
        exit(1); 
    }

    // Create black producer
    ret = pthread_create(&prod_black, NULL, black_prod_routine, NULL);
    if(ret != 0){
        perror("\nBlack producer thread creation failed");
        exit(2);
    }
    
    // Create white producer
    ret = pthread_create(&prod_white, NULL, white_prod_routine, NULL);
    if(ret != 0){
        perror("\nWhite producer thread creation failed");
        exit(3);
    }

    // Create consumer
    ret = pthread_create(&cons, NULL, cons_routine, "White");
    if(ret != 0){
        perror("\nConsumer thread creation failed");
        exit(4);
    }
    
    // Wait on the consumer thread
    if (ret = pthread_join(cons, NULL)) { 
       perror("\npthread_join failed");
       exit(5);
    }
   
    printf("\nProgram Terminated Successfully\n\n");  
    return 0; 
}
