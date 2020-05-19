/*
 * 
 *Producer/Consumer Problem Simulation.
 * 
 *Linux Programming
 * 
 *Data Structure used for synchronization :
 **Semaphores.
 * 
 *Printing Mode
 **Console.
 * 
 *BY
 **MARRAKCHI Ghassen
 * 
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/sem.h>

//_ PreProcessor Constants.
#define BUFFER_SIZE     10
#define OBJECT          1
#define ERROR_PRODUCER -1	//_ERROR Values.
#define ERROR_CONSUMER -2
#define EXIT_MANAGER    0	//_EXIT Values.
#define EXIT_PRODUCER   1
#define EXIT_CONSUMER   2



/*_ PRIMITIVES DEFINITIONS _*/
void P(int semid){	//_ Resource REQUESTING Primitive.
	struct sembuf op;
	op.sem_num=0;
	op.sem_op=-1;
	op.sem_flg=0;
	semop(semid, &op, 1);
	
}

void V(int semid){	//_ Resource FREEING Primitive.
	struct sembuf op;
	op.sem_num=0;
	op.sem_op=1;
	op.sem_flg=0;
	semop(semid, &op, 1);
	
}

void init(int semid, int E){	//_ Semaphore INITIALIZATION Primitive.
	semctl(semid, 0, SETVAL, E);
	
}



/*_ ACTORS SUBMODULS DEFINITIONS _*/
void Place_Objects(int sem_emptyID, int sem_fullID, int nbreObjects){
	for(int index=0;index<nbreObjects;index++){	//_ Multiple Resource Request.
		P(sem_emptyID);		//_ Unavailability Update. (Borderline Case:  0 initially placed Objects).
		V(sem_fullID);		//_ Availibility Update (Borderline Case : 10 initially placed Objects).
	}
	
}

void Real_Time_Buffer_Status(int sem_Buffer_ID){		//_ Buffer State.
	/*_ STATE VARIABLE DEFINITION _*/
	int content=0;
	
	/*_ PROCESSING _*/
	content = semctl(sem_Buffer_ID, 0, GETVAL, 1);
	printf("---------------------------- Buffer Status:\tOccupied(s):%2d\tVacant(s):%2d\n", BUFFER_SIZE-content, content);
	
}

int  Produce_Object()										{printf("PROD__Object being Produced...\n"); return OBJECT; }											//_ PRODUCER.
void Deposit_Object(int depositoryObject, int sem_Buffer_ID){printf("PROD__Object being Deposit...\n");  Real_Time_Buffer_Status(sem_Buffer_ID); }

int  Pull_Object   (int sem_Buffer_ID)						{printf("CONSO__Object being Pulled...\n");	 Real_Time_Buffer_Status(sem_Buffer_ID); return OBJECT; }	//_ CONSUMER.
void Consume_Object(int consObjet)							{printf("CONSO__Object being Consumed...\n"); }



/*_ ACTORS MODULS DEFINITIONS _*/
void Producer(int prod_mutexID, int prod_fullID, int prod_emptyID, int prod_speed){
	/*_ OBJECTS TO BE PRODUCED _*/
	int object=0;
	
	/*_ PRODUCER PROCESSING _*/
	while(1){
		object=Produce_Object();	//_ Production.
		
		P(prod_emptyID);
		P(prod_mutexID);
		
		Deposit_Object(object, prod_emptyID);	//_ Depository.
		
		V(prod_mutexID);
		V(prod_fullID);
		sleep(prod_speed);
	}
	
}

void Consumer(int cons_mutexID, int cons_fullID, int cons_emptyID, int cons_speed){
	/*_ OBJECTS TO BE CONSUMED _*/
	int object=0;
	
	/*_ CONSUMER PROCESSING _*/
	while(1){
		P(cons_fullID);
		P(cons_mutexID);
		
		object=Pull_Object(cons_emptyID);	//_ Pull.
		
		V(cons_mutexID);
		V(cons_emptyID);
		
		Consume_Object(object);	//_ Consume.
		sleep(cons_speed);
	}
	
}



/*_ MAIN PROGRAM _*/
int main(void){
	/*_ PROCESSING VARIABLES DEIFNITION _*/
	int consumerSpeed, producerSpeed, Buffer_Objects;
	
	
	
	/*_ FOREWORD PROCESSING _*/
	printf("___________Producer/Consumer Problem Simulation____________\n");
	printf("______Data Entry:\n");
	printf("Enter the Producer Speed (0: Fast/ 10: Slow) : _");
	scanf("%d", &producerSpeed);
	printf("\nEnter the Consumer Speed (0: Fast/ 10: Slow) : _");
	scanf("%d", &consumerSpeed);
	printf("\nEnter the number of initially deposited objects on the buffer (10 maximum) : _");
	scanf("%d", &Buffer_Objects);
	printf("\n");
	
	
	
	/*_ SIMULATION PROCESSING _*/	
	/*_ SEMAPHORES DEFINITION _*/
	int mutexID, fullID, emptyID;
	printf("______Semaphores Definition:\n");
	
	mutexID= semget(IPC_PRIVATE, 1, IPC_CREAT|0666);		//_ Mutual Exclusion.
	init(mutexID, 1);
	printf("___Mutual Exclusion Semaphore created successfully!\n");
	
	fullID= semget(IPC_PRIVATE, 1, IPC_CREAT|0666);		//_ Availability.
	init(fullID, 0);
	printf("___Availability Semaphore created successfully!\n");
	
	emptyID=  semget(IPC_PRIVATE, 1, IPC_CREAT|0666);		//_ Unavailability.
	init(emptyID, BUFFER_SIZE);
	Place_Objects(emptyID, fullID, Buffer_Objects);
	printf("___Unavailability Semaphore created successfully!\n");
	
	
	
	/*_ PRODUCER & CONSUMER ESTABLISHMENT _*/
	printf("\n\n______Process starting & Actors Establishing:\n");
	int producer_pid, consumer_pid;
	
	producer_pid=fork();
	switch(producer_pid){
		case -1 :
			printf("Producer creation failed!\n");
			exit(ERROR_PRODUCER);
			
		case  0 :
			printf("Producer created successfully with PID=%d\n", getpid());
			printf("Producer starts the execution..\n");
			Producer(mutexID, fullID, emptyID, producerSpeed);
			printf("Producer Stopped.\n");
			exit(EXIT_PRODUCER);
		break;
		
		default :
			consumer_pid=fork();
			
			switch(consumer_pid){
				case -1 :
					printf("Consumer creation failed!\n");
					exit(ERROR_CONSUMER);
					
				case  0 :
					printf("Consumer created successfully with PID=%d\n", getpid());
					printf("Consumer starts the execution..\n");
					Consumer(mutexID, fullID, emptyID, consumerSpeed);
					printf("Consumer Stopped.\n");
					exit(EXIT_CONSUMER);
				break;
				
				default :
					wait(NULL);
					wait(NULL);
					printf("Manager Stopped.\n");
					exit(EXIT_MANAGER);
				break;
				
			}
		break;
		
	}
	
}
