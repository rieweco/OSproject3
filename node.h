//node.h
//contains structures for children and clock being used in shared memory
#ifndef NODE_H
#define NODE_H

#define CLOCK_KEY 12221
#define MESSAGE_KEY 21112
#define CRITICAL_SECTION 6
#define USER_RESPONSE 9


//child struct that holds the child number and its pid
typedef struct ChildProcess
{
	int childNumber;
	pid_t childID;
}
ChildProcess;


//clock struct to track seconds and nanoseconds
typedef struct Clock
{
	int seconds;
	int nanoseconds;
}
Clock;

//child message struct that contains the message,
//pid, time of process in ss:nn, and a flag for
//signalling being in/out of the critical section 
typedef struct Message
{
	long mtype;
	char content[100];
}
Message;




#endif
