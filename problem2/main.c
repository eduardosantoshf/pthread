/**
 *  \file main.c (implementation file)
 *
 *  \brief Problem name: Determinant of a square matrix
 *
 *  Definition of the operations carried out by this class:
 *      \li validates de user input
 *      \li lists all the files for each we will compute each matrices' determinant and send them to the shared region
 *      \li instantiates the worker threads
 *      \li awaits for the workers to reach the end oh their lifecycle
 *      \li invokes a method on the shared region so that the results are printed
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
#include <time.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "probConst.h"
#include "sharedRegion.h"
#include "PartialInfo.h"

PartialInfo **finalInfo;
int *totalMatrices;

/** \brief process the called command. */
static int process_command(int argc, char *argv[]);

/** \brief Print the explanation of how to use the command. */
static void printUsage (char *cmdName);

int *statusWorker;

/** \brief producer life cycle routine */
static void *worker (void *par);

struct timespec start, finish;                                                                                  /* time limits */

/**
 * \brief Compute determinant
 *
 *  \param order matrix order
 *  \param matrix matrix array
 *
 *  \return matrix's determinant
 */
double computeDet(int order, double **matrix) {
    double ratio;
    double det = 1;
    int i, j, k;

    //t0 = ((double) clock ()) / CLOCKS_PER_SEC;

    //Here we are using Gauss Elimination
    //Technique for transforming matrix to
    //upper triangular matrix

    // Applying Gauss Elimination         
    for (i = 0; i < order; i++)
    {
        if (matrix[i][i] == 0.0)
        {
            printf("Mathematical Error!");
            exit(0);
        }
        for (j = i + 1; j < order; j++)
        {
            ratio = matrix[j][i] / matrix[i][i];

            for (k = 0; k < order; k++)
            {
                    matrix[j][k] = matrix[j][k] - ratio * matrix[i][k];
            }
        }
    }

    // Finding determinant by multiplying
    // elements in principal diagonal elements 
    for (i = 0; i < order; i++)
        det = det * matrix[i][i];

    return det;
}

/**
 * \brief Main method
 *
 * Will read and validate the input.
 * Its role is to send the files to be processed to the shared region, launch the workers and wait for their
 * lifecycle to end.
 * In the end, accesses the shared region to obtain the results and stores them in files.
 */
int main(int argc, char * argv[]) {
    int threads = N;

    // will hold the output of processing the command
    int command_result;

    // process the command and act according to it
    command_result = process_command(argc, argv);
    if (command_result != EXIT_SUCCESS)
        return command_result;

    clock_gettime (CLOCK_MONOTONIC_RAW, &start);                                                       /* begin of measurement */

    statusWorker = malloc(sizeof(int)*threads);

    pthread_t tIdworker[threads];
    unsigned int workers[threads];
    int *status_p;                                                                              /* pointer to execution status */

    for (int t = 0; t < threads; t++)
        workers[t] = t;
    
    int numberOfFiles = argc - 2;
    storeFileNames(numberOfFiles, argv);

    /* --------------- THREADS --------------- */
    for (int t = 0; t < threads; t++){
        if (pthread_create (&tIdworker[t], NULL, worker, &workers[t]) != 0)                                 /* thread producer */
        { 
            perror ("Error on creating thread worker");
            exit (EXIT_FAILURE);
        }
    }
    
    for (int t = 0; t < threads; t++){
        if (pthread_join (tIdworker[t], (void *) &status_p) != 0)                                           /* thread producer */
        { 
            perror ("Error on waiting for thread worker");
            exit (EXIT_FAILURE);
        }
        
        printf ("Thread worker, with id %d, has terminated: ", t);
        printf ("Its status was %d\n", *status_p);
    }

    /*
    for (int i = 0; i < numberOfFiles; i++) {
        for (int j = 0; j < totalMatrices[i]; j++) {
            printf("det: %f\n", finalInfo[i][j].det);
        }
    }
    */

    clock_gettime (CLOCK_MONOTONIC_RAW, &finish);                                                        /* end of measurement */

    printf ("\nElapsed time = %.6f s\n",  (finish.tv_sec - start.tv_sec) / 1.0 + (finish.tv_nsec - start.tv_nsec) / 1000000000.0);

    return 0;

}

/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */
static void *worker (void *par) {
    unsigned int id = *((unsigned int *) par);

    printf("Thread %d created \n", id);

    PartialInfo info; 

    while (getVal(id, &info) != 2) {
        info.det = computeDet(info.order, info.matrix);
        printf("det for file %d matrix %d: %.3e \n", info.file_id, info.matrix_id, info.det);
    }

    statusWorker[id] = EXIT_SUCCESS;
    pthread_exit (&statusWorker[id]);
}

/**
 * \brief Processes the input command
 * @return int representing the success or failure of the method
 */
static int process_command(int argc, char *argv[]) {
    int opt;                  /* selected option */
    char *fName = "no name";  /* file name (initialized to "no name" by default) */
    int val = -1;             /* numeric value (initialized to -1 by default) */
    opterr = 0;
    do
    { switch ((opt = getopt (argc, argv, "f:n:h"))) { 
        case 'f': /* file name */
                if (optarg[0] == '-')
                    { fprintf (stderr, "%s: file name is missing\n", basename (argv[0]));
                    printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                    }
                    fName = optarg;
                    break;
        case 'n': /* numeric argument */
                    if (atoi (optarg) <= 0)
                    { fprintf (stderr, "%s: non positive number\n", basename (argv[0]));
                        printUsage (basename (argv[0]));
                        return EXIT_FAILURE;
                    }
                    val = (int) atoi (optarg);
                    break;
        case 'h': /* help mode */
                    printUsage (basename (argv[0]));
                    return EXIT_SUCCESS;
        case '?': /* invalid option */
                    fprintf (stderr, "%s: invalid option\n", basename (argv[0]));
                printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
        case -1:  break;
        }
    } while (opt != -1);

    if (argc == 1){ 
        fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
        printUsage (basename (argv[0]));
        return EXIT_FAILURE;
    }

    int o; /* counting variable */

    printf ("File name = %s\n", fName);
    printf ("Numeric value = %d\n", val);

    for (o = 0; o < argc; o++)
        printf ("Word %d = %s\n", o, argv[o]);

    return EXIT_SUCCESS;
}

/**
 * \brief Print command usage.
 * \param cmdName string with the command
 */
static void printUsage (char *cmdName)
{
  fprintf (stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
           "  OPTIONS:\n"
           "  -h      --- print this help\n"
           "  -f      --- filename\n"
           "  -n      --- positive number\n", cmdName);
}