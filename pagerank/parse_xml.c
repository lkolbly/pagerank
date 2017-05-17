#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<expat.h>
#include<regex.h>
#include<bzlib.h>
//#include<search.h>

typedef struct hashentry_t {
  int valid;
  unsigned int hash;
  int val;
} hashentry_t;

typedef struct hash_t {
  int num_entries;
  hashentry_t *entries;
} hash_t;

hash_t *hashCreate(int size)
{
  hash_t *h = malloc(sizeof(hash_t));
  h->num_entries = size;
  h->entries = malloc(sizeof(hashentry_t)*size);
  int i;
  for (i=0; i<size; i++) {
    h->entries[i].valid = 0;
  }
  return h;
}

void hashDestroy(hash_t *table)
{
  free(table->entries);
  free(table);
}

void hashInsert(hash_t *table, hashentry_t entry)
{
  unsigned int index = entry.hash % table->num_entries;
  while (table->entries[index].valid) {
    index = (index+1)%table->num_entries;
  }
  table->entries[index] = entry;
}

hashentry_t hashSearch(hash_t *table, unsigned int hash)
{
  unsigned int index = hash % table->num_entries;
  while (table->entries[index].hash != hash && table->entries[index].valid) {
    index = (index+1)%table->num_entries;
  }
  if (!table->entries[index].valid) {
    hashentry_t entry;
    entry.valid = 0;
    return entry;
  }
  return table->entries[index];
}

unsigned int hashString(const char *s)
{
  int len = strlen(s);
  int i;
  int hash = 0;
  for (i=0; i<len; i++) {
    hash = (hash * s[i]) ^ s[i];
  }
  return hash;
}

hashentry_t hashMakeEntry(const char *key, int value)
{
  hashentry_t entry;
  entry.valid = 1;
  entry.hash = hashString(key);
  entry.val = value;
  return entry;
}

void testHash()
{
  hash_t *table = hashCreate(256);
  hashInsert(table, hashMakeEntry("Lane", 1));
  int hash = hashString("Lane");
  printf("Hash for Lane is %i\n", hash);
  hashentry_t entry = hashSearch(table, hash);
  printf("Valid = %i\n", entry.valid);
  hashDestroy(table);
}

typedef struct page_t {
  char *title;
  char **unresolved_links;
  int nlinks;
  int *links;
} page_t;

struct parseState_t {
  /*int in_page;
    int in_title;*/

  page_t *current_page;

  int npages_alloced;
  int npages;
  page_t **pages;

  int buf_alloced;
  int buf_len;
  char *buf;

  int links_seen;
};

void dumpToFile(page_t **pages, int npages)
{
  // Dump the title/number mapping
  FILE *f = fopen("name-reference", "w");
  if (!f) {
    fprintf(stderr, "Couldn't open name-reference for writing\n");
    return;
  }
  int i;
  printf("Dumping name references\n");
  for (i=0; i<npages; i++) {
    fputs(pages[i]->title, f);
    fputc('\n', f);

    if (i%100000 == 0) {
      printf("Dumped %i names\n", i);
    }
  }
  fclose(f);

  // Dump the links
  int nlinks = 0;
  f = fopen("network", "w");
  if (!f) {
    fprintf(stderr, "Couldn't open network for writing\n");
    return;
  }
  printf("Dumping network\n");
  fprintf(f, "%i\n", npages);
  for (i=0; i<npages; i++) {
    int j;
    for (j=0; j<pages[i]->nlinks; j++) {
      fprintf(f, "%i ", pages[i]->links[j]+1);
      nlinks++;
    }
    fputc('\n', f);

    if (i%10000 == 0) {
      printf("Dumped %i nodes\n", i);
    }
  }
  fclose(f);

  printf("Total pages: %i Total links: %i\n", npages, nlinks);
}

hash_t *global_Hash_Table;

// Returns -1 if cannot be resolved
int resolveLink(page_t **pages, int npages, const char *link)
{
  // Process the link
  char *l2 = strdup(link);
  char *p = strchr(l2, '|');
  if (p != NULL) {
    p[0] = 0;
  }

  // Lookup the link in the hash
  unsigned int hash = hashString(link);
  hashentry_t entry = hashSearch(global_Hash_Table, hash);
  free(l2);
  if (entry.valid == 0) return -1;
  return entry.val;

#if 0
  //return -1;
  // Exact title matching, lookup in the global hashtable
  ENTRY entry;
  entry.key = link;
  ENTRY *e2 = hsearch(entry, FIND);
  if (e2 != NULL) {
    int res;
    memcpy(&res, e2->data, sizeof(int));
    return res;
  }
  return -1;

  // Exact title matching
  int i;
  for (i=0; i<npages; i++) {
    if (strcmp(pages[i]->title, link) == 0) {
      return i;
    }
  }
  return -1;
#endif
}

// Returns the number of links left unresolved
int resolvePageLinks(page_t **pages, int npages, page_t *page)
{
  int nulinks = 0;
  int ulinks_alloced = 20;
  char **unresolved_links = malloc(sizeof(char*)*20);

  int nlinks = 0;
  int links_alloced = 10;
  int *links = malloc(sizeof(int)*10);

  int i;
  for (i=0; page->unresolved_links[i]!=NULL; i++) {
    int link = resolveLink(pages, npages, page->unresolved_links[i]);
    if (link == -1) {
      if (nulinks+2 > ulinks_alloced) {
	ulinks_alloced *= 2;
	unresolved_links = realloc(unresolved_links, sizeof(char*)*ulinks_alloced);
      }
      unresolved_links[nulinks++] = page->unresolved_links[i];
    } else {
      if (nlinks+1 > links_alloced) {
	links_alloced *= 2;
	links = realloc(links, sizeof(int)*links_alloced);
      }
      free(page->unresolved_links[i]);
      links[nlinks++] = link;
    }
  }

  // Merge the links we found to the links we already had
  page->links = realloc(page->links, sizeof(int)*(page->nlinks + nlinks));
  memcpy(page->links+page->nlinks, links, sizeof(int)*nlinks);
  page->nlinks += nlinks;
  free(links);

  unresolved_links[nulinks] = NULL;
  free(page->unresolved_links);
  page->unresolved_links = unresolved_links;

  return nulinks;
}

int compare_ints(const int *a, const int *b)
{
  return *a > *b;
}

void dedupPageLinks(page_t *page)
{
  // Sort the links
  qsort(page->links, page->nlinks, sizeof(int), compare_ints);

  // Walk through the array, copying elements (offset) places forward
  // Increment offset whenever we see a duplicate.
  int offset = 0;
  int i;
  for (i=1; i<page->nlinks; i++) {
    if (page->links[i-1] == page->links[i]) {
      offset++;
    }
    page->links[i-offset] = page->links[i];
  }
}

int resolveAllLinks(page_t **pages, int npages)
{
  // Create the hash table used to quickly lookup titles
  //char *entry_data_buf = malloc(sizeof(int)*npages);
  //hcreate(npages*2);
  global_Hash_Table = hashCreate(npages*2);
  int i;
  for (i=0; i<npages; i++) {
    hashentry_t entry = hashMakeEntry(pages[i]->title, i);
    hashInsert(global_Hash_Table, entry);
#if 0
    ENTRY entry;
    entry.key = pages[i]->title;
    char *buf = entry_data_buf + i;
    memcpy(buf, &i, sizeof(int));
    entry.data = buf;
    hsearch(entry, ENTER);
#endif
  }

  int unresolved = 0;
  for (i=0; i<npages; i++) {
    unresolved += resolvePageLinks(pages, npages, pages[i]);
    if (i%10000 == 0) {
      printf("Resolved links for %i pages.\n", i);
    }
  }

  // Delete the hash table
  hashDestroy(global_Hash_Table);
  //hdestroy();
  //free(entry_data_buf);
  return unresolved;
}

regex_t *fetchLinks_Pattern;

int fetchLinks(page_t *page, const char *text)
{
  // Compile a regex pattern to do the matching
  //regex_t *pattern = malloc(sizeof(regex_t));
  //regcomp(pattern, "\\[\\[([^]\\[]*)\\]\\]", REG_EXTENDED);
  regex_t *pattern = fetchLinks_Pattern;

  int nlinks = 0;
  int links_alloced = 1;
  char **links = malloc(sizeof(char*));

  regmatch_t *matches = malloc(sizeof(regmatch_t)*8);

  // Check...
  const char *p = text;
  while (1) {
    int retval = regexec(pattern, p, 8, matches, REG_NOTBOL);
    if (retval == REG_NOMATCH) {
      //printf("%s\n", p);
      //free(matches);
      break;
    }

    if (nlinks+3 > links_alloced) {
      links_alloced *= 2;
      links = realloc(links, sizeof(char*)*links_alloced);
    }

    links[nlinks] = malloc(matches[1].rm_eo - matches[1].rm_so + 1);
    memcpy(links[nlinks], p + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
    links[nlinks][matches[1].rm_eo - matches[1].rm_so] = 0;
    //printf("%s\n", links[nlinks]);
    nlinks++;
    p += matches[0].rm_eo;
  }
  free(matches);

  links[nlinks++] = strdup("Main Page/"); // Hack to get around performance issue of having no links
  links[nlinks] = NULL;
  page->unresolved_links = links;
  //regfree(pattern);
  //free(pattern);
  //printf("Found %i links\n", nlinks);
  return nlinks;
}

void start(void *data, const char *el, const char **attr)
{
  struct parseState_t *state = (struct parseState_t*)data;

  if (!state->buf) {
    state->buf = malloc(1);
    state->buf_len = 0;
    state->buf_alloced = 1;

    state->pages = malloc(sizeof(page_t*));
    state->npages_alloced = 1;
    state->npages = 0;
  }

  state->buf_len = 0;
  /*if (strcmp(el, "page") == 0) {
    state->in_page = 1;
  } else if (strcmp(el, "title") == 0) {
    state->in_title = 1;
    }*/
}

void chardata(void *data, const char *text, int len)
{
  struct parseState_t *state = (struct parseState_t*)data;

  if (state->buf_len+len > state->buf_alloced) {
    state->buf = realloc(state->buf, 2*(state->buf_len+len));
    state->buf_alloced = 2*(state->buf_len+len);
  }

  memcpy(state->buf+state->buf_len, text, len);
  state->buf_len += len;
}

void end(void *data, const char *el)
{
  struct parseState_t *state = (struct parseState_t*)data;

  if (strcmp(el, "title") == 0) {
    page_t *p = calloc(1, sizeof(page_t));
    p->title = malloc(state->buf_len+1);
    strncpy(p->title, state->buf, state->buf_len+1);
    p->title[state->buf_len] = 0;

    if (state->npages+1 > state->npages_alloced) {
      state->npages_alloced *= 2;
      state->pages = realloc(state->pages, sizeof(page_t*)*state->npages_alloced);
    }

    if (state->npages%3000000 == 0) {
      printf("Resolving links partway through\n");
      int unresolved = resolveAllLinks(state->pages, state->npages);
      printf("%i links were unresolved\n", unresolved);
    }

    state->pages[state->npages++] = p;
    state->current_page = p;
    //printf("Found page with title %s\n", p->title);
    if (state->npages%1000 == 0) {
      printf("Found %i pages (seen %i links)\n", state->npages, state->links_seen);
    }
  } else if (strcmp(el, "text") == 0) {
    // Parse it for the links
    state->buf[state->buf_len] = 0;
    state->links_seen += fetchLinks(state->current_page, state->buf);
    //fetchLinks(state->current_page, "asdfasdfasdf[[fdsafdsa]]asdfasdf[[foobar]]asdf");
    //fetchLinks(state->current_page, "[[fdsafdsa]]");
  }
}

#define BUFSIZE 100000000

void readXML(const char *filename)
{
  fetchLinks_Pattern = malloc(sizeof(regex_t));
  regcomp(fetchLinks_Pattern, "\\[\\[([^]\\[]*)\\]\\]", REG_EXTENDED);

  XML_Parser xml = XML_ParserCreate(NULL);

  struct parseState_t userdata;
  memset(&userdata, 0, sizeof(struct parseState_t));
  userdata.buf = NULL;
  userdata.links_seen = 0;
  XML_SetUserData(xml, &userdata);
  XML_SetElementHandler(xml, start, end);
  XML_SetCharacterDataHandler(xml, chardata);

  FILE *f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Couldn't open %s\n", filename);
    return;
  }
  int bzerr;
  BZFILE *bzf = BZ2_bzReadOpen(&bzerr, f, 0, 0, NULL, 0);

  char *buf = malloc(BUFSIZE);
  while (1) {
    //int nread = fread(buf, 1, BUFSIZE, f);
    int nread = BZ2_bzRead(&bzerr, bzf, buf, BUFSIZE);

    int islast = 0;
    if (nread < BUFSIZE) islast = 1;
    XML_Parse(xml, buf, nread, islast);
    if (islast) {
      break;
    }
    //printf("Read %i bytes.\n", BUFSIZE);
  }
  free(buf);

  // Resolve the links
  printf("Finished, resolving links.\n");
  int unresolved = resolveAllLinks(userdata.pages, userdata.npages);
  printf("Total unresolved links: %i\n", unresolved);
  // Dedup all the resolved links
  int i;
  for (i=0; i<userdata.npages; i++) {
    dedupPageLinks(userdata.pages[i]);
  }
      /*int i;
  for (i=0; i<userdata.npages; i++) {
  resolvePageLinks(userdata.pages, userdata.npages, userdata.pages[i]);
  }*/

  BZ2_bzReadClose(&bzerr, bzf);
  fclose(f);

  regfree(fetchLinks_Pattern);
  free(fetchLinks_Pattern);

  dumpToFile(userdata.pages, userdata.npages);
}

#include <signal.h>

 void intHandler(int dummy) {
  exit(0);
}

 int main(int argc, char **argv)
 {
  signal(SIGINT, intHandler);
  //testHash();
  readXML("/home/lane/Downloads/enwiki-20141106-pages-articles.xml.bz2");
  return 0;
}
