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

#define MAX_LINE 100
#define DEFAULT_SLAVE 5
#define DEFAULT_RUNTIME 20
#define DEFAULT_FILENAME "logfile"

pid_t r_wait(int* stat_loc);
void helpOptionPrint();
void programRunSettingsPrint(int numberOfSlaveProcesses, char *filename, int runtime);

int main(int argc, char *argv[])
{
	int opt = 0;
	int numberOfSlaveProcesses = DEFAULT_SLAVE;
	char *filename = DEFAULT_FILENAME;
	int runtime = DEFAULT_RUNTIME;

	//read command line options
	while((opt = getopt(argc, argv, "h s:l:t:")) != -1)
	{
		switch(opt)
		{
			case 'h': printf("option h pressed\n");
				helpOptionPrint();
				break;
			case 's': numberOfSlaveProcesses = atoi(argv[2]);
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
	
	//set up memory for children 
	ChildProcess *pid;
	pid = (ChildProcess *)malloc(sizeof(ChildProcess) * numberOfSlaveProcesses);
	
	int i = 0;
	pid_t master;
	pid_t child;

	if((master = fork()) < 0)
	{
		perror("Master failed to fork!\n");
		exit(EXIT_FAILURE);
	}
	else if(master == 0)
	{
		printf("Master %d \n\n", getpid());
	}

	for(i = 0; i < numberOfSlaveProcesses; i++)
	{
		if((child = fork()) < 0)
		{
			perror("Child failed to fork!\n");
			exit(EXIT_FAILURE);
		}
		else
		{
			printf("Child %d  Master is %d \n\n", getpid(), getppid());
		}
	}

	
	//wait until processes are finished
	//while(r_wait(NULL) < 0);
	

	return 0;
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
