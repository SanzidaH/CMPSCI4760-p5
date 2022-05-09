/*libraries*/
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<getopt.h>
#include<sys/wait.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <limits.h>
#include <sys/sem.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/wait.h>


/* macros */
#define MAXPROC 18
#define MAXRESOURCE 10
#define SHM_KEY 0x12345 //shared memory key
#define LEN 150
#define TO_NANO 1000000000UL
#define timeslice 100000

/* DS */

struct  ResourceDescriptor{
    int requestMatrix[MAXPROC][MAXRESOURCE];
    int allocationMatrix[MAXPROC][MAXRESOURCE];
    int resourceVector[MAXRESOURCE];
    int allocationVector[MAXRESOURCE];
    int releaseVector[MAXRESOURCE];
    pid_t allpid[MAXPROC];
};

struct StatRecord {
    int immediateGrant; 
    int waitedGrant; 
    int deadlockTerminations; 
    int deadlockRuns;
    int terminationSuccess; 
};


/* shm variables */

//System clock
unsigned int *clock_s;
unsigned int *clock_ns;
int clock_sid, clock_nsid, rd_id, sr_id;

struct StatRecord *sr;
struct ResourceDescriptor *rd;

FILE *cstest = NULL, *logfile = NULL, *file = NULL;
int *allpid2 = NULL, *parentpid = NULL, *childpid = NULL;
pid_t childPid;
int availableResource[10], work[10];
bool finishedProc[MAXPROC];
bool doSim = true;

/* functions */   
void cleanAll();
void ReportStatistics();
void increase_clock(int inc);
bool req_lt_avail(const int *req, const int *avail, const int pnum, const int num_res);
bool deadlock(int allocationVector[MAXRESOURCE], const int m, const int n, int requestMatrix[MAXPROC][MAXRESOURCE], int allocated[MAXPROC][MAXRESOURCE]);
void logging(char * string);

/* signalhandlers */
void signal_timer(int signal);
void signal_abort();

/* iterators */
int i, j, c; 




