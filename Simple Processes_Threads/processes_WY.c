/*
    Wesley Yue, January 25, 2011
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>

#define MINPRSSIZE 5
#define MINFIBOSIZE 5

/*------ Function prototypes ----------*/
void forkChildren(int);
char* calculateFibonacci(int);
//void getTime(char*);

/*--- Demonstrate processes ---*/
int main (void)
{
    forkChildren(MINPRSSIZE);
    exit(0);
}

void forkChildren (int nChildren) {
    int i;
    //int processTime[MINPRSSIZE];
    struct timeval start, end;
    struct tm* ptm;

    char tempBuff[15], buff[15];
    char tempStoreBuff[131072], storeBuff[131072];

    char* hmsStartTime, *hmsEndTime;
    long startMicroseconds, endMicroseconds;

    //int totalTime = 0;
    int num = 0;

    FILE *file;

    pid_t pid;

    /* User enters the number amount of patterns for Fibonacci */
    if (num <= 0) {
        printf("Specify a number below 340 to generate x amount of patterns for Fibonacci. (Repeated x5)\t");
        scanf("%d",&num);
    }

    for (i = 1; i <= MINPRSSIZE; i++) {
        pid = fork();

        /* Gets the local time when the process started */
        gettimeofday(&start, NULL);
        ptm = localtime (&start.tv_sec);

        /* Format the date and time, down to milliseconds, and store it into the buff */
        strftime(tempBuff, sizeof tempBuff, "%H:%M:%S", ptm);
        sprintf(buff, tempBuff, start.tv_usec);

        /* The hmsStartTime buff is passed onto the variables */
        hmsStartTime = strncpy(buff, buff, sizeof buff);
        startMicroseconds = start.tv_sec * 1000000 + (start.tv_usec);
        /* Checks if the processes are created */
        if (pid == -1) {
            perror("Fork failed.");
            return;
        }
        else if (pid > 0) {
            /* Gets the time when the process started */
            //getTime(&time);

           exit(0);
        }
        else if (pid == 0) {

            /* Calculates the Fibonacci Pattern with the specified number */
            printf("%s", calculateFibonacci(num));

            /* Store the printf above into the storeBuff */
            sprintf(tempStoreBuff, "%s", calculateFibonacci(num));
            strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff);

            /* Gets the local time when the process ended */
            gettimeofday(&end, NULL);
            ptm = localtime (&end.tv_sec);

            /* Format the date and time, down to milliseconds, and store it into the buff */
            strftime(tempBuff, sizeof tempBuff, "%H:%M:%S", ptm);
            sprintf(buff, tempBuff, end.tv_usec);

            /* The hmsEndTime buff is passed onto the variables */
            hmsEndTime = strncpy(buff, buff, sizeof buff);
            endMicroseconds = end.tv_sec * 1000000 + (end.tv_usec);

            /*printf("\nPROCESS PID: %d\tStarting time:\t%s.%ld\n\t\t\tEnding time:\t%s.%ld\n\n"
                    ,getpid(), hmsStartTime, startMicroseconds, hmsEndTime, endMicroseconds); */

            /* Store the printf above into the storeBuff */
          /*  sprintf(tempStoreBuff, "\nPROCESS PID: %d\tStarting time:\t%s.%ld\n\t\t\tEnding time:\t%s.%ld\n\n"
                    ,getpid(), hmsStartTime, startMicroseconds, hmsEndTime, endMicroseconds);
            strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff); */

            printf("Total process time for ID: %d = %ld microseconds\n\n",getpid(), endMicroseconds - startMicroseconds);

            /* Store the printf above into the storeBuff */
            sprintf(tempStoreBuff, "Total process time for ID: %d = %ld microseconds\n\n",getpid(), endMicroseconds - startMicroseconds);
            strncat(storeBuff, tempStoreBuff, sizeof tempStoreBuff);

            /* Calculate the total time for the processes */
            //totalTime += processTime[i] = ((endMicroseconds) - (startMicroseconds));
        }
    }

    file = fopen("processResults.txt", "a+");
    fprintf(file,"%s",storeBuff);
    fclose(file);
}

char* calculateFibonacci(int nPattern){

    int i;
    double firstNumber = 0, secondNumber = 1, sum;

    char* filePrint;

    char buff[131072], storeBuff[131072];

    firstNumber = 0;
    secondNumber = 1;
    /* Clear the storeBuff buffer */
    strncpy(storeBuff, "", sizeof buff);

    for (i=0; i < (nPattern); i++) {
        /* Format the string and store it into the storeBuff */
        sprintf(buff, "PROCESS PID: %d | Pattern %d) ==== %.0lf\n", getpid(), i+1, firstNumber);
        strncat(storeBuff, buff, sizeof buff);

        sum = firstNumber + secondNumber;
        firstNumber = secondNumber;
        secondNumber = sum;
    }
    filePrint = storeBuff;
    return filePrint;
}



/*
void getTime(char* timetime) {

    struct timeval tv;
    struct tm* ptm;

    char tempBuff[64], buff[64];

    char* temp;

    gettimeofday(&tv, NULL);
    ptm = localtime (&tv.tv_sec);

    // Format the date and time, down to milliseconds
    strftime(tempBuff, sizeof tempBuff, "%H:%M:%S.%%06u", ptm);
    sprintf(buff, tempBuff, tv.tv_usec);

    *timetime = strncpy(buff, buff, 64);
    temp = strncpy(buff, buff, 64);

    printf("timetime: %s\n", timetime);
    printf("temp: %s\n", temp);

    //printf("'%s'\n", buff);

    // Print the formatted time, in seconds, followed by a decimal point
    and the milliseconds.
    //printf("%s.%03ld\n", buffer, milliseconds);
    //printf("%.6ld %d seconds elapsed\n", t1, t2);
}
*/
