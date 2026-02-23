#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define FILE_DELIMETERS ";\r\n"

//Datastructure definitions

typedef enum p_state {
  NEW = 0,
  READY,
  RUNNING,
  TERMINATED
} p_state;

typedef struct process //represents a single process
{
	char pid[4];
	unsigned int startTime;
	unsigned int lifeTime;
//add more members here as per requirement
	p_state state;
	unsigned int runningTime; // The time this task had had to run. Gets incremented every cycle it is in the running state. Should never be greater than the lifeTime.
	unsigned int q_start; // The time its latest quatum started
} Process;


//Global variables
Process* processes = NULL; //the array to store processes read from the file
int processCount = 0; //count of total processes; update it after reading file
unsigned int timeQuantum;//This is the timeQuantum; initialize it based on the command line argument
time_t programClock;//the global timer/clock for the program

int last_p_started_i = 0;
int last_p_rr_i = 0;


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
	// Guarding for CLI args. Causes early return and program termination.
	if (argc < 3) {
        printf("ERROR: You must provide a timeQuantum AND an input file as a CLI arguments. Program Terminating.\n");
        return 0;
    } else if (argc > 3) {
        printf("ERROR: You must provide ONLY a timeQuantum, and an input file as a CLI arguments. No other flags or arguments are allowed. Program Terminating.\n");
        return 0;
    }

	// Try converting string to int
	int num = atoi(argv[1]);

	// Ensure proper formatting
	if (num < 1) {
        printf("ERROR: You must provide a timeQuantum that is GREATER than ZREO. Program Terminating.\n");
		return 0;
	}

	// Assign to the global var
	timeQuantum = (unsigned int)num;

	readFile(argv[2]);
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

	free(processes);

	return 0;
}

int readFile(char* fileName)//use this method in a suitable way to read file
{
	int returnSuccess;
	long fileCharNum;
	char *taskString;

	FILE *inputFile = fopen(fileName, "r");

	if (inputFile == NULL) {
        printf("ERROR: Could not open file %s. Program Terminating.\n", fileName);
        return 0;
    }

	returnSuccess = fseek(inputFile, 0, SEEK_END);

    // Guarding for moving file pointer to end of file so its size can be found. Causes early return and program termination.
    if (returnSuccess !=0 ) {
        printf("ERROR: Could not seek to end of %s. Program Terminating.\n", fileName);
        return 0;
    }

    fileCharNum = ftell(inputFile);
	if (-1 == fileCharNum) {
        printf("ERROR: Could not tell the length of file %s. Program Terminating.\n", fileName);
        return 0;
    }

    // Guarding for moving file pointer back to the start of file so can now be read into mem. Causes early return and program termination.
    returnSuccess = fseek(inputFile, 0, SEEK_SET);
    
    if (returnSuccess !=0 ) {
        printf("ERROR: Could not seek back to start of %s. Program Terminating.\n", fileName);
        return 0;
    }

    taskString = calloc((fileCharNum + 1), sizeof(char));

	if (NULL == taskString) {
        printf("ERROR: Could not allocate enough memory to fit %ld characters from %s. Program Terminating.\n", fileCharNum, fileName);
        return 0;
    }

    size_t elmtRead = fread(taskString, sizeof(char), fileCharNum, inputFile);
    returnSuccess = fclose(inputFile);

	if (returnSuccess !=0 ) {
        printf("ERROR: Could not close the file %s. Program Terminating.\n", fileName);
        return 0;
    }

    if (((size_t)fileCharNum) != elmtRead) {
        printf("ERROR: Bad read from %s. Size read does not match size of file. Program Terminating.\n", fileName);
        return 0;
    }

	// Split the string on using the '\n' or '\r' or ';' deliminator
	char *tok = strtok(taskString, FILE_DELIMETERS);
	int parseCount = 0;
	int tempProcessCount = 0;

	while (NULL != tok) {
		if (0 == parseCount) {
			// Enlarge array by 1
			processes = realloc(processes, sizeof(Process) * (tempProcessCount + 1));

			// Take the first (and only) three chars of the process number plus the null terminator
			strncpy(processes[tempProcessCount].pid, tok, 4);
			processes[tempProcessCount].state = NEW;
			processes[tempProcessCount].runningTime = 0;
			processes[tempProcessCount].q_start = 0;
			parseCount++;
		} else if (1 == parseCount) {
			// Try converting string to int
			int num = atoi(tok);

			// Do not ensure proper formatting since the error return value of atoi is 0. 0 is a valid value soo.... no check
			
			processes[tempProcessCount].startTime = num;
			parseCount++;
		} else if (2 == parseCount) {
			// Try converting string to int
			int num = atoi(tok);

			processes[tempProcessCount].lifeTime = num;
			parseCount = 0;

			// Ensure proper formatting
			// Jacob said to just not add this task to the list.
			if (num < 1) {
				// printf("ERROR: You must provide lifeTime in correct format. Must be greater than 0. Program Terminating.\n");
				// Discard this process!!
				// Do NOT increment tempProcessCount so that it can be overwritten by the next valid task.

				processes[tempProcessCount].state = TERMINATED; //In the case this is the last task and does not get over written mark is so that it does not get started
			} else {
				tempProcessCount++;
			}
		} else {
			printf("ERROR: You must provide input file in correct format.\n");
		}

		tok = strtok(NULL, FILE_DELIMETERS);
	}

	// Update after reading the file
	processCount = tempProcessCount;

	free(taskString);

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

void advanceRoundRobin() {
	last_p_rr_i = last_p_rr_i == (processCount - 1) ? 0 : last_p_rr_i + 1;
}

int scheduler()//implement this function as per the given description
{
	long int currentTime = getCurrentTime();

	// Check for new arrivals and add them all
	while (last_p_started_i < processCount && NEW == processes[last_p_started_i].state && processes[last_p_started_i].startTime == currentTime) {
		processes[last_p_started_i].state = READY;
		logStart(processes[last_p_started_i].pid);
		last_p_started_i++;
	}

	// Check if the process is ready for termination
	if (RUNNING == processes[last_p_rr_i].state) {
		processes[last_p_rr_i].runningTime++;

		// FIRST check for needs termination
		if (processes[last_p_rr_i].runningTime == processes[last_p_rr_i].lifeTime) {
			processes[last_p_rr_i].state = TERMINATED;
			logFinish(processes[last_p_rr_i].pid);
			advanceRoundRobin();
		}
		
		// How long has the duration of this quatum been?
		int q_duration = currentTime - processes[last_p_rr_i].q_start;
		// SECOND check for done time quatum, force the round robin
		// Ensure the task is STILL in the running state and not since terminated 
		if (0 == (q_duration % (long int)timeQuantum) && RUNNING == processes[last_p_rr_i].state) {
			processes[last_p_rr_i].state = READY;
			logEndQuantum(processes[last_p_rr_i].pid);
			advanceRoundRobin();
		}
	}

	// There is a chance after advanceRoundRobin() was called that the state of the next task is now NEW or TERMINATED.
	// Neither of these need to ever be ran. So advance past them.
	if (RUNNING != processes[last_p_rr_i].state && READY != processes[last_p_rr_i].state){
		// Go around the robin once to see if any other tasks are in the READY state.
		// Only go once to prevent an infinite loop.
		int robinStart = last_p_rr_i;

		do
		{
			advanceRoundRobin();
		} while (last_p_rr_i != robinStart && READY != processes[last_p_rr_i].state);
		// TODO: What should the scheduler do if idle? Skip ^^^ to next task? Wait? Ask TA?
		// Yes Jacob said this is correct.
	}
	
	if (READY == processes[last_p_rr_i].state) {
		processes[last_p_rr_i].state = RUNNING;
		processes[last_p_rr_i].q_start = currentTime;
		logRun(processes[last_p_rr_i].pid);
	}
	
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
