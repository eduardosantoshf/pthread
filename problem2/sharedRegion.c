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

/** \brief number of file that will be processed */
int numFiles;

/** \brief worker threads return status array */
extern int statusWorkers[N];

/** \brief locking flag which warrants mutual exclusion inside the monitor */
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;