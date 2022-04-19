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

#ifndef SHARED_REGION_H_
#define SHARED_REGION_H_

void storeFileNames(int filesNumber, char * fileNames[]);

void openNextFile();

int getVal(int threadID, int fileID, int order, double ** matrix);

void savePartialResults(int threadID, int fileID, int matrixNumber, double det);

void storeResults();

void checkProcessingResults();

#endif /* SHARED_REGION_H_ */