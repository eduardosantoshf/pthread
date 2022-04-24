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
#include <stdbool.h>
#include "probConst.h"
#include "PartialInfo.h"

/** \brief worker status */
extern int *statusWorker;

extern PartialInfo **finalInfo;
extern int *totalMatrices;

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;


static int numberOfFiles;
static char **files;
static FILE *file [15];
static int currFile = 0;        
static int currIndex = 0;
static bool finishedFiles = false;
static int c = 0;

PartialInfo **finalInfo;
int *totalMatrices;



void openNextFile() {
    currIndex = 0;

    if (numberOfFiles <= currFile) return;

    printf("---------------OPENED %s-------------- \n", files[currFile]);
    printf("\n");
    
    file[currFile] = fopen(files[currFile],"rb");

    if (file[currFile] == NULL) {
        printf("Error! File %s not found.\n", files[currFile]);
        exit(1);
    }

    int nMatrices = 0;
    int order = 0;
    int size = 0;

    fread(&nMatrices, sizeof(int), 1, file[currFile]);
    fread(&order, sizeof(int), 1, file[currFile]);

    size = order * order;
    totalMatrices[currFile] = nMatrices;
    finalInfo[currFile] = malloc(sizeof(*finalInfo[0])*nMatrices);
    
    for (int i = 0; i < nMatrices; i++) {
        finalInfo[currFile][i].file_id = currFile + 1;
        finalInfo[currFile][i].matrix_id = i + 1;
        finalInfo[currFile][i].order = order;
        finalInfo[currFile][i].matrix = malloc(order * sizeof(double *));

        for(int ii = 0; ii < order; ii++) {
            finalInfo[currFile][i].matrix[ii] = malloc(order * sizeof(double));
            fread(finalInfo[currFile][i].matrix[ii], sizeof(double), order, file[currFile]);
        }
        
        // for(int x = 0; x < order; x++)
        // {
        //     for(int j = 0; j < order; j++)
        //     {
        //         printf("%7.2f\t",finalInfo[currFile][i].matrix[x][j]);
        //     }
        //     printf("\n");
        // }
    }
    
    return;
}

void storeFileNames(int filesNumber, char * fileNames[]) {
    numberOfFiles = filesNumber;                     //number of files
    files = malloc(sizeof(char *)*numberOfFiles);

    for (int i = 2; i < 2 + filesNumber; i++){
        files[c] = fileNames[i];
        c++;
    }

    finalInfo = malloc(sizeof(*finalInfo)*numberOfFiles);
    totalMatrices = malloc(sizeof(int)*numberOfFiles);

    openNextFile();
}

int getVal(int threadID, PartialInfo *info) {
    if ((statusWorker[threadID] = pthread_mutex_lock (&accessCR)) != 0)                                   /* enter monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on entering monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
    }

    if (totalMatrices[currFile] <= currIndex) {
        //printf("Read next file\n");
        currFile++;
        openNextFile();
    }

    if (numberOfFiles <= currFile){
        //printf("\nAll files read\n");
        finishedFiles = true;
    }

    int status;
    if (!finishedFiles)    //work is not over
    {
        //printf("reading data\n");
        // Writing to the variables we need to
        (*info) = finalInfo[currFile][currIndex];
        
        currIndex++;
        status = 0;
    }
    else {
        //printf("exiting\n");
        status = 2; // status 2 == endProcess
    }

    if ((statusWorker[threadID] = pthread_mutex_unlock (&accessCR)) != 0)                                  /* exit monitor */
    { 
        errno = statusWorker[threadID];                                                            /* save error in errno */
        perror ("error on exiting monitor(CF)");
        statusWorker[threadID] = EXIT_FAILURE;
        pthread_exit (&statusWorker[threadID]);
    }
    //printf("outside lock\n");

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
    finalInfo[fileID]->det = det;

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