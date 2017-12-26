/*
 * File:  fwrite-emotifs.c
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
#include "darray.h"
#include "emotif.h"
#include <math.h>
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


/*
 * ----------
 * Function Prototypes
 * ----------
 */

static unsigned long ConcatenateFields (char *id, char *description,
					char *motif, char *sensitivity,
					DArray recordsArray);

static unsigned long ConvertToBinary (char *motif, DArray bitMotifsArray,
				      double *charToPercentageMap,
				      unsigned long *homingBitLetterPtr,
				      unsigned long *motifOffsetPtr);

static void FreeMemoryBlockObject (motifMemoryBlockADT
				   motifMemoryBlockObject);

static void InitializeMemoryBlockObject (motifMemoryBlockADT
					 motifMemoryBlockObject);

static void LoadDatabase (FILE *infile,
	                  motifMemoryBlockADT motifMemoryBlockObject);

static void WriteMemoryBlocksToBinaryFiles (motifMemoryBlockADT
					    motifMemoryBlockObject,
					    char *fileRoot,
					    int fileRootLength);


int main (int argc, char *argv[])
{
  motifMemoryBlockADT motifMemoryBlockObject;
  int opt, fileRootLength;
  char database[FILENAME_LENGTH + 1], fileRoot[FILENAME_LENGTH + 1];
  FILE *infile, *outfile;

  strcpy (database, "blocksplus");
  infile = stdin;

  while ((opt = getopt (argc, argv, "d:")) != -1) {
    switch (opt) {
    case 'd': strcpy (database, optarg);
      break;
   }
  }

  strcpy (fileRoot, "./emotifs/");
  strcat (fileRoot, database);
  fileRootLength = strlen (fileRoot);

  motifMemoryBlockObject = (motifMemoryBlockADT)
    malloc (sizeof (struct motifMemoryBlockCDT));
  assert (motifMemoryBlockObject != NULL);

  InitializeMemoryBlockObject (motifMemoryBlockObject);

  LoadDatabase (infile, motifMemoryBlockObject);

  WriteMemoryBlocksToBinaryFiles (motifMemoryBlockObject, fileRoot,
				  fileRootLength);

  strcpy (fileRoot + fileRootLength, "-stats");
  outfile = fopen (fileRoot, "w");

  fprintf (outfile, "%u\n", 
	   ArrayLength (motifMemoryBlockObject -> logSpecificitiesArray));
  fprintf (outfile, "%u\n",
	   ArrayLength (motifMemoryBlockObject -> bitMotifsArray));
  fprintf (outfile, "%u\n",
	   ArrayLength (motifMemoryBlockObject -> recordsArray));

  fclose (outfile);
  fclose (infile);

  FreeMemoryBlockObject (motifMemoryBlockObject);
  free (motifMemoryBlockObject);

  return (0);
}


static void InitializeMemoryBlockObject (motifMemoryBlockADT
					 motifMemoryBlockObject)
{
  motifMemoryBlockObject -> logSpecificitiesArray =
    ArrayNew (sizeof (double), MAX_NUM_EMOTIFS, NULL);
  assert (motifMemoryBlockObject -> logSpecificitiesArray != NULL);

  motifMemoryBlockObject -> specificitiesArray =
    ArrayNew (sizeof (double), MAX_NUM_EMOTIFS, NULL);
  assert (motifMemoryBlockObject -> specificitiesArray != NULL);

  motifMemoryBlockObject -> lengthCorrectionsArray =
    ArrayNew (sizeof (double), MAX_NUM_EMOTIFS, NULL);
  assert (motifMemoryBlockObject -> lengthCorrectionsArray != NULL);

  motifMemoryBlockObject -> bitMotifsArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_BITS, NULL);
  assert (motifMemoryBlockObject -> bitMotifsArray != NULL);

  motifMemoryBlockObject -> bitIndicesArray =
    ArrayNew (sizeof (unsigned long),  MAX_NUM_EMOTIFS + 1, NULL);
  assert (motifMemoryBlockObject -> bitIndicesArray != NULL);

  motifMemoryBlockObject -> homingBitLettersArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_EMOTIFS, NULL);
  assert (motifMemoryBlockObject -> homingBitLettersArray != NULL);

  motifMemoryBlockObject -> offsetsArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_EMOTIFS, NULL);
  assert (motifMemoryBlockObject -> offsetsArray != NULL);

  motifMemoryBlockObject -> recordsArray =
    ArrayNew (sizeof (char), MAX_NUM_CHARS, NULL);
  assert (motifMemoryBlockObject -> recordsArray != NULL);

  motifMemoryBlockObject -> recordIndicesArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_EMOTIFS + 1, NULL);
  assert (motifMemoryBlockObject -> recordIndicesArray != NULL);
}


static void LoadDatabase (FILE *infile,
	                  motifMemoryBlockADT motifMemoryBlockObject)
{
  char logSpecificityBuffer[LOG_SPECIFICITY_LENGTH + 1], 
    idBuffer[ID_LENGTH + 1], descriptionBuffer[DESCRIPTION_LENGTH + 1],
    motifBuffer[MOTIF_LENGTH + 1], sensitivityBuffer[SENSITIVITY_LENGTH + 1];
  double logSpecificity, specificity, lengthCorrection,
    charToPercentageMap['z' + 1];
  unsigned long recordStart, bitStart, motifWidth, homingBitLetter,
    motifOffset;
  Scanner scannerObject;

  InitializeCharToPercentageMap (charToPercentageMap);

  scannerObject = NewScannerFromFile (infile, "\t\n", TRUE);
  assert (scannerObject != NULL);

  recordStart = bitStart = 0;
  ArrayAppend (motifMemoryBlockObject -> recordIndicesArray, &recordStart);
  ArrayAppend (motifMemoryBlockObject -> bitIndicesArray, &bitStart);

  while (ReadNextToken (scannerObject, logSpecificityBuffer,
			LOG_SPECIFICITY_LENGTH + 1)) {
    logSpecificity = strtod (logSpecificityBuffer, NULL);
    ArrayAppend (motifMemoryBlockObject -> logSpecificitiesArray,
		 &logSpecificity);

    specificity = pow (10, logSpecificity);
    ArrayAppend (motifMemoryBlockObject -> specificitiesArray,
		 &specificity);

    ReadNextToken (scannerObject, idBuffer, ID_LENGTH + 1);
    ReadNextToken (scannerObject, descriptionBuffer, DESCRIPTION_LENGTH + 1);
    ReadNextToken (scannerObject, motifBuffer, MOTIF_LENGTH + 1);

    motifWidth = ConvertToBinary (motifBuffer, 
				   motifMemoryBlockObject -> bitMotifsArray,
				   charToPercentageMap, &homingBitLetter,
				   &motifOffset);

    lengthCorrection = (motifWidth * specificity);
    ArrayAppend (motifMemoryBlockObject -> lengthCorrectionsArray,
		 &lengthCorrection);

    bitStart += motifWidth;
    ArrayAppend (motifMemoryBlockObject -> bitIndicesArray, &bitStart);

    ArrayAppend (motifMemoryBlockObject -> homingBitLettersArray,
		 &homingBitLetter);

    ArrayAppend (motifMemoryBlockObject -> offsetsArray,
		 &motifOffset);

    ReadNextToken (scannerObject, sensitivityBuffer, SENSITIVITY_LENGTH + 1);

    recordStart += ConcatenateFields (idBuffer, descriptionBuffer,
				      motifBuffer, sensitivityBuffer,
				      motifMemoryBlockObject -> recordsArray);

    ArrayAppend (motifMemoryBlockObject -> recordIndicesArray, &recordStart);
  }

  FreeScanner (scannerObject);
}


static void WriteMemoryBlocksToBinaryFiles (motifMemoryBlockADT
					    motifMemoryBlockObject,
					    char *fileRoot, int fileRootLength)
{
  char filename[FILENAME_LENGTH + 1];

  strcpy (filename, fileRoot);

  strcpy ((char*) (filename + fileRootLength), "-logSpecificities.bin");
  WriteDoublesBlockToBinaryFile (motifMemoryBlockObject -> 
				 logSpecificitiesArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-specificities.bin");
  WriteDoublesBlockToBinaryFile (motifMemoryBlockObject -> specificitiesArray, 
				 filename);

  strcpy ((char*) (filename + fileRootLength), "-lengthCorrections.bin");
  WriteDoublesBlockToBinaryFile (motifMemoryBlockObject ->
				 lengthCorrectionsArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-bitMotifs.bin");
  WriteUnsignedLongsBlockToBinaryFile (motifMemoryBlockObject ->
				       bitMotifsArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-bitIndices.bin");
  WriteUnsignedLongsBlockToBinaryFile (motifMemoryBlockObject -> 
				       bitIndicesArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-homingBitLetters.bin");
  WriteUnsignedLongsBlockToBinaryFile (motifMemoryBlockObject ->
				       homingBitLettersArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-offsets.bin");
  WriteUnsignedLongsBlockToBinaryFile (motifMemoryBlockObject ->
				       offsetsArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-records.bin");
  WriteCharsBlockToBinaryFile (motifMemoryBlockObject -> recordsArray,
			       filename);

  strcpy ((char*) (filename + fileRootLength), "-recordIndices.bin");
  WriteUnsignedLongsBlockToBinaryFile (motifMemoryBlockObject -> 
				       recordIndicesArray, filename);
}


static unsigned long ConvertToBinary (char *motif, DArray bitMotifsArray,
				      double *charToPercentageMap,
				      unsigned long *homingBitLetterPtr,
				      unsigned long *motifOffsetPtr)
{
  unsigned long i, bit, bestRankPosition, motifWidth;
  double groupRank, bestRank;

  bestRank = WORST_RANK;
  motifWidth = bestRankPosition = 0;

  for (i = 0; motif[i]; i++) {
    bit = 0;

    if (motif[i] == '.') {
      bit = WILDCARD_BIT_PATTERN;
      ArrayAppend (bitMotifsArray, &bit);
    } else if (motif[i] == '[') {
      groupRank = 0.0;

      while (motif[++i] != ']') {
	bit |= 1 << (motif[i] - 'a');
	groupRank += charToPercentageMap[(int) motif[i]];
      }

      ArrayAppend (bitMotifsArray, &bit);

      if (groupRank < bestRank) {
	bestRank = groupRank;
	bestRankPosition = motifWidth;
	*homingBitLetterPtr = bit;
	*motifOffsetPtr = motifWidth;
      }
    } else {
      bit = 1 << (motif[i] - 'a');
      ArrayAppend (bitMotifsArray, &bit);

      if (charToPercentageMap[(int) motif[i]] < bestRank) {
	bestRank = charToPercentageMap[(int) motif[i]];
	bestRankPosition = motifWidth;
	*homingBitLetterPtr = bit;
	*motifOffsetPtr = motifWidth;
      }
    }

    motifWidth++;
  }

  return (motifWidth);
}


static unsigned long ConcatenateFields (char *id, char *description,
					char *motif, char *sensitivity,
					DArray recordsArray)
{
  unsigned long recordLength, i;
  char character;

  recordLength = 0;

  for (i = 0; sensitivity[i]; i++) {
    character = sensitivity[i];
    ArrayAppend (recordsArray, &character);
    recordLength++;
  }

  character = '|';
  ArrayAppend (recordsArray, &character);
  recordLength++;

  for (i = 0; id[i]; i++) {
    character = id[i];
    ArrayAppend (recordsArray, &character);
    recordLength++;
  }

  character = '|';
  ArrayAppend (recordsArray, &character);
  recordLength++;

  for (i = 0; description[i]; i++) {
    character = description[i];
    ArrayAppend (recordsArray, &character);
    recordLength++;
  }

  character = '|';
  ArrayAppend (recordsArray, &character);
  recordLength++;

  for (i = 0; motif[i]; i++) {
    character = motif[i];
    ArrayAppend (recordsArray, &character);
    recordLength++;
  }

  return (recordLength);
}


static void FreeMemoryBlockObject (motifMemoryBlockADT motifMemoryBlockObject)
{ 
  ArrayFree (motifMemoryBlockObject -> logSpecificitiesArray);
  ArrayFree (motifMemoryBlockObject -> specificitiesArray);
  ArrayFree (motifMemoryBlockObject -> lengthCorrectionsArray);
  ArrayFree (motifMemoryBlockObject -> bitMotifsArray);
  ArrayFree (motifMemoryBlockObject -> bitIndicesArray);
  ArrayFree (motifMemoryBlockObject -> homingBitLettersArray);
  ArrayFree (motifMemoryBlockObject -> offsetsArray);
  ArrayFree (motifMemoryBlockObject -> recordsArray);
  ArrayFree (motifMemoryBlockObject -> recordIndicesArray);
}

