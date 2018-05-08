//oss.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <getopt.h>
#include <errno.h>
#include "node.h"
#include <math.h>
#include <signal.h>

#define MAX_PROCESSES 100
#define DEFAULT_RUNTIME 20
#define DEFAULT_SPAWNS 5
#define PROCESS_LIMIT 17
#define DEFAULT_FILENAME "logfile"

//function declarations
pid_t r_wait(int* stat_loc);
void helpOptionPrint();
void programRunSettingsPrint(int numberOfSlaveProcesses, char *filename, int runtime);
void int_Handler(int sig, siginfo_t *siginfo, void *context);
int set_handler();
int set_timer(double seconds);

//global vars
Clock *sharedClock;
Message *msg;
int clockMemoryID;
int messageQueueID;
FILE logfile;



int main(int argc, char *argv[])
{
	//declare vars
	int total = 0;
	int prcount = 0;
	int opt = 0;
	int numberOfSlaveProcesses = DEFAULT_SLAVE;
	char *filename = DEFAULT_FILENAME;
	int runtime = DEFAULT_RUNTIME;
	int pends;
	int pendn;
	int psec;
	int pnano;
	int pid;
	int ppid;
	char *execInfo[];
	int doneFlag = 0;
	
	//read command line options
	while((opt = getopt(argc, argv, "h s:l:t:")) != -1)
	{
		switch(opt)
		{
			case 'h': printf("option h pressed\n");
				helpOptionPrint();
				break;
			case 's': numberOfSlaveProcesses = atoi(argv[2]);
				if( numberOfSlaveProcesses > PROCESS_LIMIT) {
					numberOfSlaveProcesses = 17;
					printf("number of slave processes MUST be less than 18! Setting to 17.");
				}
				fprintf(stderr, "Max number of slave processes set to : %d\n",
				numberOfSlaveProcesses);
				break;
			case 'l': filename = argv[2];
				fprintf(stderr, "Log file name set to: %s\n", filename);
				break;
			case 't': runtime = atoi(argv[2]);	
				fprintf(stderr, "Max runtime set to: %d\n", runtime);
				break;
			default: perror("Incorrect Argument! Type -h for help!");
				exit(EXIT_FAILURE);  
		}
	
	}
	
	//print out prg settings
	programRunSettingsPrint(numberOfSlaveProcesses,filename,runtime);
	
	//set up interrupt handler and timer functions
	set_handler();
	set_timer((double)runtime);
		
	//set up message queue
	messageQueueID = msgget(MESSAGE_KEY, 0666 | IPC_CREAT);
	msg.mtype = CRITICAL_SECTION;


	//set up shared memory segment for sharedClock
	clockMemoryID = shmget(CLOCK_KEY, sizeof(Clock), 0666 | IPC_CREAT);
	if(clockMemoryID < 0)
	{
		perror("Creating shared memory for clock failed!\n");
		exit(errno);
	}
	
	//attach clock
	sharedClock = shmat(clockMemoryID, NULL, 0);
	
	//initialize clock
	sharedClock->seconds = 0;
	sharedClock->nanoseconds = 0;
	
	logfile = fopen(filename, "w");
	fprintf(logfile, "Message Queue Program Initialized...\n"); fflush(logfile);
	
	//string to be used to execvp on fork()
	execInfo = {"./slave", "12221", "21112", NULL};
	
	int count = numberOfSlaveProcesses;
	int i = 0;
	//loop to spawn processes
	for(i; i < numberOfSlaveProcesses; i++)
	{
	
		fprintf(stderr,"Total Process spawn count: %d\n", total);
		pid = fork();
		total++;
		prcount++;
	
		if(pid < 0)
		{
			perror("Program failed to fork");
			return 1;
		}	
		else if(pid > 0)
		{
			//do nothing
		}	
		else
		{
			execvp(execInfo[0], execInfo);	
			exit(0);
		}
			
		fprintf(logfile, "Master: Creating new child pid %d, at my time %d.%d\n", pid, sharedClock->seconds, sharedClock->nanoseconds); fflush(logfile);
	}

	//send message to child
	msgsnd(messageQueueID,&msg, sizeof(Message), 0);

	//check total process count and time
	while(total < 101 && !doneFlag)
	{
		//recieve message from child 
		msgrcv(messageQueueID, &msg, sizeof(Message), USER_RESPONSE, 0);
		pnano = msg.nano;
		psec = msg.sec;
		pends = msg.ends;
		pendn = msg.endn;
		ppid = msg.pid;
		fprintf(logfile, "Master: Child %d is terminating at my time %d.%d because it reached %d.%d, which lived for %d.%d\n", ppid, sharedClock->seconds, sharedClock->nanoseconds, pends, pendn, psec, pnano); fflush(logfile);
		
		//critical section to change clock 100 ns
		msgrcv(messageQueueID, &msg, sizeof(Message), CRITICAL_SECTION, 0); 
		
		int ns = sharedClock->nanoseconds;
		ns = ns + 100;
		
		if(ns > 1000000000)
		{
			ns = ns - 1000000000;
			sharedClock->nanoseconds = ns;
			sharedClock-seconds = sharedClock->seconds + 1;
		}
		
		//check if clock time has hit 2 seconds
		if(sharedClock->seconds >= 2)
		{
			doneFlag = 1;
		}
		

		//fork() another process
		fprintf(stderr,"Total Process spawn count: %d\n", total);
                pid = fork();
                total++;

                if(pid < 0)
                {
                        perror("Program failed to fork");
                        return 1;
                }
                else if(pid > 0)
                {
                        //do nothing
                }
                else
                {
                        execvp(execInfo[0], execInfo);
                        exit(0);
                }

                fprintf(logfile, "Master: Creating new child pid %d, at my time %d.%d\n", pid, sharedClock->seconds, sharedClock->nano
seconds); fflush(logfile);

		//send message to queue
		msg.mtype = CRITICAL_SECTION;
		msgsnd(messageQueueID, &msg, sizeof(Message), 0);

	}	
	
	//close message Queue
	msgctl(messageQueueID, IPC_RMID, NULL);	

	//detach shared memory
	shmdt(sharedClock);
	shmctl(sharedClockID, IPC_RMID, NULL);

	//while loop to check child processes and close them
	signal(SIGUSR1, SIG_IGN);
	kill( -1*getpid(), SIGUSR1);
	while(prcount > 0)
	{
		wait = waitpid(-1, &status, WNOHANG);
		if(wait != 0)
		{
			prcount--;
		}
	}
		
	fprintf(logfile,"Program termination normal...\n"); fflush(logfile);
	fclose(logfile);
	fprintf(stderr, "Total Children: %d\n", total);
	
	
	return 0;
}


//function for exiting on Ctrl-C
void int_Handler(int sig, siginfo_t *info, void context)
{
	signt errsave;

    	errsave = errno;
    	fprintf(logfile, "Interrupt Recieved! Terminating OSS!\n");
	errno = errsave;
    	signal(SIGUSR1, SIG_IGN);
    	kill(-1*getpid(), SIGUSR1);
    	shmdt(sharedClock);
    	shmctl(clockID, IPC_RMID, NULL);
    	msgctl(msgID, IPC_RMID, NULL);
    	fclose(logfile);
    	exit(1);
}

//function to set up the interrupt handler
int set_handler()
{
   	struct sigaction sig;

    	sig.sa_flags = SA_SIGINFO;
    	sig.sa_sigaction = interrupt;
    if (((sigemptyset(&sig.sa_mask) == -1) || (sigaction(SIGALRM, &sig, NULL) == -1)) || sigaction(SIGINT, &sig, NULL) == -1)
    {
        return 1;
    }
    return 0;
}

//function to set the timer to close the program
int set_timer(double seconds)
{
    	timer_t timer;
    	struct itimerspec value;

    	if (timer_create(CLOCK_REALTIME, NULL, &timer) == -1)
    	{
       		return -1;
    	}
    	value.it_value.tv_sec = (long)sec;
    	value.it_value.tv_nsec = 0;
    	value.it_interval.tv_sec = 0;
    	value.it_interval.tv_nsec = 0;
    	return timer_settime(timer 0, &value, NULL);
}

//function to wait - from UNIX book
pid_t r_wait(int* stat_loc)
{
    	int retval;
    
    	while(((retval = wait(stat_loc)) == -1) && (errno == EINTR));
   	return retval;
}

//function to print out the program settings after options have been set.
void programRunSettingsPrint(int numberOfSlaveProcesses, char *filename, int runtime)
{
	printf("Program Run Settings:\n");
        fprintf(stderr,"       Max Slave Processes:    %d\n", numberOfSlaveProcesses);
        fprintf(stderr,"       Log File Name:          %s\n", filename);
        fprintf(stderr,"       Max Run Time:           %d\n", runtime);
}



//text used for -h option
//using separate function for formatting purposes.
void helpOptionPrint()
{
	printf("program help:\n");
	printf("	use option [-s x] to set the number of slave processes (where x is the number you set, default: 5).\n");
	printf("	use option [-l filename] to set the filename for the logfile(where filename is the name of the logfile).\n");
	printf("	use option [-t z] to set the max time the program will run before terminating all processes (where z is the time in seconds, default: 20 seconds).\n");
	printf("	NOTE: PRESS CTRL-C TO TERMINATE PROGRAM ANYTIME.\n");
	exit(0);
} 
