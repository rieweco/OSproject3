//slave.c file
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "node.h"
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

//global vars
Clock *sharedClock;
Message *msg;
int clockMemoryID;
int messageQueueID;


//functions
void int_Handler(int sig, siginfo_t *info, void *context);

int main(int argc, char *argv[])
{

	//initialize interrupt handler
	signal(SIGUSR1, int_Handler);
	
	//declare vars
	int end_s;
	int end_n;
	int total_s = 0;
	int total_n = 0;
	int runtime;
	int endtime;
	int burst;
	//roll runtime between 1 and 1000000
	srand(time(0));
	endtime = (rand() % 1000000) + 1;
	
	//set up msg queue
	messageQueueID = msgget(MESSAGE_KEY, 0666);

	//set up shared memory for clock
	clockMemoryID = shmget(CLOCK_KEY, sizeof(Clock), 0666);

	if(clockMemoryID < 0)
	{
		perror("inside Slave: Creating Clock shared memory Failed!!\n");
		exit(1);
	}

	//attach clock
	sharedClock = shmat(clockMemoryID, NULL, 0);
	
	//initialize runtime to 0
	runtime = 0;
	
	while(runtime < endtime)
	{
		//check for permission to run
		msgrcv(messageQueueID, &msg, sizeof(Message), CRITICAL_SECTION, 0);
		
		//roll for burst time
		burst = (rand() % 500000) + 1000;

		if((runtime + burst) > endtime) 
		{
			runtime = endtime;
		}
		else 
		{
			runtime = runtime + burst;
			total_n + total_n + burst;	
		}

		int ns = sharedClock->nanoseconds;
                ns = ns + burst;

                if(ns > 1000000000)
                {
                        ns = ns - 1000000000;
                        sharedClock->nanoseconds = ns;
                        sharedClock->seconds = sharedClock->seconds + 1;
                }	

		end_s = sharedClock->seconds;
		end_n = sharedClock->nanoseconds;
		
		//printf("ends: %d, endn: %d\n", end_s, end_n);

		//send message back to master for completing one burst
		msg.mtype = CRITICAL_SECTION;
		msgsnd(messageQueueID, &msg, sizeof(Message), 0);

		//end one burst round
		
	}

	
	//once program has reached endtime, send message with runtime data and terminate
	msg.mtype = USER_RESPONSE;
	msg.pid = getpid();
	msg.sec = 0;
	msg.nano = runtime;
	msg.ends = end_s;
	msg.endn = end_n;
	

	msgsnd(messageQueueID, &msg, sizeof(Message), 0);

	//all done sending, detach shared memory
	shmdt(sharedClock);
	
	return 0;
}



//function to recieve interrupts
void int_Handler(int sig, siginfo_t *info, void *context)
{
	//detach clock
	shmdt(sharedClock);

	//print and exit()
	printf("Slave: Recieved Interrupt, Terminating!\n");
	
	exit(1);
}

