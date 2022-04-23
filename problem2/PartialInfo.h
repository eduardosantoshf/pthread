#ifndef PARTIAL_INFO_H_
#define PARTIAL_INFO_H_

struct PartialInfo{
    int file_id;
    int matrix_id;
    int order;
    double ** matrix;
    double det;
};

typedef struct PartialInfo PartialInfo;

#endif /* PARTIAL_INFO_H_ */