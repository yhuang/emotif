/*
 * File:  emotif.c
 * Date:  March 1, 2002
 * Package:  eMOTIF-3.6
 * Programmer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 *
 * ------------------------------------------------------------
 * Copyright (c) 2002 
 * All Rights Reserved.
 * The Leland Stanford Junior University Board of Trustees
 */


#include <assert.h>
#include "emotif.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


/*
 * ----------
 * InitializeCharToBitMap
 * ==========
 *
 * ----------
 */

void InitializeCharToBitMap (unsigned long *charToBitMap, int mapLength)
{
  int i;
  char c;

  memset (charToBitMap, 0, mapLength * sizeof (unsigned long));

  /*
   * ----------
   * The following for loop will turn on bit 0 for the unsigned long variable
   * map['A'] (The ASCII character 'A' has a corresponding integer value
   * of 65.), bit 1 for the unsigned long variable map['B'] (The ASCII
   * character 'B' has a corresponding integer value of 66.), bit 2 for
   * the unsigned long variable map['C'] (The ASCII character 'C' has a
   * corresponding integer value of 67.), all the way to bit 25 for the
   * unsigned long variable map['Z'] (The ASCII character 'a' has a
   * corresponding integer value of 90.).
   * ----------
   */

  for (c = 'A', i = 0; c <= 'Z'; ++c, ++i) {
    charToBitMap[(int) c] = 1 << i;
  }  /* for (c = 'A', i = 0; c <= 'Z'; ++c, ++i) */


  /*
   * ----------
   * The following for loop will turn on bit 0 for the unsigned long variable
   * charToBitMap['a'] (The ASCII character 'a' has a corresponding integer
   * value of 97.), bit 1 for the unsigned long variable map['b'] (The ASCII
   * character 'b' has a corresponding integer value of 98.), bit 2 for
   * the unsigned long variable map['c'] (The ASCII character 'c' has a 
   * corresponding integer value of 98.), all the way to bit 25 for the
   * integer var map['z'] (The ASCII character 'z' has a
   * corresponding integer value of 122.).
   * ----------
   */

  for (c = 'a', i = 0; c <= 'z'; ++c, ++i) {
    charToBitMap[(int) c] = 1 << i;
  }  /* for (c = 'a', i = 0; c <= 'z'; ++c, ++i) */
}


/*
 * ----------
 * InitializeCharToPercentageMap
 * ==========
 * 
 * ----------
 */

void InitializeCharToPercentageMap (double *charToPercentageMap)
{
  memset (charToPercentageMap, 0, ('z' + 1) * sizeof (double));

  charToPercentageMap['W'] = charToPercentageMap['w'] = 0.0123;
  charToPercentageMap['C'] = charToPercentageMap['c'] = 0.0162;
  charToPercentageMap['H'] = charToPercentageMap['h'] = 0.0224;
  charToPercentageMap['M'] = charToPercentageMap['m'] = 0.0239;
  charToPercentageMap['Y'] = charToPercentageMap['y'] = 0.0320;
  charToPercentageMap['Q'] = charToPercentageMap['q'] = 0.0389;
  charToPercentageMap['F'] = charToPercentageMap['f'] = 0.0412;
  charToPercentageMap['N'] = charToPercentageMap['n'] = 0.0440;
  charToPercentageMap['P'] = charToPercentageMap['p'] = 0.0485;
  charToPercentageMap['R'] = charToPercentageMap['r'] = 0.0520;
  charToPercentageMap['D'] = charToPercentageMap['d'] = 0.0526;
  charToPercentageMap['T'] = charToPercentageMap['t'] = 0.0565;
  charToPercentageMap['I'] = charToPercentageMap['i'] = 0.0592;
  charToPercentageMap['K'] = charToPercentageMap['k'] = 0.0597;
  charToPercentageMap['E'] = charToPercentageMap['e'] = 0.0636;
  charToPercentageMap['V'] = charToPercentageMap['v'] = 0.0665;
  charToPercentageMap['G'] = charToPercentageMap['g'] = 0.0686;
  charToPercentageMap['S'] = charToPercentageMap['s'] = 0.0705;
  charToPercentageMap['A'] = charToPercentageMap['a'] = 0.0765;
  charToPercentageMap['L'] = charToPercentageMap['l'] = 0.0948;
}


/*
 * ----------
 * InitializeIndexToBitMap
 * ==========
 * 
 * ----------
 */

void InitializeIndexToBitMap (unsigned long *indexToBitMap, int mapLength)
{
  int i;

  memset (indexToBitMap, 0, mapLength * sizeof (unsigned long));  

  for (i = 0; i < mapLength; i++) {
    indexToBitMap[i] = 1 << i;
  }
}


/*
 * ----------
 * MapDoublesToBlock
 * ==========
 * 
 * ----------
 */

double* MapDoublesBlock (char *mappedFilename, unsigned long blockLength)
{
  int doublesBlock;
  double *mappedDoubles;

  if ((doublesBlock = open (mappedFilename, O_RDONLY, 0)) != -1) {
    mappedDoubles = (double*) mmap (NULL, blockLength * sizeof (double),
				    PROT_READ, MAP_SHARED, doublesBlock, 0);
    assert (mappedDoubles != NULL);
  } else {
    fprintf (stderr, "Fatal Error:  Memory mapping failed!\n ");
    exit (1);
  }

  close (doublesBlock);
  return (mappedDoubles);
}


/*
 * ----------
 * MapCharsToBlock
 * ==========
 * 
 * ----------
 */

char* MapCharsBlock (char *mappedFilename, unsigned long blockLength)
{
  int charsBlock;
  char *mappedChars;

  if ((charsBlock = open (mappedFilename, O_RDONLY, 0)) != -1) {
    mappedChars = (char*) mmap (NULL, blockLength * sizeof (char),
				PROT_READ, MAP_SHARED, charsBlock, 0);
    assert (mappedChars != NULL);
  } else {
    fprintf (stderr, "Fatal Error:  Memory mapping failed!\n ");
    exit (1);
  }

  close (charsBlock);
  return (mappedChars);
}


/*
 * ----------
 * MapUnsignedLongsToBlock
 * ==========
 * 
 * ----------
 */

unsigned long* MapUnsignedLongsBlock (char *mappedFilename,
				      unsigned long blockLength)
{
  int unsignedLongsBlock;
  unsigned long *mappedUnsignedLongs;

  if ((unsignedLongsBlock = open (mappedFilename, O_RDONLY, 0)) != -1) {
    mappedUnsignedLongs = (unsigned long*) mmap (NULL, blockLength *
						 sizeof (unsigned long),
						 PROT_READ, MAP_SHARED,
						 unsignedLongsBlock, 0);
    assert (mappedUnsignedLongs != NULL);
  } else {
    fprintf (stderr, "Fatal Error:  Memory mapping failed!\n ");
    exit (1);
  }

  close (unsignedLongsBlock);
  return (mappedUnsignedLongs);
}


/*
 * ----------
 * WriteCharsBlockToBinaryFile
 * ==========
 * 
 * ----------
 */

void WriteCharsBlockToBinaryFile (DArray charsArray, char *outfilename)
{
  unsigned long i, charsArrayLength;
  char *charsBlock;
  FILE *outfile;

  outfile = fopen (outfilename, "w");

  charsArrayLength = ArrayLength (charsArray);

  charsBlock = (char*) malloc (sizeof (char) * charsArrayLength);
  assert (charsBlock != NULL);

  for (i = 0; i < charsArrayLength; i++) {
    charsBlock[i] = *(char*) ArrayNth (charsArray, i);
  }

  fwrite (charsBlock, sizeof (char), charsArrayLength, outfile);

  fclose (outfile);
  free (charsBlock);
}


/*
 * ----------
 * WriteDoublesBlockToBinaryFile
 * ==========
 * 
 * ----------
 */

void WriteDoublesBlockToBinaryFile (DArray doublesArray, char *outfilename)
{
  unsigned long i, doublesArrayLength;
  double *doublesBlock;
  FILE *outfile;

  outfile = fopen (outfilename, "w");

  doublesArrayLength = ArrayLength (doublesArray);

  doublesBlock = (double*) malloc (sizeof (double) * doublesArrayLength);
  assert (doublesBlock != NULL);

  for (i = 0; i < doublesArrayLength; i++) {
    doublesBlock[i] = *(double*) ArrayNth (doublesArray, i);
  }

  fwrite (doublesBlock, sizeof (double), doublesArrayLength, outfile);

  fclose (outfile);
  free (doublesBlock);
}


/*
 * ----------
 * WriteUnsignedLongsBlockToBinaryFile
 * ==========
 * 
 * ----------
 */

void WriteUnsignedLongsBlockToBinaryFile (DArray unsignedLongsArray,
					  char *outfilename)
{
  unsigned long i, unsignedLongsArrayLength;
  unsigned long *unsignedLongsBlock;
  FILE *outfile;

  outfile = fopen (outfilename, "w");

  unsignedLongsArrayLength = ArrayLength (unsignedLongsArray);

  unsignedLongsBlock = (unsigned long*)
    malloc (sizeof (unsigned long) * unsignedLongsArrayLength);
  assert (unsignedLongsBlock != NULL);

  for (i = 0; i < unsignedLongsArrayLength; i++) {
    unsignedLongsBlock[i] =
      *(unsigned long*) ArrayNth (unsignedLongsArray, i);
  }

  fwrite (unsignedLongsBlock, sizeof (unsigned long),
	  unsignedLongsArrayLength, outfile);

  fclose (outfile);
  free (unsignedLongsBlock);
}

