#ifndef PAGERANK_H
#define PAGERANK_H

#include<stdint.h>

// Note: linebuf size and linkbuf size have to be big enough.
#define LINEBUF_SIZE 100000000

typedef struct prNode_t {
  int nlinks;
  uint64_t *links;
} prNode_t;

int *pagerank(prNode_t *nodes, int num_nodes);

#endif
