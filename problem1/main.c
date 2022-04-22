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

/** \brief consumer threads return status array */
int statusCons[N];

/** \brief consumer life cycle routine */
static void *consumer(void *id);

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
 *  Its role is starting the simulation by generating the intervening entities threads (consumers),
 *  waiting for their termination and printing the final results for each file.
 */
int main(int argc, char *argv[]) {
  
  pthread_t tIdCons[N];            /* consumers internal thread id array */ /* consumers internal thread id array */
  unsigned int cons[N];                                          /* consumers application defined thread id array */
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

  /* allocate the needed space in the arrays to save the results*/
  array_num_words = (int *)malloc(num_files * sizeof(int));
  array_num_vowels = (int *)malloc(num_files * sizeof(int));
  array_num_cons = (int *)malloc(num_files * sizeof(int));

  double t0, t1;                                                                                    /* time limits */

  for (i = 0; i < N; i++)
    cons[i] = i;

  srandom((unsigned int)getpid());
  t0 = ((double)clock()) / CLOCKS_PER_SEC;

  /* generation of worker threads */

  for (i = 0; i < N; i++)
    if (pthread_create(&tIdCons[i], NULL, consumer, &cons[i]) != 0){                            /* thread consumer */
      perror("error on creating thread consumer");
      exit(EXIT_FAILURE);
    }

  /* waiting for the termination of the intervening entities threads */
  for (i = 0; i < N; i++) {
    if (pthread_join(tIdCons[i], (void *)&status_p) != 0) {                                     /* thread consumer */
      perror("error on waiting for thread customer");
      exit(EXIT_FAILURE);
    }

    printf("thread consumer, with id %u, has terminated: ", i);
    printf("its status was %d\n", *status_p);
  }

  t1 = ((double)clock()) / CLOCKS_PER_SEC;

  /* call function to write final results */
  writeFinal();

  /* print time spent */
  printf("\nElapsed time = %.6f s\n", t1 - t0);

  exit(EXIT_SUCCESS);
}

/**
 *  \brief Function consumer.
 *
 *  Its role is to simulate the life cycle of a consumer.
 *
 *  \param par pointer to application defined consumer identification
 */

static void *consumer(void *par){

  /* consumer id */
  unsigned int id = *((unsigned int *)par);

  /* while there is files to read */
  while (file_available(id)) {

    /* while there is data to read from the current file */
    while (getVal(id)){
      usleep((unsigned int)floor(40.0 * random() / RAND_MAX + 1.5));
    }

    //usleep((unsigned int)floor(40.0 * random() / RAND_MAX + 1.5));
    
    /* saves results for each file in the variables */
    write_file_results(id);

    /* close current file */
    closeFile(id);
  }

  statusCons[id] = EXIT_SUCCESS;
  pthread_exit(&statusCons[id]);
}
