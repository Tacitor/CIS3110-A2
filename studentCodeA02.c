#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

//Datastructure definitions

typedef struct process //represents a single process
{
	char pid[4];
	unsigned int startTime;
	unsigned int lifeTime;
//add more members here as per requirement
} Process;


//Global variables
Process* processes = NULL; //the array to store processes read from the file
int processCount = 0; //count of total processes; update it after reading file
unsigned int timeQuantum;//This is the timeQuantum; initialize it based on the command line argument
time_t programClock;//the global timer/clock for the program


//Function prototypes
void logStart(char* pID);//function to log that a new process is started
void logRun(char* pID);//function to log that a process is entered into running state
void logEndQuantum(char* pID);//function to log that a process has finished its time quantum and going back to ready queue
void logFinish(char* pID);//function to log that a process is finished
void startClock();//function to start program clock
long getCurrentTime();//function to check current time since clock was started
int totalTime();//the function that computes total simulation time based on process list

int scheduler();//the scheduler function; you have to implement this
int readFile(char* fileName);//function to read the file content and build array of processes; you have to implement this


int main(int argc, char *argv[])
{

    //you can add some suitable code here if needed. Make sure all your data is set properly before this point
    
	startClock();//do not remove this line
	time_t currentTime = -1;//do not remove this line
	
	while(getCurrentTime()<=totalTime())//this loop iterates for the total life of all processes, do not remove this line
	//do not add or change anything in this while loop
	//all you logic must be in the scheduler	
	{
		if(getCurrentTime()==currentTime+1)//this condition simulates the clock ticks and calls scheduler whenever local clock time progresses by 1
		{
			currentTime++;
			scheduler();
		}
	}

	return 0;
}

int readFile(char* fileName)//use this method in a suitable way to read file
{
	return 0;
}

void logStart(char* pID)//invoke this method when you start a process
{
	printf("[%ld] New process with ID %s is arrived.\n", getCurrentTime(), pID);
}

void logFinish(char* pID)//invoke this method when a process is over
{
	printf("[%ld] process with ID %s is finished.\n", getCurrentTime(), pID);
}

void logEndQuantum(char* pID)//invoke this method when a process finishes its time quantum
{
	printf("[%ld] Process with ID %s finished its time quantum.\n", getCurrentTime(), pID);
}

void logRun(char* pID)//invoke this method when a process started its time quantum
{
	printf("[%ld] Process with ID %s started its time quantum.\n", getCurrentTime(), pID);
}

int totalTime()
{
	int largestTime = 0;
	for(int k=0; k<processCount; k++)
	{
		if(processes[k].lifeTime+processes[k].startTime > largestTime+processes[k].lifeTime)
			largestTime = processes[k].lifeTime+processes[k].startTime;
		else
			largestTime+=processes[k].lifeTime;
	}
	return largestTime;
}

int scheduler()//implement this function as per the given description
{
	return 0;
}

void startClock()//do not change any code in this fucntions
{
	programClock = time(NULL);
}

long getCurrentTime()//invoke this method whenever you want to check how much time units passed
//since you invoked startClock()
{
	time_t now;
	now = time(NULL);
	return now-programClock;
}
