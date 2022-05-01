/**
* @Author: Sanzida Hoque
* @Course: CMP_SCI 4760 Operating Systems 
* @Sources: https://www.geeksforgeeks.org/deadlock-detection-algorithm-in-operating-system/
*           
*/

#include "config.h"

int opt, nprocs = MAXPROC, nresources = MAXRESOURCE, terminate_time = 5, errno, pid = 0, linenum = 0, *parents = NULL, *children = NULL;
static bool verbose = false;
char buf[250];

int main(int argc, char *argv[]){
    /* Interrupt signal handling */
    signal(SIGALRM, signal_abort);//Abort for end of termination time 
    signal(SIGINT, signal_abort);// Abort for Ctrl+C 


    printf("oss is processessing...\n");    
    /* Parsing Command Line argument */
    while ((opt = getopt(argc, argv, "t:n:r:h:v")) != -1) {

        switch (opt) {
        case 'h':
            printf("Help:\n");
            printf("Run using ./oss -v where all other parameters are default, number of process 18, number of resource 10, termination time 5.\n");
            printf("Or Run using ./oss -v -t [termination time] -n [number of procrss] -r [number of resource]\n");
        case 't':
            terminate_time = atoi(optarg);
            if(terminate_time < 1){
 		perror("OSS: Termination time is not valid");
            }
        case 'n':
            nprocs = atoi(optarg);
            if(nprocs > MAXPROC){
            	perror("OSS: Warning: Numbers of Processes can not exceed 18");
            	nprocs = 18;
            }
            break;
        case 'r':
            nresources = atoi(optarg);
            if(nresources > MAXRESOURCE){
            	perror("OSS: Warning: Numbers of resources can not exceed 10");
            	nresources = 10;
            }
            break; 
        case 'v':    
            verbose = true;    
        case '?':
            if (opt == 'c')
                perror("OSS: Error: Option requires an argument");
            else
                perror("OSS: Error: Unknown option character");
            return 1;

        }

    }

    parents = malloc(sizeof(int) * nprocs);
    children = malloc(sizeof(int) * nprocs);
    /* Creating a new shared memory segment */ 
    clock_nsid  = shmget(ftok("Makefile", '1'), sizeof(clock_ns), IPC_CREAT | 0666);
    clock_sid = shmget(ftok("Makefile", '2'), sizeof(clock_s), IPC_CREAT | 0666);
    rd_id = shmget(ftok("Makefile", '3'), sizeof(rd), IPC_CREAT | 0666);
    sr_id = shmget(ftok("Makefile", '4'), sizeof(sr), IPC_CREAT | 0666);

    if (clock_nsid  == -1 || clock_sid == -1 || rd_id == -1 || sr_id == -1) {
       perror("oss: Error: Shared memory allocation failed");
       // return 1;
       cleanAll();
       abort();
    }

    clock_ns = (unsigned int *)shmat(clock_nsid , NULL, 0);
    clock_s = (unsigned int *)shmat(clock_sid, NULL, 0);
    rd = (struct ResourceDescriptor *)shmat(rd_id, NULL, 0);
    sr = (struct StatRecord *)shmat(sr_id, NULL, 0);
    
    if (clock_ns == (void *) -1 || clock_s == (void *) -1 || rd == (void *) -1 || sr == (void *) -1) {
       perror("oss: Error: Shared memory attachment failed");
      //return 1;
       abort();
    }
   
   /* Setting System Clock to zero */ 
    clock_ns[0] = 0;
    clock_s[0] = 0;
    
    srand(time(NULL));
   
   /* Initializing Resource descriptor */
       
    for (int r = 0; r < nresources; r++){
        /* Assigning a number between 1 and 20 (inclusive) for the initial instances in each resource */
        rd->resourceVector[r] = (rand() % 20) + 1;
        rd->releaseVector[r] = 0;
    }
   
    memset(rd->allocationMatrix, 0, sizeof(rd->allocationMatrix));
    memcpy(rd->allocationVector, rd->resourceVector, sizeof(rd->allocationVector));
    
   /* variables */ 
    int localpidCount = 0, forkedchild = 0, grantedReq = 0;
    unsigned int processScheduleNS, trackNS;
    bool runDeadlock = true, waitingQ[MAXPROC] = {false}, immediate[MAXPROC] = {false}, deadlockdetected = false;
    
    /* terminated after 5 real seconds */
    alarm(terminate_time);   
   
   /* Logging */
    file = fopen("osslogfile", "w+");
    
   /* Simulation starts */ 
    while(1){
        /* inceases system clock */    
        increase_clock(1000);
        
        /* random time for forking (1-500ms) */
        if (doSim){
            doSim = false;
            processScheduleNS =  (*clock_s * TO_NANO) + *clock_ns + ((rand() % 500000000) + 1000000);         
        }
        /* time track for periodic deadlock detection */
        if (runDeadlock){
            runDeadlock = false;
            trackNS = (*clock_s * TO_NANO) + *clock_ns;           
        }

       /* check if it should terminate */
        for (int i = 0; i < nprocs; i++) {
            if (rd->allpid[i] > 0) {
                break;
            } else {
                if(forkedchild >= 40){
                   signal_abort();
                }
            }
        }

        /* fork user process when hit random time */
        if ((((*clock_s * TO_NANO) + *clock_ns) > processScheduleNS) && forkedchild < 40) {
            increase_clock(10000);
            childPid = fork();//Forking the process
            /* Checking fork */
            if (childPid == -1) {
                 perror("OSS: Error: Failed to create a child process");
	         cleanAll();
                 abort();
            }            
            if (childPid == 0) {
                char procid[256];
                sprintf(procid, "%d", localpidCount);
                execl("./process", "./process", procid, nprocs, nresources, NULL);//execs the process
                children[localpidCount] = childPid; 
            }
            else {
                rd->allpid[localpidCount] = childPid;
                parents[localpidCount] = childPid; 
            }
            doSim = true;
            forkedchild++;           
        }


        /* Checking if resource is requested or released */
        for (int p = 0; p < nprocs; p++) {   //Processes iteration       
            for (int r = 0; r < nresources; r++) { // Resources iteration
                increase_clock(1000);
                if(rd->releaseVector[r] > 0){
                    rd->allocationVector[r] += rd->releaseVector[r];// incrementing available resource after releasing
                    rd->allocationMatrix[p][r] -= rd->releaseVector[r];// decrementing allocated resource after releasing
                    sprintf(buf, "\tResources released : R%i:%i\n", r, rd->releaseVector[r]);
                    logging(buf);
                    for (int i = 0; i < nprocs; i++) {//since resources has been released, waiting processes awake to check if they can get resource 
                        waitingQ[i] = false;
                    }
                    rd->releaseVector[r] = 0;// release vector set to zero
                }
                
                if (rd->requestMatrix[p][r] > 0) { // resource requested
                   sprintf(buf, "Master has detected Process P%d requesting R%d at time %d:%d\n", p, r, *clock_s, *clock_ns);
                   logging(buf);

                   if (rd->allocationVector[r] >= rd->requestMatrix[p][r]){//resource available --> grant Request
                        grantedReq++;
                        rd->allocationVector[r] -= rd->requestMatrix[p][r];// decrementing available resource after granting
                        rd->allocationMatrix[p][r] += rd->requestMatrix[p][r];// incrementing allocated resource after granting
                        sprintf(buf, "Master granting P%d request R%d at time %d:%d\n", p, r, *clock_s, *clock_ns);
                        logging(buf);//logging
                        rd->requestMatrix[p][r] = 0;//updating requested matrix to zero since request has been granted

                        if (immediate[p]){
                            sr->immediateGrant++;
                        }
                        else{
                            sr->waitedGrant++;
                            immediate[p] = true;
                        }
                    }else{ // resource not available --> keeping in waiting queue
                        sprintf(buf, "Master: no instances of R%d available, P%d added to wait queue at time %d:%d\n", r, p, *clock_s, *clock_ns);
                        logging(buf);//logging
                        waitingQ[p] = true;
                        immediate[p] = false;
                    }
                }else if (rd->requestMatrix[p][r] < 0) {
                    rd->allocationVector[r] += rd->requestMatrix[p][r];// incrementing available resource after releasing
                    rd->allocationMatrix[p][r] -= rd->requestMatrix[p][r];// decrementing allocated resource after releasing
                    sprintf(buf, "Master has acknowledged Process P%d requesting R%d at time %d:%d\n", p, r, *clock_s, *clock_ns);
                    logging(buf);
                    sprintf(buf, "\tResources released : R%i:%i\n", r, rd->requestMatrix[p][r]);
                    logging(buf);
                    for (int i = 0; i < nprocs; i++) {//since resources has been released, waiting processes awake to check if they can get resource 
                        waitingQ[i] = false;
                    }
                    rd->requestMatrix[p][r] = 0;// request matrix set to zero
                }
               
            }
        }

        /* deadlock detection algorithm is running periodically in every second */
        bool deadlockflag = true;
        if (((*clock_s * TO_NANO) + *clock_ns) > (trackNS + TO_NANO)) {    
            increase_clock(10000);
            while(1){
                sr->deadlockRuns++;
                if (deadlock(rd->allocationVector, nresources, nprocs, rd->requestMatrix , rd->allocationMatrix)){
                    for (int i = 0; i < nprocs; i++) {
                        if (waitingQ[i]) {//process in waiting queue
                            deadlockflag = true;
                            break;
                        } else {
                            deadlockflag = false;
                        }
                    }
                    if (!deadlockflag) break;
                 
                    sprintf(buf, "Master running deadlock detection at time %d:%d: deadlock detected\n", *clock_s, *clock_ns);
                    logging(buf);//logging
                    //deadload detected --> Aborting process in waiting queue
                    for (int p = 0; p < nprocs; p++) {
                        if (waitingQ[p]) {
                            sprintf(buf, "\tMaster terminating P%d at time %d:%d to remove deadlock\n",p, *clock_s, *clock_ns);
                            logging(buf);//logging
                            kill(rd->allpid[p], SIGTERM);
                            rd->allpid[p] = 0;

                            sprintf(buf, "\tResources released: ");
                            logging(buf);//logging
                            for (int r = 0; r < nresources; r++) {
                                if (rd->allocationMatrix[p][r] > 0) {
                                   sprintf(buf, "R%i:%i \t", r, rd->allocationMatrix[p][r]);
                                   logging(buf);//logging
                                   rd->allocationVector[r] += rd->allocationMatrix[p][r];
                                   rd->allocationMatrix[p][r] = 0;
                                  }
                             }
                             fprintf(file, "\n");


                            for (int r = 0; r < nresources; r++) {
                                rd->requestMatrix[p][r] = 0;
                            }
                            waitingQ[p] = false;
                            deadlockdetected = true;
                            sr->deadlockTerminations += 1;

                            break;
                        }
                    }
                } else {
                    sprintf(buf, "Master running deadlock detection at time %d:%d: No deadlock detected\n", *clock_s, *clock_ns);
                    logging(buf);//logging
                    deadlockdetected = false;
                    break;
                }
            }

            if (deadlockdetected) { // deadlock detected
                deadlockdetected = false;
                for (int p = 0; p < nprocs; p++) {
                    waitingQ[p] = false;//resetting waiting queue
                }                
            }
            runDeadlock = true;
        }

        if (verbose) {
            if (grantedReq >= 5) {
                sprintf(buf, "## Resource Vector ##\n");
                logging(buf);//logging
                for (int i = 0; i < nresources; i++) {
                    sprintf(buf, "\tR%d ", i);
                    logging(buf);//logging
                }
                fprintf(file, "\n");
                for (int i = 0; i < nresources; i++) {
                    sprintf(buf, "\t%d ", rd->resourceVector[i]);
                    logging(buf);//logging
                }                
                sprintf(buf, "\n## Allocation Vector ##\n");
                logging(buf);//logging                
                for (int i = 0; i < nresources; i++) {
                    sprintf(buf, "\tR%d ", i);
                    logging(buf);//logging
                }
                fprintf(file, "\n");              
                for (int i = 0; i < nresources; i++) {
                sprintf(buf, "\t%d ", rd->allocationVector[i]);
                logging(buf);//logging
                }
                sprintf(buf, "\n## Allocation Matrix ##\n");
                logging(buf);//logging   
                for (int i = 0; i < nresources; i++) {
                    sprintf(buf, "\tR%d ", i);
                    logging(buf);//logging
                }
                fprintf(file, "\n");
                for (int p = 0; p < nprocs; p++) {
                sprintf(buf, "P%d", p);
                logging(buf);//logging
                    for (int r = 0; r < nresources; r++) {
                	sprintf(buf, "\t %i", rd->allocationMatrix[p][r]);
                	logging(buf);//logging
                    }
                    fprintf(file, "\n");
                    }
                    grantedReq = 0;
            }            
        }
    }
    cleanAll();
    return 0;
}



/* signal handle for time out or CTRL+C or other interuption */
void signal_abort() {

    ReportStatistics(); 
    cleanAll();
}

void ReportStatistics(){
    sprintf(buf, "\n## Statistics Report ##\n");
    logging(buf);//logging
    sprintf(buf, "Number of requests granted immediately: %d\n", sr->immediateGrant);
    logging(buf);//logging
    sprintf(buf, "Number of requests granted after waiting: %d\n", sr->waitedGrant);
    logging(buf);//logging
    sprintf(buf, "Number of requests terminated by deadlock: %d\n", sr->deadlockTerminations);
    logging(buf);//logging
    sprintf(buf, "Number of requests terminated successfully: %d\n", sr->terminationSuccess);
    logging(buf);//logging
    sprintf(buf, "Number of time deadlock detection algorithm run: %d\n", sr->deadlockRuns);
    logging(buf);//logging    
    fclose(file);
}

void cleanAll(){
    int status;
    for (int p = 0; p < nprocs; p++) {
        wait(NULL);
        if (rd->allpid[p] != 0) {
            kill(rd->allpid[p], SIGTERM);
            waitpid(rd->allpid[p], &status, 0);
        }
    }  
    killpg((*parents), SIGTERM);
    killpg((*children), SIGTERM);
    sleep(2);
    if (shmdt(clock_ns) == -1 || shmdt(clock_s) == -1 || shmdt(rd) == -1 || shmdt(sr) == -1) {
      perror("OSS: Error: shmdt failed to detach memory");
      cleanAll();
      abort();
    }
    if (shmctl(clock_nsid, IPC_RMID, 0) == -1 || shmctl(clock_sid, IPC_RMID, 0) == -1 || shmctl(rd_id, IPC_RMID, 0) == -1 || shmctl(sr_id, IPC_RMID, 0) == -1) {
      perror("OSS: Error: shmctl failed to delete shared memory");
      cleanAll();
      abort();
    }   
    cleanAll();     
    abort();
    exit(EXIT_SUCCESS);
}

void increase_clock(int inc){
        clock_ns[0] = clock_ns[0] + inc;//Increasing system clock
        while (clock_ns[0] >= TO_NANO) {//Coversion of nanosecond to second 
               clock_s[0] = clock_s[0] + 1;
               clock_ns[0] = clock_ns[0] - TO_NANO;
         }
}


void logging(char * str){
    linenum++;
    if (linenum <= 10000){
        fputs(str, file);
    }
    else{
        printf("OSS: logfile exceeds 10000 line, terminating\n");
        fputs("OSS: logfile exceeds 10000 line, terminating\n", file);
        cleanAll();
    }
}

/* From class notes/slides */
bool deadlock(int allocationVector[MAXRESOURCE], const int m, const int n, int requestMatrix[MAXPROC][MAXRESOURCE], int allocated[MAXPROC][MAXRESOURCE]){
    int work[m];    // m resources
    bool finish[n]; // n processes
    for (int i = 0; i < m; i++) work[i] = rd->allocationVector[i]; //Available resources
    for (int i = 0; i < n; finish[i++] = false);
    
    int p = 0;
    for (; p < n; p++)
    {
        if (finish[p]) continue; 
        if (req_lt_avail((int *)requestMatrix, work, p, m)){
            finish[p] = true;
            for (int i = 0; i < m; i++) {
                work[i] += allocated[p][i];
            }
            p = -1;
        }
    }

    for (p = 0; p < n; p++) {
        if (!finish[p]) {
            break;
        }
    }

    return (p != n);
}

bool req_lt_avail(const int *req, const int *avail, const int pnum, const int num_res){
    int i = 0;
    for (; i < num_res; i++) {
        if (rd->requestMatrix[pnum][i] > avail[i]) {
            break;
        }
    }

    return (i == num_res);
}


