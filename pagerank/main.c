#include<stdlib.h>
#include<stdio.h>
#include<string.h>

#include "pagerank.h"

prNode_t *loadPRFile(const char *filename, int *num_nodes)
{
  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open file %s\n", filename);
    return NULL;
  }

  // The first line has the number of nodes
  char buf[256];
  char *retval = fgets(buf, 256, f);
  if (!retval) {
    printf("Error reading file! (256 bytes)\n");
    return NULL;
  }
  *num_nodes = atoi(buf);
  printf("Reading %i nodes from file\n", *num_nodes);

  prNode_t *res = malloc(sizeof(prNode_t)*(*num_nodes));

  // For each one of those lines, each line contains the links to which we go
  int nlinks = 0;
  char *linebuf = malloc(LINEBUF_SIZE);
  uint64_t *linkbuf = malloc(sizeof(uint64_t)*LINEBUF_SIZE/2);
  int i;
  for (i=0; i<*num_nodes; i++) {
    nlinks = 0;
    retval = fgets(linebuf, LINEBUF_SIZE, f);
    if (!retval) {
      printf("Error reading file!\n");
    }

    char *p = linebuf;
    char *p_last = p;
    while (p != NULL) {
      strsep(&p, " "); // Insert a NULL at the next space
      //printf("%s\n", p_last);
      if (p_last[0] == '\n') {
	p_last = p;
	continue;
      }
      linkbuf[nlinks++] = atoi(p_last) - 1;
      /*if (linkbuf[nlinks-1] > 100000000) {
	printf("Error: '%s' on %i\n", p_last, i);
	exit(0);
      }*/
      //printf("%i,", atoi(p_last));
      p_last = p;
    }
    //printf("\n");

    // Copy the linkbuf into the prNode_t
    res[i].links = malloc(sizeof(int)*nlinks);
    memcpy(res[i].links, linkbuf, sizeof(int)*nlinks);
    res[i].nlinks = nlinks;
  }
  free(linebuf);
  free(linkbuf);

  fclose(f);
  return res;
}

int main(int argc, char **argv)
{
  int nnodes = 0;
  //prNode_t *nodes = loadPRFile("testdata", &nnodes);
  prNode_t *nodes = loadPRFile("network", &nnodes);

  pagerank(nodes, nnodes);
  return 0;
}
