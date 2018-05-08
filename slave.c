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
	//roll runtime between 1 and 1000000
	srand(time(0));
	endtime = (rand() % 1000000) + 1;

	



















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

