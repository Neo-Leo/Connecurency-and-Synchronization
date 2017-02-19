// Name : Nilanshu Sharma

/*
 * This code creates shared memory segments between   
 * two processes. This segment is used a common buffer for     
 * producer and consumer. There is one producer and one
 * consumer. Since both the processes share the buffer, the
 * producer produces (writes) the item on buffer, sends signal
 * to consumer and then waits till consumer reads the item from
 * the shared buffer. The consumer (intially waiting), wakes up 
 * upon the receiving the signal, consumes the item and sends a
 * signal to the producer to produce another item, while it 
 * waits again. This process goes on till all the items have
 * been produced and consumed.    
 */

/* POSIX header files */

#include <fcntl.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>

/* ISO/ANSI header files */

#include<stdlib.h>
#include<stdio.h>
#include<errno.h>
#include<signal.h>
#include<unistd.h>

/* Constants */

#define SHM_BUF_KEY 1498 
#define SHM_COUNT_KEY 5037

/* Signal Handler for SIGUSR1 and SIGUSR2 */

void handler(int signo){
    // Do Nothing, this is so that process is not killed 
}

int main(int argc, char* argv[]){
     
    int shmBufID, shmCountID, count=0;

    // Make a shared memory region of size 1024 bytes which 
    // will contain the data
    shmBufID  = shmget(SHM_BUF_KEY, 1024, 0666 | IPC_CREAT); 
    if(shmBufID < 0) {
        perror("\nshmget Buffer Failed!"); 
        exit(1);
    } 
   
    // Make a shared memory region of size 4 bytes which 
    // will contain size of data written shared memory region.
    // These 4 bytes will act as the count variable.  
    shmCountID = shmget(SHM_COUNT_KEY, 4, 0666 | IPC_CREAT);
    if(shmCountID < 0) {
        perror("\nshmget Count Failed!");
        exit(1); 	
    }  

    pid_t pid;
    pid = fork();

    if(pid > 0) {

        // This is the parent/producer process 

        signal(SIGUSR2, handler);
        char *shmBufSeg = shmat(shmBufID, NULL, 0666);
        if(shmBufSeg == (void*)-1) {
            perror("\nProducer : shmat buffer failed"); 
            exit(1);
        }

        int *shmCountSeg = shmat(shmCountID, NULL, 0666);
        if(shmCountSeg == (void*)-1) {
            perror("\nProducer : shmat count failed");
            exit(1);
        }
        
        count = shmCountSeg[0] = 0;

        int ifp = open(argv[1], O_RDONLY);
        
        if(ifp == -1) {
            perror("\nError in opening the input file"); 
            exit(1);
        }
       
       // The parent process should wait till 
       // SIGUSR2 is received from consumer.  
       sigset_t set; 
       sigemptyset(&set);  
       sigaddset(&set, SIGUSR2);

       while((count = read(ifp, shmBufSeg, 1024)) != 0){
            if(count == -1) {
	        if(errno == EINTR)
                    continue; 
                perror("\nread");
                break;  	
            }
            
            shmCountSeg[0] = count; 
            // Send a signal to consumer   
            kill(pid, SIGUSR1);
            // And wait
            if(shmCountSeg[0] > 0) { 
                int sig; 
                printf("\nIn Producer, %d bytes written to shared memory\n", count);
                sigwait(&set, &sig);  
            } 
        }

        // Done reading the whole file  
        // Notify the consumer about this event
        shmCountSeg[0] = -1;        
        kill(pid, SIGUSR1);
        
        // Wait for the child to terminate
        int retVal = wait(NULL); 
        if( retVal == -1) {
            perror("\nwait()\n"); 
        } else {
            if (retVal == pid) {
                printf("\nChild Terminated\n"); 
            }
        }
        // Detach the shared memory regions from 
        // producer's address space 
        shmdt((void*)shmBufSeg); 
        shmdt((void*)shmCountSeg);
        // Close the input file 
        close(ifp);
        printf("\nParent Terminating\n\n");                    
    } else if(pid == 0) {
        // This is the child/consumer process

        // Register the handler for SIGUSR1 
        signal(SIGUSR1, handler); 

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
       
        // Add SIGUSR1 to set
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);

        // Open output file for writing
        int  ofp = open("output.txt", O_WRONLY | O_CREAT | O_APPEND); 
        while(shmCountSeg[0] != -1) {
            
            // If no data is written yet, then wait 
            if(shmCountSeg[0] == 0){
                int sig;
                sigwait(&set, &sig); 
            }
            // Data available for reading, process it  
            if(shmCountSeg[0] > 0){
                printf("\nIn consumer, %d bytes read from shared memory\n", shmCountSeg[0]);
                write(ofp, shmBufSeg, shmCountSeg[0]);
                shmCountSeg[0]= 0; 
                kill(getppid(), SIGUSR2);      
            }
        }
        // Detach the shared memory region in consumer process 
        shmdt((void*)shmBufSeg);
        shmdt((void*)shmCountSeg);
        // Closing output file 
        close(ofp); 
    } else {
        perror("Fork Failed!"); 
        exit(1); 
    } 
    return 0; 
}
