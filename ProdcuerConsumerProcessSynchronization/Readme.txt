/* This code demostrates producer-consumer synchronization using  
 * condition variables. There are three producers processes- White, 
 * Red, Black each of which produce 1000 items. The items are simple  
 * strings which contain the producer name - While, Red or Black 
 * and the unix timestamp at which was generated. The consumer 
 * hence, reads a total of 3000 items. Since the producers and   
 * consumers have different address spaces, they share variable 
 * "count" by creating a shared memory segment. The "count" variable 
 * is protected inside the critical section. The mutual exclusion and
 * synchronization happens via a shared mutex and two condition variables - 
 * items and spaces. The producers waits on spaces and consumer
 * waits on items. The mutex and condition variables are in shared memory
 * segment which is accessed by all the producer and consumer processes. 
 */ 

1) To Compile, execute "make all". This will create an executable called "main". 

2) Execute main using "./main". This will generate .txt files. 

3) To clean up, execute "make clean". This will delete the executable "main" and all the .txt files.  
