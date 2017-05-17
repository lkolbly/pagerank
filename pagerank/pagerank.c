#include<stdlib.h>
#include<stdio.h>
#include<math.h>
#include "sparsematrix.h"
#include "pagerank.h"

#define RANDOM_JUMP_CHANCE (0.15)

void printVec(vector_t *v)
{
  int i;
  for (i=0; i<v->size; i++) {
    printf("%.12e,", v->elems[i]);
  }
  printf("\n");
}

void vectorToFile(const char *fname, vector_t *v)
{
  FILE *f = fopen(fname,"w");
  if (!f) {
    fprintf(stderr, "Couldn't open %s for writing\n", fname);
    return;
  }
  int i;
  for (i=0; i<v->size; i++) {
    fprintf(f, "%.12e\n", v->elems[i]);
  }
  fclose(f);
}

// Returns a sorted list of pointers to pageranked nodes.
int *pagerank(prNode_t *nodes, int num_nodes)
{
  double default_value = RANDOM_JUMP_CHANCE / (double)num_nodes;
  printf("Initializing %ix%i sparse matrix\n", num_nodes, num_nodes);
  sparseMatrix_t *m = smNew(num_nodes, num_nodes, default_value);
  int i;
  for (i=0; i<num_nodes; i++) {
    if (i%500000 == 0)
      printf("Node %i has %i links\n", i, nodes[i].nlinks);
    if (nodes[i].nlinks == 0) {
      // Hrm, special case here. Set every value in column to 1.0 / nnodes
      // TODO: Make it less inefficient
      int j;
      for (j=0; j<num_nodes; j++) {
	//smSet(m, j, i, 1.0 / (double)num_nodes);
      }
    } else {
      // The normal case.
      uint64_t j;
      double linkval = (1.0-RANDOM_JUMP_CHANCE) / (double)nodes[i].nlinks + default_value;
      for (j=0; j<nodes[i].nlinks; j++) {
	/*if (nodes[i].links[j] > 100000000) {
	  printf("i=%lu j=%lu val=%f\n", i, j, linkval);
	  }*/
	smSet(m, nodes[i].links[j], i, linkval);
      }
    }
  }

#if 0 // Enable for debugging on small matrices
  smPrintMatrix(m);

  for (i=0; i<m->hash_capacity; i++) {
    printf("Bucket %lu:\n", i);
    hashnode_t *root = m->hashroot[i];
    while (root) {
      printf("%lu,%lu: %f\n", root->x, root->y, root->elem);
      root = root->next;
    }
  }
#endif

  // Generate a vector
  vector_t *v = malloc(sizeof(vector_t));
  v->size = num_nodes;
  v->elems = malloc(sizeof(double)*v->size);
  for (i=0; i<v->size; i++) {
    v->elems[i] = 0.1;
  }
  //printf("\n");

  // Multiply vector by matrix. Repeat.
  for (i=0; i<1000; i++) {
    vector_t *v2 = smMultVec(m, v);

    if (i%5 == 0) {
      // We implement a Kahan summation algorithm
      double dotp = 0.0; double c_dotp = 0.0;
      double l1 = 0.0;   double c_l1 = 0.0;
      double l2 = 0.0;   double c_l2 = 0.0;
      double resl = 0.0; double c_resl = 0.0; // Length of residual
      int j;
      for (j=0; j<v2->size; j++) {
	//dotp += v2->elems[j] * v->elems[j];
	double y = v2->elems[j] * v->elems[j] - c_dotp;
	double t = dotp + y;
	c_dotp = (t - dotp) - y;
	dotp = t;

	//l1 += v->elems[j]*v->elems[j];
	y = v->elems[j]*v->elems[j] - c_l1;
	t = l1 + y;
	c_l1 = (t - l1) - y;
	l1 = t;

	//l2 += v2->elems[j]*v2->elems[j];
	y = v2->elems[j]*v2->elems[j] - c_l2;
	t = l2 + y;
	c_l2 = (t - l2) - y;
	l2 = t;

	//resl += (v2->elems[j]-v->elems[j])*(v2->elems[j]-v->elems[j])
	y = v2->elems[j] - v->elems[j];
	y = y*y;
	t = resl + y;
	c_resl = (t - resl) - y;
	resl = t;
      }
      l1 = sqrt(l1);
      l2 = sqrt(l2);
      resl = sqrt(resl);
      printf("Did %i multiplies, dTheta = %.10f |v'-v|/sqrt(|v||v'|)=%.10f\n",
	     i, acos(dotp / (l1*l2)), resl/sqrt(l1*l2));
    }

    free(v->elems);
    free(v);
    v = v2;
    if (i%100 == 0) {
      // Rescale vector
#if 1
      double max = 0.0;
      int j;
      for (j=0; j<v->size; j++)
	if (v->elems[j] > max) max = v->elems[j];
      for (j=0; j<v->size; j++)
	v->elems[j] = v->elems[j] / max;
#endif

      //printVec(v);
      vectorToFile("vector-out", v);
    }
  }

  vectorToFile("vector-out", v);

  return NULL;
}
