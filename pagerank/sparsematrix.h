#ifndef SPARSEMATRIX_H
#define SPARSEMATRIX_H

#include<stdint.h>

typedef struct vector_t {
  int size;
  double *elems;
} vector_t;

typedef struct hashnode_t {
  uint64_t x,y;
  double elem;

  struct hashnode_t *next;
} hashnode_t;

typedef struct sparseMatrix_t {
  uint64_t w, h;
  double default_val;

  int hash_capacity;
  hashnode_t **hashroot;
} sparseMatrix_t;

sparseMatrix_t *smNew(uint64_t w, uint64_t h, double default_val);
void smSet(sparseMatrix_t *m, uint64_t x, uint64_t y, double val);
double smGet(sparseMatrix_t *m, uint64_t x, uint64_t y);
uint64_t smHash(sparseMatrix_t *m, uint64_t x, uint64_t y);
vector_t *smMultVec(sparseMatrix_t *a, vector_t *v);
void smPrintMatrix(sparseMatrix_t *m);

#endif
