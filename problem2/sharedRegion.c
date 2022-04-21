/**
 *  \file sharedRegion.c (implementation file)
 *
 *  \brief Problem name: Determinant of a square matrix
 *
 *  Definition of the operations carried out by the workers:
 *     \li add the files to be processed
 *     \li get a piece of data to be processed
 *     \li store the results processed by the workers
 *     \li print the results
 *
 *  \author Eduardo Santos and Pedro Bastos - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "probConst.h"
#include "PartialInfo.h"

/** \brief worker status */
extern int *statusWorker;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

static int numberOfFiles;
static char *files[15];
static FILE *file [15];
static int currFile = 0;
static int currIndex = 0;
static int finishedFiles = 0;
struct PartialInfo finalInfo[15];



void openNextFile() {
    if (numberOfFiles == currFile){
        printf("\nAll files read\n");
        finishedFiles = 1;
        return;
    }

    currIndex = 0;

    printf("---------------OPENED %s-------------- \n", files[currFile]);

    file[currFile] = fopen(files[currFile],"rb");

    int order = 0;
    int size = 0;

    fread(&order, sizeof(int), 1, file[currFile]);

    finalInfo[currFile].order = order;
    finalInfo[currFile].matrix = malloc(size * sizeof(int *));

    size = finalInfo[currFile].order * finalInfo[currFile].order;

    for(int i = 0; i < size ; i++) {
        finalInfo[currFile].matrix[i] = malloc(size * sizeof(int));
    }

    fread(finalInfo[currFile].matrix, sizeof(size), 1, file[currFile]);

    return;
}

void storeFileNames(int filesNumber, char * fileNames[]) {
    //int nextfile;
    numberOfFiles = filesNumber;                     //number of files

    for (int i = 0; i < filesNumber; i++){
        files[i] = fileNames[i];
    }

    openNextFile();
}

int getVal(int threadId, int* fileId, int order, double ** matrix) {
    if ((statusWorker[threadId] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }
    

    // This condition means we have reached the end of the file, so the next point we want to process is
    if (finalInfo[currFile].order == currIndex) {
        currFile++;
        openNextFile();
    }

    int status;
    if (!finishedFiles)    //work is not over
    {

        // Writing to the variables we need to
        *fileId = currFile;
        order = finalInfo[currFile].order;
        //**matrix = finalInfo[currFile].matrix;
        matrix = finalInfo[currFile].matrix;
        currIndex++;
        status = 0;


    }
    else
        status = 2; // status 2 == endProcess

    //printf("THREAD %d released lock on ProcessConvPoint, status=%d\n\n", threadId, status);
    if ((statusWorker[threadId] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadId];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadId] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadId]);
    }

    return status;
}