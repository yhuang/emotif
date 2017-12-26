/*
 * File:  emotif.h
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


#ifndef _EMOTIF_H
#define _EMOTIF_H


#include "darray.h"


#define FILENAME_LENGTH 100
#define DEFAULT_CUTOFF_LOG_EXPECTATION -2
#define DESCRIPTION_LENGTH 2500
#define ID_LENGTH 20
#define MAX_NUM_BITS 10000000
#define MAX_NUM_CHARS MAX_NUM_BITS
#define MAX_NUM_EMOTIFS 1000000
#define MAX_NUM_SEQUENCES 100000
#define MOTIF_LENGTH 5000
#define NUM_LETTERS 26
#define RECORD_LENGTH 10000
#define SENSITIVITY_LENGTH 4
#define SEQUENCE_LENGTH 50000
#define STATS_LENGTH 10
#define LOG_SPECIFICITY_LENGTH 20
#define WILDCARD_BIT_PATTERN 33554431UL
#define WORST_RANK 1.0


typedef struct motifMappedMemoryCDT {
  double *logSpecificities;
  double *specificities;
  double *lengthCorrections;
  unsigned long *bitMotifs;
  unsigned long *bitIndices;
  unsigned long *homingBitLetters;
  unsigned long *offsets;
  char *records;
  unsigned long *recordIndices;
} *motifMappedMemoryADT;

typedef struct sequenceMappedMemoryCDT {
  char *names;
  unsigned long *nameIndices;
  char *textSequences;
  unsigned long *bitSequences;
  unsigned long *sequenceIndices;
  unsigned long *offsets;
  unsigned long *offsetIndices;
} *sequenceMappedMemoryADT;

typedef struct motifMemoryBlockCDT {
  DArray logSpecificitiesArray;
  DArray specificitiesArray;
  DArray lengthCorrectionsArray;
  DArray bitMotifsArray;
  DArray bitIndicesArray;
  DArray homingBitLettersArray;
  DArray offsetsArray;
  DArray recordsArray;
  DArray recordIndicesArray;
} *motifMemoryBlockADT;

typedef struct sequenceMemoryBlockCDT {
  DArray namesArray;
  DArray nameIndicesArray;
  DArray textSequencesArray;
  DArray bitSequencesArray;
  DArray sequenceIndicesArray;
  DArray offsetsArray;
  DArray offsetIndicesArray;
} *sequenceMemoryBlockADT;

typedef struct sequenceOffsetMapCDT {
  DArray map[NUM_LETTERS];
} *sequenceOffsetMapADT;


void InitializeCharToBitMap (unsigned long *charToBitMap, int mapLength);

void InitializeCharToPercentageMap (double *charToPercentageMap);

void InitializeIndexToBitMap (unsigned long *indexToBitMap, int mapLength);

char* MapCharsBlock (char *mappedFilename, unsigned long blockLength);

double* MapDoublesBlock (char *mappedFilename, unsigned long blockLength);

unsigned long* MapUnsignedLongsBlock (char *mappedFilename,
				      unsigned long blockLength);

void WriteCharsBlockToBinaryFile (DArray charsArray, char *outfilename);

void WriteDoublesBlockToBinaryFile (DArray doublesArray, char *outfilename);

void WriteUnsignedLongsBlockToBinaryFile (DArray unsignedLongsArray,
					  char *outfilename);

#endif

