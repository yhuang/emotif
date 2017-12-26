/*
 * File:  emotif-search.c
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
#include <ctype.h>
#include "emotif.h"
#include <math.h>
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>


/*
 * ----------
 * Global Variables
 * ----------
 */

static unsigned long numMotifs, numMotifBits, numMotifChars,
  numSequences, numSequenceNameChars, numSequenceBits;


/*
 * ----------
 * Function Prototypes
 * ----------
 */

static void BruteForceSearch (motifMappedMemoryADT motifMappedMemoryObject,
			      char *sequenceName, char *textSequence,
			      unsigned long *charToBitMap,
			      double cutoffExpectation, int isFasta);

static void InitializeMotifGlobalVariables (char *filename);

static void InitializeSequenceGlobalVariables (char *filename);


static void MapMotifBlocksToMemory (int useLocalPath,
                                    motifMappedMemoryADT
				    motifMappedMemoryObject,
				    char *motifDatabase);

static void MapSequenceBlocksToMemory (int useLocalPath,
                                       sequenceMappedMemoryADT
				       sequenceMappedMemoryObject,
				       char *sequenceDatabase);

static void RunSearchOnFastaSequences (Scanner scannerObject,
				       unsigned long *charToBitMap,
				       motifMappedMemoryADT
				       motifMappedMemoryObject,
				       double cutoffExpectation);

static void RunSearchOnSequence (Scanner scannerObject,
				 unsigned long *charToBitMap,
				 motifMappedMemoryADT motifMappedMemoryObject,
				 double cutoffExpectation);

static void SmartSearch (motifMappedMemoryADT motifMappedMemoryObject,
			 sequenceMappedMemoryADT sequenceMappedMemoryObject,
			 double cutoffExpectation);

static void UnmapMotifBlocksFromMemory (motifMappedMemoryADT
					motifMappedMemoryObject);

static void UnmapSequenceBlocksFromMemory (sequenceMappedMemoryADT
					sequenceMappedMemoryObject);


int main (int argc, char *argv[])
{
  Scanner scannerObject;
  motifMappedMemoryADT motifMappedMemoryObject;
  sequenceMappedMemoryADT sequenceMappedMemoryObject;
  char motifDatabase[FILENAME_LENGTH + 1],
    sequenceDatabase[FILENAME_LENGTH + 1];
  int opt, cutoffLogExpectation, searchByBruteForce, isFasta, useLocalPath;
  double cutoffExpectation;
  unsigned long charToBitMap['z' + 1];

  cutoffLogExpectation = DEFAULT_CUTOFF_LOG_EXPECTATION;
  cutoffExpectation = pow (10, (double) cutoffLogExpectation);
  searchByBruteForce = FALSE;
  isFasta = FALSE;
  useLocalPath = FALSE;

  strcpy (motifDatabase, "blocksplus");
  strcpy (sequenceDatabase, "sprot");

  while ((opt = getopt (argc, argv, "be:flm:s:")) != -1) {
    switch (opt) {
    case 'b':
      searchByBruteForce = TRUE;
      break;
    case 'e':
      cutoffLogExpectation = atoi (optarg);
      cutoffExpectation = pow (10, (double) cutoffLogExpectation);
      break;
    case 'f':
      isFasta = TRUE;
      break;
    case 'l':
      useLocalPath = TRUE;
      break;
    case 'm':
      strcpy (motifDatabase, optarg);
      break;
    case 's':
      strcpy (sequenceDatabase, optarg);
      break;
   }
  }

  motifMappedMemoryObject = (motifMappedMemoryADT)
    malloc (sizeof (struct motifMappedMemoryCDT));
  assert (motifMappedMemoryObject != NULL);

  MapMotifBlocksToMemory (useLocalPath, motifMappedMemoryObject,
                          motifDatabase);

  fprintf (stderr, "\n");

  if (!searchByBruteForce) {
    sequenceMappedMemoryObject = (sequenceMappedMemoryADT)
      malloc (sizeof (struct sequenceMappedMemoryCDT));
    assert (sequenceMappedMemoryObject != NULL);

    MapSequenceBlocksToMemory (useLocalPath, sequenceMappedMemoryObject,
                               sequenceDatabase);

    SmartSearch (motifMappedMemoryObject, sequenceMappedMemoryObject,
		 cutoffExpectation);

    UnmapSequenceBlocksFromMemory (sequenceMappedMemoryObject);
    free (sequenceMappedMemoryObject);
  } else {
    scannerObject = NewScannerFromFilename (sequenceDatabase, "\n", TRUE);
    assert (scannerObject != NULL);

    InitializeCharToBitMap (charToBitMap, 'z' + 1);

    if (isFasta) {
      RunSearchOnFastaSequences (scannerObject, charToBitMap,
				 motifMappedMemoryObject, cutoffExpectation);
    } else {
      RunSearchOnSequence (scannerObject, charToBitMap,
			   motifMappedMemoryObject, cutoffExpectation);
    }

    FreeScanner (scannerObject);
  }

  fprintf (stderr, "\n");

  UnmapMotifBlocksFromMemory (motifMappedMemoryObject);
  free (motifMappedMemoryObject);

  return (0);
}


static void SmartSearch (motifMappedMemoryADT motifMappedMemoryObject,
			 sequenceMappedMemoryADT sequenceMappedMemoryObject,
			 double cutoffExpectation)
{
  char sequenceSegment[MOTIF_LENGTH + 1], sequenceName[RECORD_LENGTH + 1],
    motifRecord[RECORD_LENGTH + 1];
  int foundMatches, letterIndex;
  double expectation, probabilityOfNoRandomHit;
  unsigned long sequenceIndex, sequenceLength, motifIndex, motifWidth,
    homingBitLetter, motifOffset, sequenceOffsetIndex, sequenceOffset,
    i, j, k, motifRecordIndex, motifRecordsBlockIndex,
    indexToBitMap[NUM_LETTERS];

  InitializeIndexToBitMap (indexToBitMap, NUM_LETTERS);

  for (sequenceIndex = 0; sequenceIndex < numSequences; sequenceIndex++) {
    expectation = 0.0;
    probabilityOfNoRandomHit = 1.0;
    foundMatches = FALSE;

    sequenceLength = sequenceMappedMemoryObject -> 
      sequenceIndices[sequenceIndex + 1] - sequenceMappedMemoryObject ->
      sequenceIndices[sequenceIndex];

    fprintf (stderr, "Processing Sequence %lu\r", sequenceIndex + 1);

    for (motifIndex = 0; motifIndex < numMotifs; motifIndex++) {
      motifWidth = motifMappedMemoryObject -> bitIndices[motifIndex + 1] - 
	motifMappedMemoryObject -> bitIndices[motifIndex];

      if (motifWidth <= sequenceLength) {
	expectation += sequenceLength * motifMappedMemoryObject ->
	  specificities[motifIndex] - motifMappedMemoryObject ->
	  lengthCorrections[motifIndex] + motifMappedMemoryObject ->
	  specificities[motifIndex];

	if (expectation > cutoffExpectation) {
	  break;
	}

	probabilityOfNoRandomHit *= pow (1 - motifMappedMemoryObject ->
	  specificities[motifIndex], sequenceLength - motifWidth + 1);

	homingBitLetter = motifMappedMemoryObject ->
	  homingBitLetters[motifIndex];
	motifOffset = motifMappedMemoryObject -> offsets[motifIndex];

	for (letterIndex = 0; letterIndex < NUM_LETTERS; letterIndex++) {
	  if (indexToBitMap[letterIndex] & homingBitLetter) {
	    for (sequenceOffsetIndex = sequenceMappedMemoryObject ->
		   offsetIndices[sequenceIndex * NUM_LETTERS + letterIndex]; 
		 sequenceOffsetIndex < sequenceMappedMemoryObject ->
		   offsetIndices[sequenceIndex * NUM_LETTERS + 
				  letterIndex + 1];
		 sequenceOffsetIndex++) {

	      sequenceOffset = sequenceMappedMemoryObject ->
	        offsets[sequenceOffsetIndex];

	      if (sequenceOffset < motifOffset) {
		continue;
	      }

	      if (sequenceOffset - motifOffset + motifWidth >
		  sequenceLength) {
		break;
	      }

	      i = sequenceMappedMemoryObject -> sequenceIndices[sequenceIndex]
		+ sequenceOffset - motifOffset;

	      for (j = 0; 
		   j < motifWidth && 
		     (sequenceMappedMemoryObject -> bitSequences[i + j] & 
		      motifMappedMemoryObject ->
		      bitMotifs[motifMappedMemoryObject ->
			       bitIndices[motifIndex] + j]);
		   j++) {
		sequenceSegment[j] = sequenceMappedMemoryObject ->
		  textSequences[i + j];
	      }
	      
	      if (j == motifWidth) {
		sequenceSegment[j] = '\0';
		
		if (!foundMatches) {
		  for (j = sequenceMappedMemoryObject -> 
			 nameIndices[sequenceIndex], k = 0;
		       j < sequenceMappedMemoryObject ->
			 nameIndices[sequenceIndex + 1];
		       j++, k++) {
		    
		    sequenceName[k] = sequenceMappedMemoryObject ->
		      names[j];
		  }
		  
		  sequenceName[k] = '\0';
		  printf (">%s\n", sequenceName);
		  foundMatches = TRUE;
		}
		
		for (motifRecordIndex = 0,
		       motifRecordsBlockIndex = motifMappedMemoryObject ->
		       recordIndices[motifIndex];
		     motifRecordsBlockIndex < motifMappedMemoryObject ->
		       recordIndices[motifIndex + 1];
		     motifRecordIndex++, motifRecordsBlockIndex++) {
		  motifRecord[motifRecordIndex] = motifMappedMemoryObject -> 
		    records[motifRecordsBlockIndex];
		}
		
		motifRecord[motifRecordIndex] = '\0';
		
		printf ("%.2e|%.3f|%.3e|%s|%s|%lu|%lu\n", expectation,
			probabilityOfNoRandomHit, motifMappedMemoryObject ->
			specificities[motifIndex], motifRecord,
			sequenceSegment, sequenceOffset - motifOffset + 1,
			sequenceOffset - motifOffset + motifWidth);
	      }
	    }
	  }
	}    
      }
    }
  }
}


/*
 * ----------
 * RunSearchOnFastaSequences
 * ==========
 * 
 * ----------
 */

static void RunSearchOnFastaSequences (Scanner scannerObject,
				       unsigned long *charToBitMap,
				       motifMappedMemoryADT
				       motifMappedMemoryObject,
				       double cutoffExpectation) 
{
  char sequenceName[RECORD_LENGTH + 1], textSequence[SEQUENCE_LENGTH + 1];
  unsigned long sequenceIndex;

  sequenceIndex = 0;

  while (ReadNextToken (scannerObject, sequenceName, RECORD_LENGTH + 1)) {
    fprintf (stderr, "Processing Sequence %lu\r", ++sequenceIndex);

    ReadNextToken (scannerObject, textSequence, SEQUENCE_LENGTH + 1);

    BruteForceSearch (motifMappedMemoryObject, sequenceName, textSequence,
		      charToBitMap, cutoffExpectation, TRUE);
  }
}


/*
 * ----------
 * RunSearchOnSequence
 * ==========
 * 
 * ----------
 */

static void RunSearchOnSequence (Scanner scannerObject,
				 unsigned long *charToBitMap,
				 motifMappedMemoryADT motifMappedMemoryObject,
				 double cutoffExpectation) 
{
  char textSequence[SEQUENCE_LENGTH + 1];

  ReadNextToken (scannerObject, textSequence, SEQUENCE_LENGTH + 1);

  BruteForceSearch (motifMappedMemoryObject, NULL, textSequence,
		    charToBitMap, cutoffExpectation, FALSE);
}


/*
 * ----------
 * BruteForceSearch
 * ==========
 * 
 * ----------
 */

static void BruteForceSearch (motifMappedMemoryADT motifMappedMemoryObject,
			      char *sequenceName, char *textSequence,
			      unsigned long *charToBitMap, 
			      double cutoffExpectation,
			      int isFasta)
{
  char sequenceSegment[MOTIF_LENGTH + 1], motifRecord[RECORD_LENGTH + 1];
  static int foundMatches;
  double expectation, probabilityOfNoRandomHit;
  unsigned long sequenceLength, motifWidth, motifIndex, i, j,
    motifRecordIndex, motifRecordsBlockIndex, bitSequence[SEQUENCE_LENGTH + 1];

  foundMatches = FALSE;
  sequenceLength = strlen (textSequence);
  expectation = 0.0;
  probabilityOfNoRandomHit = 1.0;

  for (i = 0; i < sequenceLength; i++) {
    bitSequence[i] = charToBitMap[(int) textSequence[i]];
  }
  
  for (motifIndex = 0; motifIndex < numMotifs; motifIndex++) {
    motifWidth = motifMappedMemoryObject -> bitIndices[motifIndex + 1] - 
      motifMappedMemoryObject -> bitIndices[motifIndex];
    
    if (motifWidth <= sequenceLength) {
      expectation += sequenceLength * motifMappedMemoryObject -> 
	specificities[motifIndex] - motifMappedMemoryObject ->
	lengthCorrections[motifIndex] + motifMappedMemoryObject ->
	specificities[motifIndex];
      
      if (expectation > cutoffExpectation) {
	break;
      }

      probabilityOfNoRandomHit *= pow (1 - motifMappedMemoryObject ->
	specificities[motifIndex], sequenceLength - motifWidth + 1);
      
      for (i = 0; i < sequenceLength - motifWidth + 1; i++) {
	for (j = 0;
	     j < motifWidth && 
	       (bitSequence[i + j] & 
		motifMappedMemoryObject -> 
		bitMotifs[motifMappedMemoryObject -> 
			 bitIndices[motifIndex] + j]);
	     j++);
	
	if (j == motifWidth) {
	  if (!foundMatches && isFasta) {
	    printf ("%s\n", sequenceName);
	    foundMatches = TRUE;
	  }

	  for (j = 0; j < motifWidth; j++) {
	    sequenceSegment[j] = textSequence[i + j];
	  }

	  sequenceSegment[j] = '\0';
	  
	  for (motifRecordIndex = 0,
		 motifRecordsBlockIndex = motifMappedMemoryObject ->
		 recordIndices[motifIndex];
	       motifRecordsBlockIndex < motifMappedMemoryObject ->
		 recordIndices[motifIndex + 1];
	       motifRecordIndex++, motifRecordsBlockIndex++) {
	    motifRecord[motifRecordIndex] = motifMappedMemoryObject -> 
	      records[motifRecordsBlockIndex];
	  }
	  
	  motifRecord[motifRecordIndex] = '\0';
	  
	  printf ("%.2e|%.3f|%.3e|%s|%s|%lu|%lu\n", expectation,
		  probabilityOfNoRandomHit, 
                  motifMappedMemoryObject -> specificities[motifIndex],
		  motifRecord, sequenceSegment, i + 1, i + motifWidth);
	}
      }
    }
  }
}


/*
 * ----------
 * MapMotifBlocksToMemory
 * ==========
 * 
 * ----------
 */

static void MapMotifBlocksToMemory (int useLocalPath,
                                    motifMappedMemoryADT
				    motifMappedMemoryObject,
				    char *motifDatabase)
{
  int motifFileRootLength;
  char filename[FILENAME_LENGTH + 1];

  if (!useLocalPath) {
    strcpy (filename, "/usr/share/emotif/emotifs/");
  } else {
    strcpy (filename, "./@DATA_SUBDIR_BITMOTIFS@/");
  }

  strcat (filename, motifDatabase);
  motifFileRootLength = strlen (filename);

  strcpy ((char*) (filename + motifFileRootLength), "-stats");
  InitializeMotifGlobalVariables (filename);

  strcpy ((char*) (filename + motifFileRootLength), "-logSpecificities.bin");
  motifMappedMemoryObject -> logSpecificities = MapDoublesBlock (filename,
							      numMotifs);

  strcpy ((char*) (filename + motifFileRootLength), "-specificities.bin");
  motifMappedMemoryObject -> specificities = MapDoublesBlock (filename,
							      numMotifs);

  strcpy ((char*) (filename + motifFileRootLength), "-lengthCorrections.bin");
  motifMappedMemoryObject -> lengthCorrections = MapDoublesBlock (filename,
								  numMotifs);

  strcpy ((char*) (filename + motifFileRootLength), "-bitMotifs.bin");
  motifMappedMemoryObject -> bitMotifs = MapUnsignedLongsBlock (filename,
								numMotifBits);

  strcpy ((char*) (filename + motifFileRootLength), "-bitIndices.bin");
  motifMappedMemoryObject -> bitIndices =
    MapUnsignedLongsBlock (filename, numMotifs + 1);

  strcpy ((char*) (filename + motifFileRootLength), "-homingBitLetters.bin");
  motifMappedMemoryObject -> homingBitLetters =
    MapUnsignedLongsBlock (filename, numMotifs);

  strcpy ((char*) (filename + motifFileRootLength), "-offsets.bin");
  motifMappedMemoryObject -> offsets = 
    MapUnsignedLongsBlock (filename, numMotifs);
  
  strcpy ((char*) (filename + motifFileRootLength), "-records.bin");
  motifMappedMemoryObject -> records = MapCharsBlock (filename, numMotifChars);

  strcpy ((char*) (filename + motifFileRootLength), "-recordIndices.bin");
  motifMappedMemoryObject -> recordIndices =
    MapUnsignedLongsBlock (filename, numMotifs + 1);
}


static void InitializeMotifGlobalVariables (char *filename)
{
  char statsBuffer[STATS_LENGTH];
  Scanner scannerObject;

  scannerObject = NewScannerFromFilename (filename, "\n", TRUE);
  assert (scannerObject != NULL);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numMotifs = strtoul (statsBuffer, NULL, 10);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numMotifBits = strtoul (statsBuffer, NULL, 10);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numMotifChars = strtoul (statsBuffer, NULL, 10);

  FreeScanner (scannerObject);
}


/*
 * ----------
 * MapSequenceBlocksToMemory
 * ==========
 * 
 * ----------
 */

static void MapSequenceBlocksToMemory (int useLocalPath,
                                       sequenceMappedMemoryADT
				       sequenceMappedMemoryObject,
				       char *sequenceDatabase)
{
  int sequenceFileRootLength;
  char filename[FILENAME_LENGTH + 1];

  if (!useLocalPath) {
    strcpy (filename, "/usr/share/emotif/sequences/");
  } else {
    strcpy (filename, "./@DATA_SUBDIR_BITSEQUENCES@/");
  }

  strcat (filename, sequenceDatabase);
  sequenceFileRootLength = strlen (filename);

  strcpy ((char*) (filename + sequenceFileRootLength), "-stats");
  InitializeSequenceGlobalVariables (filename);

  strcpy ((char*) (filename + sequenceFileRootLength), "-names.bin");
  sequenceMappedMemoryObject -> names = MapCharsBlock (filename, 
						       numSequenceNameChars);

  strcpy ((char*) (filename + sequenceFileRootLength), "-nameIndices.bin");
  sequenceMappedMemoryObject -> nameIndices = 
    MapUnsignedLongsBlock (filename, numSequences + 1);

  strcpy ((char*) (filename + sequenceFileRootLength), "-textSequences.bin");
  sequenceMappedMemoryObject -> textSequences =
    MapCharsBlock (filename, numSequenceBits);

  strcpy ((char*) (filename + sequenceFileRootLength), "-bitSequences.bin");
  sequenceMappedMemoryObject -> bitSequences =
    MapUnsignedLongsBlock (filename, numSequenceBits);

  strcpy ((char*) (filename + sequenceFileRootLength), "-sequenceIndices.bin");
  sequenceMappedMemoryObject -> sequenceIndices =
    MapUnsignedLongsBlock (filename, numSequenceBits);

  strcpy ((char*) (filename + sequenceFileRootLength), "-offsets.bin");
  sequenceMappedMemoryObject -> offsets =
    MapUnsignedLongsBlock (filename, numSequenceBits);

  strcpy ((char*) (filename + sequenceFileRootLength), "-offsetIndices.bin");
  sequenceMappedMemoryObject -> offsetIndices =
    MapUnsignedLongsBlock (filename, numSequences * NUM_LETTERS + 1);
}


static void InitializeSequenceGlobalVariables (char *filename)
{
  char statsBuffer[STATS_LENGTH];
  Scanner scannerObject;

  scannerObject = NewScannerFromFilename (filename, "\n", TRUE);
  assert (scannerObject != NULL);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numSequences = strtoul (statsBuffer, NULL, 10);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numSequenceNameChars = strtoul (statsBuffer, NULL, 10);

  ReadNextToken (scannerObject, statsBuffer, STATS_LENGTH + 1);
  numSequenceBits = strtoul (statsBuffer, NULL, 10);

  FreeScanner (scannerObject);
}


/*
 * ----------
 * UnmapMotifBlocksFromMemory
 * ==========
 * 
 * ----------
 */

static void UnmapMotifBlocksFromMemory (motifMappedMemoryADT
					motifMappedMemoryObject)
{
  munmap ((void *) (motifMappedMemoryObject -> logSpecificities), numMotifs);
  munmap ((void *) (motifMappedMemoryObject -> specificities), numMotifs);
  munmap ((void *) (motifMappedMemoryObject -> lengthCorrections),
          numMotifs);
  munmap ((void *) (motifMappedMemoryObject -> bitMotifs),
          numMotifBits);
  munmap ((void *) (motifMappedMemoryObject -> bitIndices), 
          numMotifs + 1);
  munmap ((void *) (motifMappedMemoryObject -> homingBitLetters),
          numMotifs);
  munmap ((void *) (motifMappedMemoryObject -> offsets), numMotifs);
  munmap ((void *) (motifMappedMemoryObject -> records), numMotifChars);
  munmap ((void *) (motifMappedMemoryObject -> recordIndices), 
          numMotifs + 1);
}


/*
 * ----------
 * UnmapSequenceBlocksFromMemory
 * ==========
 * 
 * ----------
 */

static void UnmapSequenceBlocksFromMemory (sequenceMappedMemoryADT
					   sequenceMappedMemoryObject)
{
  munmap ((void *) (sequenceMappedMemoryObject -> names),
          numSequenceNameChars);
  munmap ((void *) (sequenceMappedMemoryObject -> nameIndices), 
          numSequences + 1);
  munmap ((void *) (sequenceMappedMemoryObject -> textSequences), 
          numSequenceBits);
  munmap ((void *) (sequenceMappedMemoryObject -> bitSequences), 
          numSequenceBits);
  munmap ((void *) (sequenceMappedMemoryObject -> sequenceIndices),
          numSequences + 1);
  munmap ((void *) (sequenceMappedMemoryObject -> offsets), 
          numSequenceBits);
  munmap ((void *) (sequenceMappedMemoryObject -> offsetIndices), 
	  numSequences * NUM_LETTERS + 1);
}

