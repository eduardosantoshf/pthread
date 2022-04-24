#ifndef PARTIAL_INFO_H_
#define PARTIAL_INFO_H_

/**
 *  \file PartialInfo.h (struct file)
 *
 *  \brief Problem name: Determinant of a square matrix
 *
 *  Struct to be used on the shared region with data for each matrix.
 *
 *  \author Eduardo Santos and Pedro Bastos - April 2022
 * 
 */
struct PartialInfo{
    int file_id;
    int matrix_id;
    int order;
    double ** matrix;
    double det;
};

typedef struct PartialInfo PartialInfo;

#endif /* PARTIAL_INFO_H_ */