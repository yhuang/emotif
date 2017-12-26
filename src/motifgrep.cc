/*
 * File:  motifgrep.cc
 * Date:  March 1, 2002
 * Package:  eMOTIF-3.6
 * Programmer:  Craig G. Nevill-Manning (http://craig.nevill-manning.com) 
 * Comment Writer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 *
 * ------------------------------------------------------------
 * Copyright (c) 2002 
 * All Rights Reserved.
 * The Leland Stanford Junior University Board of Trustees
 */


#include <ctype.h>
#include <fstream.h>
#include <iostream.h>

main(int argc, char **argv) {
  char *m = argv[1];
  int p[2500];
  int pi = 0;
  int position;
  int i = 0, j = 0;
  char mm[1000][256];


  for (i = 0; m[i]; i++) {
    m[i] = tolower (m[i]);
    mm[i][j] = 0;
  }

  // the array p[i] records a sequence of positions to look in
  // to determine whether the motif matches in this alignment of the
  // motif with the sequence 
  // In other words, p[] records the order of positions to visit, from most
  // to least specific 

  // for K[EQ]T.L, p would contain 0, 2, 4, 1

  // the array mm[i][a] says whether amino acid 'a' is allowable in position
  // p[i] 
  
  // get specific positions; those that match only a single amino acid
  // i.e. not a period, and not [ILV]

  position = 0;

  for (i = 0; m[i]; i++) {
    if (isalpha(m[i])) { // this position is alphabetic
      mm[pi][m[i]] = 1;
      mm[pi][toupper(m[i])] = 1;
      p[pi++] = position;
      cout << position << ' ' << m[i] << endl;
    }

    if (m[i] == '[') {
      while (m[i] != ']') {
	i++; // ignore substitution groups
      }
    }

    position++;
  }

  // p[] is now 0, 2, 4
  
  // get group positions: things in square brackets

  position = 0;
  for (i = 0; m[i]; i++) {
    if (m[i] == '[') {
      while (m[++i] != ']') {
        mm[pi][m[i]] = 1;
        mm[pi][toupper(m[i])] = 1;
      }
      p[pi++] = position;
    }
    position++;
  }
  
  // p[] is now 0, 2, 4, 1
 
  char line[100000];
  
  while (!cin.eof()) {
    cin.getline(line, 100000);
    
    //    check that the line is at least as long as the pattern
    for (i = 0; i < position; i++) if (!line[i]) break;
    if (i < position) continue;
    
    for (i = 0; line[i + position - 1]; i++) {
      for (j = 0; j < pi; j++)
	if (!mm[j][line[i + p[j]]]) break;
      if (j == pi) cout << line << endl;
    }
  }
}

