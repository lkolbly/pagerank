#include<stdlib.h>
#include<stdio.h>

#include "sparsematrix.h"

uint64_t smHash(sparseMatrix_t *m, uint64_t x, uint64_t y)
{
  return (m->h * x + y) % m->hash_capacity; // Just to pick something
}

sparseMatrix_t *smNew(uint64_t w, uint64_t h, double default_val)
{
  sparseMatrix_t *m = malloc(sizeof(sparseMatrix_t));
  m->w = w;
  m->h = h;
  m->default_val = default_val;
  m->hash_capacity = w + h; // Just to pick a value.
  m->hashroot = malloc(sizeof(hashnode_t*)*m->hash_capacity);
  int i;
  for (i=0; i<m->hash_capacity; i++) {
    m->hashroot[i] = NULL;
  }
  return m;
}

double smGet(sparseMatrix_t *m, uint64_t x, uint64_t y)
{
  uint64_t hashval = smHash(m, x, y);
  hashnode_t *root = m->hashroot[hashval];
  if (!root) {
    return m->default_val;
  }
  while (root) {
    if (root->x == x && root->y == y) {
      return root->elem;
    }
    root = root->next;
  }
  return m->default_val;
}

void smSet(sparseMatrix_t *m, uint64_t x, uint64_t y, double val)
{
  if (x > m->w || x > m->h) {
    printf("2: %lu,%lu,%f > %lu\n", x, y, val, m->h);
    exit(0);
  }
  uint64_t hashval = smHash(m, x, y);
  hashnode_t *root = m->hashroot[hashval];
  if (!root) {
    m->hashroot[hashval] = malloc(sizeof(hashnode_t));
    m->hashroot[hashval]->x = x;
    m->hashroot[hashval]->y = y;
    m->hashroot[hashval]->elem = val;
    m->hashroot[hashval]->next = NULL;
    return;
  }
  //printf("%p\n", root);
  if (x > m->w || x > m->h) {
    printf("%lu,%lu,%f > %lu\n", x, y, val, m->h);
    exit(0);
  }
  do {
    if (root->x == x && root->y == y) {
      root->elem = val;
      return;
    }
    if (root->next)
      root = root->next;
  } while (root->next);

  hashnode_t *n = malloc(sizeof(hashnode_t));
  n->x = x;
  n->y = y;
  n->elem = val;
  n->next = NULL;
  root->next = n;
}

void smClear(sparseMatrix_t *m)
{
  int i;
  for (i=0; i<m->hash_capacity; i++) {
    if (m->hashroot[i]) {
      hashnode_t *n = m->hashroot[i];
      while (n->next) {
	hashnode_t *next = n->next;
	free(n);
	n = next;
      }
      free(m->hashroot[i]);
    }
  }
}

typedef struct tstate_t {
  pthread_t pth;

  int start; // Inclusive
  int end; // Inclusive
  sparseMatrix_t *a;
  vector_t *v;
  vector_t *res;
} tstate_t;

void *smMultVec_func(void *arg)
{
  tstate_t *state = arg;
  int i;
  for (i=state->start; i<=state->end; i++) {
    hashnode_t *root = state->a->hashroot[i];
    while (root) {
      if (root->x < state->res->size) {
	state->res->elems[root->x] += (root->elem - state->a->default_val) * state->v->elems[root->y];
      }
      root = root->next;
      //printf("Multiply %lux%lu\n", root->x, root->y);
    }
  }
  return NULL;
}

#include<pthread.h>

// Returns new vector.
vector_t *smMultVec(sparseMatrix_t *a, vector_t *v)
{
  vector_t *res = malloc(sizeof(vector_t));
  res->size = a->h;
  res->elems = calloc(1, sizeof(double)*res->size);

  int nthreads = 12;
  tstate_t *tstates = malloc(sizeof(tstate_t)*nthreads);
  int i, j;
  //printf("Hash capacity is %i\n", a->hash_capacity);
  for (i=0; i<nthreads; i++) {
    tstates[i].a = a;
    tstates[i].v = v;
    tstates[i].res = malloc(sizeof(vector_t));
    tstates[i].res->size = a->h;
    tstates[i].res->elems = calloc(a->h, sizeof(double));

    tstates[i].start = a->hash_capacity / nthreads * i;
    if (i+1 == nthreads) {
      tstates[i].end = a->hash_capacity - 1;
    } else {
      tstates[i].end = a->hash_capacity / nthreads * (i+1) - 1;
    }

    //printf("Starting thread %i from %i to %i\n", i, tstates[i].start, tstates[i].end);
    pthread_create(&tstates[i].pth,NULL, smMultVec_func, &tstates[i]);
  }

  // Multiply v by the base while the threads are doing their thing
  double comp = 0.0;
  for (i=0; i<v->size; i++) {
    //res->elems[0] += a->default_val * v->elems[i];
    double y = a->default_val * v->elems[i] - comp;
    double t = res->elems[0] + y;
    comp = (t - res->elems[0]) - y;
    res->elems[0] = t;
  }
  for (i=1; i<res->size; i++) {
    res->elems[i] = res->elems[0];
  }

  // Close all the threads
  for (i=0; i<nthreads; i++) {
    pthread_join(tstates[i].pth, NULL);
  }

  // Add up each one
  for (i=0; i<nthreads; i++) {
    vector_t *pth_res = tstates[i].res;
    //printVec(pth_res);
    for (j=0; j<res->size; j++) {
      res->elems[j] += pth_res->elems[j];
    }
  }

  // Free everything
  for (i=0; i<nthreads; i++) {
    free(tstates[i].res->elems);
    free(tstates[i].res);
  }
  free(tstates);
  //printf("\n");

  return res;

#if 0
  int i;
  for (i=0; i<res->size; i++) {
    double sum = 0.0;
    int j;
    for (j=0; j<v->size; j++) {
      sum += v->elems[j] * smGet(a, i, j);
      //printf("%f * %f\n", v->elems[j], smGet(a,j,i));
    }
    res->elems[i] = sum;
  }

  return res;
#endif
}

void smPrintMatrix(sparseMatrix_t *m)
{
  int i,j;
  for (i=0; i<m->h; i++) {
    for (j=0; j<m->w; j++) {
      printf("%f,", smGet(m, i, j));
    }
    printf("\n");
  }
}

// Stores the result into c, returns c.
// Note: Untested
sparseMatrix_t *smMult(sparseMatrix_t *a, sparseMatrix_t *b, sparseMatrix_t *c)
{
  smClear(c);
  c->w = b->w;
  c->h = a->h;

  c->default_val = a->default_val * b->default_val * a->w;

  int x,y,i;
  for (x=0; x<c->w; x++) {
    for (y=0; y<c->h; y++) {
      // We need to decide if this cell is "special". That is, if both b's
      // column and a's row are entirely the default value, then this cell
      // can be rolled in as a default cell.
      int is_special = 0;
      double sum = 0.0;
      for (i=0; i<a->w; i++) {
	double v1 = smGet(a, i, y);
	double v2 = smGet(b, x, i);
	if (v1 != a->default_val || v2 != b->default_val) {
	  is_special = 1;
	}
	sum += v1 * v2;
      }
      if (is_special) {
	smSet(c, x, y, sum);
      }
    }
  }
  return c;
}
