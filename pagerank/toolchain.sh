#!/bin/bash

# Compile everything
gcc -O2 parse_xml.c -lexpat -o parser
gcc -O2 sparsematrix.c pagerank.c main.c -lpthread -lm -o pagerank

# Parse
# Files in: ../Downloads/enwiki.xml
# Files out: name-reference, network
./parser

# Find Eigenvalue
# Files in: network
# Files out: vector-out
./pagerank

# Sort & name the vector
# Files in: vector-out
# Files out: pageranked
python namevector.py | python sortnamedvector.py > pageranked

# Merge the named vector with the actual (empirical) page counts
python mkcsv.py
