/*
    Wesley Yue, January 27, 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define NUM_THREADS     5

// gcc -pthread -o threads_WY threads_WY.c

/*------ Function prototypes ----------*/
void createThreads();
void *calculateFibonacci(void *);

/*--- Demonstrate threads ---*/
int main (void)
{
    createThreads();
    pthread_exit(0);

    exit(0);
}

void createThreads() {

    pthread_t thread[NUM_THREADS];
    int i;
    int num = 0;

    /* User enters the number amount of patterns for Fibonacci */
    if (num <= 0) {
        printf("Specify a number below 340 to generate x amount of patterns for Fibonacci. (Repeated x5)\t");
        scanf("%d",&num);
    }

    /* Create and run the threads, and wait until they are done */
    for (i=0; i<NUM_THREADS; i++) {
        pthread_create (&thread[i], NULL, calculateFibonacci, &num);
    }
}

void *calculateFibonacci(void *number){

    FILE *file;

    int *nPattern = (int *)number;

    int i;
    double firstNumber = 0, secondNumber = 1, sum;

    struct timeval start, end;
    struct tm* ptm;
    char* hmsStartTime, *hmsEndTime;
    long startMicroseconds, endMicroseconds;
    //int totalTime;

    char tempTimeBuff[15], timeBuff[15];
    char tempStoreBuff[131072], storeBuff[131072];

    /* Gets the local time when the process started */
    gettimeofday(&start, NULL);
    ptm = localtime (&start.tv_sec);

    /* Format the date and time, down to milliseconds, and store it into the timeBuff */
    strftime(tempTimeBuff, sizeof tempTimeBuff, "%H:%M:%S", ptm);
    sprintf(timeBuff, tempTimeBuff, start.tv_usec);

    /* The startTime timeBuff is passed onto the variables */
    hmsStartTime = strncpy(timeBuff, timeBuff, sizeof timeBuff);
    startMicroseconds = start.tv_sec * 1000000 + (start.tv_usec);

    for (i=0; i < (*nPattern); i++) {

        printf("THREAD ID: %u | Pattern %d) ==== %.0lf\n",(unsigned int)pthread_self(), i+1, firstNumber);

        /* Format the string and store it into the storeBuff */
        sprintf(tempStoreBuff, "THREAD ID: %u | Pattern %d) ==== %.0lf\n",(unsigned int)pthread_self(), i+1, firstNumber);
        strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff);

        sum = firstNumber + secondNumber;
        firstNumber = secondNumber;
        secondNumber = sum;
    }

    /* Gets the local time when the process ended */
    gettimeofday(&end, NULL);
    ptm = localtime (&end.tv_sec);

    /* Format the date and time, down to milliseconds, and store it into the timeBuff */
    strftime(tempTimeBuff, sizeof tempTimeBuff, "%H:%M:%S", ptm);
    sprintf(timeBuff, tempTimeBuff, end.tv_usec);

    /* The endTime timeBuff is passed onto the variables */
    hmsEndTime = strncpy(timeBuff, timeBuff, sizeof timeBuff);
    endMicroseconds = end.tv_sec * 1000000 + (end.tv_usec);

    /* printf("\nTHREAD ID: %u\tStarting time:\t%s.%ld\n\t\t\tEnding time:\t%s.%ld\n\n"
            ,(unsigned int)pthread_self(), hmsStartTime, startMicroseconds, hmsEndTime, endMicroseconds); */

    /* Store the printf above into the storeBuff */
    /* sprintf(tempStoreBuff, "\nTHREAD ID: %u\tStarting time:\t%s.%ld\n\t\t\tEnding time:\t%s.%ld\n\n"
            ,(unsigned int)pthread_self(), hmsStartTime, startMicroseconds, hmsEndTime, endMicroseconds);
    strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff); */

    printf("Total thread time for ID: %u = %ld microseconds\n\n", (unsigned int)pthread_self(), endMicroseconds - startMicroseconds );

    /* Store the printf above into the storeBuff */
    sprintf(tempStoreBuff, "Total thread time for ID: %u = %ld microseconds\n\n", (unsigned int)pthread_self(), endMicroseconds - startMicroseconds );
    strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff);

    file = fopen("threadsResults.txt", "a+");
    fprintf(file,"%s",storeBuff);
    fclose(file);

    return 0;
}
