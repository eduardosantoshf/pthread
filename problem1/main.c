/**
 *  \file main.c (implementation file)
 *
 *  \brief Problem name: Total number of words, number of words beginning with a vowel and ending with a consonant.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author Eduardo Santos and Pedro Bastos - April 2022
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <libgen.h>

#include "probConst.h"
#include "shared.h"

/** \brief time limits */
struct timespec start, finish;

/** \brief Number of files */
int num_files;

/** \brief pointer to save the filenames */
char **filenames;

/** \brief array to save the total number of words for each file */
int *array_num_words;

/** \brief array to save the number of words beginning with a vowel for each file */
int *array_num_vowels;

/** \brief array to save the number of words ending with a consonant for each file */
int *array_num_cons;

/** \brief worker threads return status array */
int statusCons[N];

/** \brief worker life cycle routine */
static void *worker(void *id);

/** \brief Prints command usage */
static void printUsage(char *cmdName)
{
  fprintf(stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
                  "  OPTIONS:\n"
                  "  -h      --- print this help\n"
                  "  -f      --- filename\n"
                  "  -n      --- positive number\n",
          cmdName);
}

/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (workers),
 *  waiting for their termination and printing the final results for each file.
 */
int main(int argc, char *argv[]) {
  
  pthread_t tIdCons[N];                                                       /* workers internal thread id array */
  unsigned int cons[N];                                            /* workers application defined thread id array */
  filenames = malloc((argc - 1) * sizeof(char *));                /* Allocate the needed memory for the filenames */

  int i;                                                                                     /* counting variable */
  int *status_p;                                                                   /* pointer to execution status */

  int opt;                                                                                     /* selected option */
  char *fName = "no name";                                     /* file name (initialized to "no name" by default) */
  int value_opt = -1;                                             /* numeric value (initialized to -1 by default) */


  /* Handle command line options */
  do {
    switch ((opt = getopt(argc, argv, "f:n:h"))) {
      case 'f':                                                                                      /* file name */
        if (optarg[0] == '-') {
          fprintf(stderr, "%s: file name is missing\n", basename(argv[0]));
          printUsage(basename(argv[0]));
          return EXIT_FAILURE;
        }
        fName = optarg;
        break;

      case 'n':                                                                               /* numeric argument */
        if (atoi(optarg) <= 0) {
          fprintf(stderr, "%s: non positive number\n", basename(argv[0]));
          printUsage(basename(argv[0]));
          return EXIT_FAILURE;
        }
        value_opt = (int)atoi(optarg);
        break;

      case 'h':                                                                                      /* help mode */
        printUsage(basename(argv[0]));
        return EXIT_SUCCESS;

      case '?':                                                                                 /* invalid option */
        fprintf(stderr, "%s: invalid option\n", basename(argv[0]));
        printUsage(basename(argv[0]));
        return EXIT_FAILURE;

      case -1:
        break;

    }

  } while (opt != -1);

  if (argc == 1) {
    fprintf(stderr, "%s: invalid format\n", basename(argv[0]));
    printUsage(basename(argv[0]));
    return EXIT_FAILURE;
  }


  /* Save filenames */
  for (int i = 2; i < argc; i++){
    filenames[i - 2] = argv[i];
  }

  /* Save number of files */
  num_files = argc - 2;

  /* allocate the needed space in the arrays to save the results */
  array_num_words = (int *)malloc(num_files * sizeof(int));
  array_num_vowels = (int *)malloc(num_files * sizeof(int));
  array_num_cons = (int *)malloc(num_files * sizeof(int));

  for (i = 0; i < N; i++)
    cons[i] = i;

  srandom((unsigned int)getpid());
  clock_gettime (CLOCK_MONOTONIC_RAW, &start);                                            /* begin of measurement */

  /* generation of worker threads */
  for (i = 0; i < N; i++)
    if (pthread_create(&tIdCons[i], NULL, worker, &cons[i]) != 0){                             /* thread worker */
      perror("error on creating thread worker");
      exit(EXIT_FAILURE);
    }

  /* waiting for the termination of the worker threads */
  for (i = 0; i < N; i++) {
    if (pthread_join(tIdCons[i], (void *)&status_p) != 0) {                                      /* thread worker */
      perror("error on waiting for thread customer");
      exit(EXIT_FAILURE);
    }

    printf("thread worker, with id %u, has terminated: ", i);
    printf("its status was %d\n", *status_p);
  }

  printf ("\nFinal report\n\n");

  clock_gettime (CLOCK_MONOTONIC_RAW, &finish);                                             /* end of measurement */

  /* call function to print final results */
  print_final_results();

  /* print time spent */
  printf ("\nElapsed time = %.6f s\n",  (finish.tv_sec - start.tv_sec) / 1.0 + (finish.tv_nsec - start.tv_nsec) / 1000000000.0);

  exit(EXIT_SUCCESS);
}

/**
 *  \brief Function worker.
 *
 *  Its role is to simulate the life cycle of a worker.
 *
 *  \param par pointer to application defined worker identification
 */

static void *worker(void *par){

  /* worker id */
  unsigned int id = *((unsigned int *)par);

  /* while there is files to read */
  while (check_for_file(id)) {

    /* while there is data to read from the current file */
    while (getVal(id)){
      usleep((unsigned int)floor(40.0 * random() / RAND_MAX + 1.5));
    }
    
    /* saves results for each file in the variables */
    save_file_results(id);

    /* close current file */
    check_close_file(id);
  }

  statusCons[id] = EXIT_SUCCESS;
  pthread_exit(&statusCons[id]);
}
