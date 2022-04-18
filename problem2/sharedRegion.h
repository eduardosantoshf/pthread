/**
 *  \file sharedRegion.h (interface file)
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

#ifndef SHARED_REGION_H
#define SHARED_REGION_H

extern void presentFileNames (char* files[], unsigned int nFiles);

extern void savePartialResults(struct CONTROLINFO *controlInfo);

extern int getPieceOfData(unsigned int workerId, struct CONTROLINFO *controlInfo);

extern int printResults();

#endif /* SHARED_REGION_H */