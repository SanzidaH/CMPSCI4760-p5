#include "config.h"

int main(int argc, char *argv[])
{
     /* shared memory segment */ 
    clock_nsid = shmget(ftok("Makefile", '1'), sizeof(clock_ns), IPC_EXCL  | 0666);
    clock_sid = shmget(ftok("Makefile", '2'), sizeof(clock_s), IPC_EXCL  | 0666);
    rd_id = shmget(ftok("Makefile", '3'), sizeof(rd), IPC_EXCL  | 0666);
    sr_id = shmget(ftok("Makefile", '4'), sizeof(sr), IPC_EXCL  | 0666);
   
    if (clock_nsid  == -1 || clock_sid == -1 || rd_id == -1 || sr_id == -1) {
       perror("Process: Error: Shared memory allocation failed");
     // return 1;
       //abort();
    }
    
    clock_ns = (unsigned int *)shmat(clock_nsid , NULL, 0);
    clock_s = (unsigned int *)shmat(clock_sid, NULL, 0);
    rd = (struct ResourceDescriptor *)shmat(rd_id, NULL, 0);
    sr = (struct StatRecord *)shmat(sr_id, NULL, 0);
    
    if (clock_ns == (void *) -1 || clock_s == (void *) -1 || rd == (void *) -1 || sr == (void *) -1) {
       perror("Process: Error: Shared memory attachment failed");
      //return 1;
    }
    
    int p = atoi(argv[1]);  //local procid
    //int nprocs = atoi(argv[2]); 
    int nresources = atoi(argv[3]); 
    srand(time(NULL) + getpid());
   
    unsigned int processScheduleNS = 0, terminationNS = 0, procStartNS = (*clock_s * TO_NANO) + *clock_ns, currenTimeTemp = 0;
    bool doSim = true, terminationCheck = true, request = false, release = false, terminate = false;
    int randomResource = 0, randomResourceNo = 0;
    
    if ((rand() % 100) < 20) {
            terminate = true;
    }
    else if ((rand() % 100) < 50) {
            request = true;
    }
    else if ((rand() % 100) < 40) {
            release = true;
    }


   // procStartNS = (*clock_s * TO_NANO) + *clock_ns;

    while(1){
        /* inceases system clock */    
        increase_clock(1000);
        //continue to loop and check to see if it is granted that resource
        for (int r = 0; r < nresources; r++) { // resource requested
            if (rd->requestMatrix[p][r] > 0) { 
                while (rd->requestMatrix[p][r] > 0);
            }
        }

        /* random times for requesting or releasing resource */
        int bound = 10000000;
        if (doSim){
            doSim = false;
            processScheduleNS =  (*clock_s * TO_NANO) + *clock_ns + ((rand() % bound) + 0);        
        }
        /* random times (between 0 and 250ms) for process checking if it should terminate */        
        if (terminationCheck) {
            terminationCheck = false;
            terminationNS = (*clock_s * TO_NANO) + *clock_ns + ((rand() % 250000000) + 1);           
        }
        /* Hitting the random time for requesting or releasing resource */
        if (((*clock_s * TO_NANO) + *clock_ns) > processScheduleNS) {
            randomResource = (rand() % 9) + 0;
            
            if (request) {
                randomResourceNo = (rand() % rd->resourceVector[randomResource]) + 1; //random number of resources to request
                rd->requestMatrix[p][randomResource] += randomResourceNo;
            } 
            if (release) {
                randomResourceNo = (rand() % rd->allocationMatrix[p][randomResourceNo]) + 1; //random number of resources to release
                rd->releaseVector[randomResourceNo] = randomResourceNo;
            } 

          doSim = true;
        }
        
        /* Checking if process has run for at least 1 second and hitted termination time */
        currenTimeTemp = (*clock_s * TO_NANO) + *clock_ns;
        if ((currenTimeTemp > (procStartNS + TO_NANO)) && ( currenTimeTemp > terminationNS)) { 
            //Do a russian roulette dice roll
            if (terminate) {
                rd->requestMatrix[p][0] = -30;
                sr->terminationSuccess += 1;
                cleanAll();
            }

            terminationCheck = true;
        }
    }

    cleanAll();
    return 0;
}
void increase_clock(int inc){
        clock_ns[0] = clock_ns[0] + inc;//Increasing system clock
        while (clock_ns[0] >= TO_NANO) {//Coversion of nanosecond to second 
               clock_s[0] = clock_s[0] + 1;
               clock_ns[0] = clock_ns[0] - TO_NANO;
         }
}

void cleanAll() {

    if (shmdt(clock_ns) == -1 || shmdt(clock_s) == -1 || shmdt(rd) == -1 || shmdt(sr) == -1) {
      perror("process: Error: shmdt failed to detach memory");
    }
    abort();
    exit(EXIT_SUCCESS);
}

