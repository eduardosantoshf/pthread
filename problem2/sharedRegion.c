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
static int c = 0;



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

    size = finalInfo[currFile].order * finalInfo[currFile].order; // working correctly, on a 128 * 128 matrix order = 16 384

    for(int i = 0; i < size ; i++) {
        finalInfo[currFile].matrix[i] = malloc(size * sizeof(int));
    }
    
    printf("ENTROU AQUI\n");

    fread(finalInfo[currFile].matrix, sizeof(size), 1, file[currFile]);

    return;
}

void storeFileNames(int filesNumber, char * fileNames[]) {
    numberOfFiles = filesNumber;                     //number of files
    
    for (int i = 2; i < 2 + filesNumber; i++){
        files[c] = fileNames[i];
        c++;
    }

    openNextFile();
}

int getVal(int threadID, int* fileID, int* order, double ** matrix) {
    if ((statusWorker[threadID] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
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
        *fileID = currFile;
        *order = finalInfo[currFile].order;
        //**matrix = finalInfo[currFile].matrix;
        matrix = finalInfo[currFile].matrix;
        currIndex++;
        status = 0;


    }
    else
        status = 2; // status 2 == endProcess

    if ((statusWorker[threadID] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
    }

    return status;
}

void savePartialResults(int threadID, int fileID, int matrixNumber, double det) {
     // Here we need the lock to write partial results from PartialInfo to FinalInfo
    // Only after partial results are saved, that we can save final results
    if ((statusWorker[threadID] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
    }

    // Actual writing to struct
    finalInfo[fileID].det = det;

    if ((statusWorker[threadID] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
    }
}

void storeResults() {

}

void checkProcessingResults() {
    
}