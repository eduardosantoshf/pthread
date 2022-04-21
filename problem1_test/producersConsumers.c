/**
 *  \file producersConsumers.c (implementation file)
 *
 *  \brief Problem name: Producers / Consumers.
 *
 *  Synchronization based on monitors.
 *  Both threads and the monitor are implemented using the pthread library which enables the creation of a
 *  monitor of the Lampson / Redell type.
 *
 *  Generator thread of the intervening entities.
 *
 *  \author António Rui Borges - March 2019
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
#include "fifo.h"

FILE *fp;
int size_of_filenames;
int num_of_bytes;


/** \brief consumer threads return status array */
int statusProd[N];
int statusCons[N];

/** \brief producer life cycle routine */
static void *producer (void *id);

/** \brief consumer life cycle routine */
static void *consumer (void *id);

char **filenames;


static void printUsage (char *cmdName) {
  fprintf (stderr, "\nSynopsis: %s OPTIONS [filename / positive number]\n"
           "  OPTIONS:\n"
           "  -h      --- print this help\n"
           "  -f      --- filename\n"
           "  -n      --- positive number\n", cmdName);
}



static int processCommandLine(int argc, char *argv[]){

  int opt;                  /* selected option */
  char *fName = "no name";  /* file name (initialized to "no name" by default) */
  int value_opt = -1;             /* numeric value (initialized to -1 by default) */

  /* Process command line optios */
  do
    { 
      switch ((opt = getopt (argc, argv, "f:n:h"))) { 

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
                    value_opt = (int) atoi (optarg);
                    break;

        case 'h': /* help mode */
                    printUsage (basename (argv[0]));
                    return EXIT_SUCCESS;

        case '?': /* invalid option */
                    fprintf (stderr, "%s: invalid option\n", basename (argv[0]));
                printUsage (basename (argv[0]));
                    return EXIT_FAILURE;
                  /* break switch */  
        case -1:  break;
        }
    } while (opt != -1);

    if (argc == 1){ 
        fprintf (stderr, "%s: invalid format\n", basename (argv[0]));
        printUsage (basename (argv[0]));
        return EXIT_FAILURE;
    }
    
    /* Store filenames in array */
    for(int i = 2; i < argc; i++){
      filenames[i - 2] = argv[i]; 
    }

    size_of_filenames = argc - 2;

    return EXIT_SUCCESS;

}


/**
 *  \brief Main thread.
 *
 *  Its role is starting the simulation by generating the intervening entities threads (producers and consumers) and
 *  waiting for their termination.
 */
int main (int argc, char* argv[])
{

  printf("ola");
  pthread_t tIdProd[N],                                                        /* producers internal thread id array */
            tIdCons[N];                                                        /* consumers internal thread id array */
  unsigned int prod[N],                                             /* producers application defined thread id array */
               cons[N];                                           /* producers application defined thread id array */

  filenames = malloc((argc - 1) * sizeof(char*));
                                                            /* consumers application defined thread id array */
  int i;                                                                                        /* counting variable */
  int *status_p;                                                                      /* pointer to execution status */

  /* initializing the application defined thread id arrays for the producers and the consumers and the random number
     generator */

  

  int proccess_command = processCommandLine(argc, argv);

  if(proccess_command == EXIT_FAILURE){
    return EXIT_FAILURE;
  }

  

  double t0, t1;
  
  //statusCons[N] = EXIT_SUCCESS;                                                                                      /* time limits */

   for (i = 0; i < N; i++)
    prod[i] = i;

  for (i = 0; i < N; i++)
    cons[i] = i;

  srandom ((unsigned int) getpid ());
  t0 = ((double) clock ()) / CLOCKS_PER_SEC;

  for (i = 0; i < N; i++)
    if (pthread_create (&tIdProd[i], NULL, producer, &prod[i]) != 0)                              /* thread producer */
       { perror ("error on creating thread producer");
         exit (EXIT_FAILURE);
       }
  for (i = 0; i < N; i++)
    if (pthread_create (&tIdCons[i], NULL, consumer, &cons[i]) != 0)                             /* thread consumer */
       { perror ("error on creating thread consumer");
         exit (EXIT_FAILURE);
       }

  /* waiting for the termination of the intervening entities threads */
  for (i = 0; i < N; i++)
  { if (pthread_join (tIdProd[i], (void *) &status_p) != 0)                                       /* thread producer */
       { perror ("error on waiting for thread producer");
         exit (EXIT_FAILURE);
       }
    printf ("thread producer, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *status_p);
  }
  for (i = 0; i < N; i++)
  { if (pthread_join (tIdCons[i], (void *) &status_p) != 0)                                       /* thread consumer */
       { perror ("error on waiting for thread customer");
         exit (EXIT_FAILURE);
       }
    printf ("thread consumer, with id %u, has terminated: ", i);
    printf ("its status was %d\n", *status_p);
  }

  t1 = ((double) clock ()) / CLOCKS_PER_SEC;
  printf ("\nElapsed time = %.6f s\n", t1 - t0);

  exit (EXIT_SUCCESS);
}




/**
 *  \brief Function consumer.
 *
 *  Its role is to simulate the life cycle of a consumer.
 *
 *  \param par pointer to application defined consumer identification
 */

//static void *consumer (void *par)
//{
//  unsigned int id = *((unsigned int *) par),                                                          /* consumer id */
//               val;                                                                                /* produced value */
//  int i;                                                                                        /* counting variable */
//
//  for (i = 0; i < M; i++)
//  { usleep((unsigned int) floor (40.0 * random () / RAND_MAX + 1.5));                           /* do something else */
//    val = getVal (id);                                                                           /* retrieve a value */
//    //printf ("The value %u was produced by the thread P%u and consumed by the thread C%u.\n",      /* consume a value */
//    //        val % 1000, val / 1000, id);
//    
//    //printf("%d \n\n", id);
//  }
//  printf("%d \n", val);
//  //printf("%d \n\n", id);
//
//  statusCons[id] = EXIT_SUCCESS;
//  pthread_exit (&statusCons[id]);
//}

static void *producer (void *par)
{
  unsigned int id = *((unsigned int *) par);                                                          /* producer id */
  int i;                                                                                        /* counting variable */

  for (i = 0; i < size_of_filenames; i++)
  { 

    fp = fopen(filenames[i], "r");

    do{
      
      putVal(id, fp, num_of_bytes);

    }while (!feof(fp));
    
    usleep((unsigned int) floor (40.0 * random () / RAND_MAX + 1.5));                           /* do something else */
  }

  statusProd[id] = EXIT_SUCCESS;
  pthread_exit (&statusProd[id]);
}

static void *consumer(void *par) {
  unsigned int id = *((unsigned int *) par);

  int i; 
  int value;                                                                                       /* counting variable */

  for (i = 0; i < size_of_filenames; i++)
  { usleep((unsigned int) floor (40.0 * random () / RAND_MAX + 1.5));                           /* do something else */
    value = getVal (id);                                                                           /* retrieve a value */
  }

  printf("%d \n", value);

  statusCons[id] = EXIT_SUCCESS;
  pthread_exit (&statusCons[id]);

}



