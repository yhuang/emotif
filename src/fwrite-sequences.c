/*
 * File:  fwrite-sequences.c
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

static void FreeMemoryBlockObject (sequenceMemoryBlockADT
				   sequenceMemoryBlockObject);

static void InitializeMemoryBlockObject (sequenceMemoryBlockADT
					 sequenceMemoryBlockObject);

static void LoadDatabase (FILE *infile,
                          sequenceMemoryBlockADT sequenceMemoryBlockObject);

static void MapSequenceContent (char *textSequence,
				unsigned long *charToBitMap,
				DArray bitSequencesArray,
				sequenceOffsetMapADT
				sequenceOffsetMapObject);

static void MapSequenceOffsets (sequenceOffsetMapADT sequenceOffsetMapObject,
				DArray offsetsArray,
				DArray offsetIndicesArray);

static unsigned long PushCharsOntoDArray (char *string, DArray charsArray);

static void WriteMemoryBlocksToBinaryFiles (sequenceMemoryBlockADT
					    sequenceMemoryBlockObject,
					    char *fileRoot,
					    int fileRootLength);


int main (int argc, char *argv[])
{
  sequenceMemoryBlockADT sequenceMemoryBlockObject;
  int opt, fileRootLength;
  char database[FILENAME_LENGTH + 1], fileRoot[FILENAME_LENGTH + 1];
  FILE *infile, *outfile;

  strcpy (database, "sprot");
  infile = stdin;

  while ((opt = getopt (argc, argv, "d:")) != -1) {
    switch (opt) {
    case 'd': strcpy (database, optarg);
      break;
   }
  }

  strcpy (fileRoot, "./sequences/");
  strcat (fileRoot, database);
  fileRootLength = strlen (fileRoot);

  sequenceMemoryBlockObject = (sequenceMemoryBlockADT)
    malloc (sizeof (struct sequenceMemoryBlockCDT));
  assert (sequenceMemoryBlockObject != NULL);

  InitializeMemoryBlockObject (sequenceMemoryBlockObject);

  LoadDatabase (infile, sequenceMemoryBlockObject);

  WriteMemoryBlocksToBinaryFiles (sequenceMemoryBlockObject, fileRoot,
				  fileRootLength);

  strcpy (fileRoot + fileRootLength, "-stats");
  outfile = fopen (fileRoot, "w");

  fprintf (outfile, "%u\n", 
	   ArrayLength (sequenceMemoryBlockObject -> 
			sequenceIndicesArray) - 1);
  fprintf (outfile, "%u\n",
	   ArrayLength (sequenceMemoryBlockObject -> namesArray));
  fprintf (outfile, "%u\n",
	   ArrayLength (sequenceMemoryBlockObject -> textSequencesArray));

  fclose (outfile);
  fclose (infile);

  FreeMemoryBlockObject (sequenceMemoryBlockObject);
  free (sequenceMemoryBlockObject);

  return (0);
}


static void InitializeMemoryBlockObject (sequenceMemoryBlockADT
					 sequenceMemoryBlockObject)
{
  sequenceMemoryBlockObject -> namesArray =
    ArrayNew (sizeof (char), MAX_NUM_CHARS, NULL);
  assert (sequenceMemoryBlockObject -> namesArray != NULL);

  sequenceMemoryBlockObject -> nameIndicesArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_SEQUENCES + 1, NULL);
  assert (sequenceMemoryBlockObject -> nameIndicesArray != NULL);

  sequenceMemoryBlockObject -> textSequencesArray =
    ArrayNew (sizeof (char), MAX_NUM_CHARS, NULL);
  assert (sequenceMemoryBlockObject -> textSequencesArray != NULL);

  sequenceMemoryBlockObject -> bitSequencesArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_BITS, NULL);
  assert (sequenceMemoryBlockObject -> bitSequencesArray != NULL);

  sequenceMemoryBlockObject -> sequenceIndicesArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_SEQUENCES + 1, NULL);
  assert (sequenceMemoryBlockObject -> sequenceIndicesArray != NULL);

  sequenceMemoryBlockObject -> offsetsArray =
    ArrayNew (sizeof (unsigned long), MAX_NUM_BITS, NULL);
  assert (sequenceMemoryBlockObject -> offsetsArray != NULL);

  sequenceMemoryBlockObject -> offsetIndicesArray =
    ArrayNew (sizeof (unsigned long), NUM_LETTERS * MAX_NUM_SEQUENCES + 1,
	      NULL);
  assert (sequenceMemoryBlockObject -> offsetIndicesArray != NULL);
}


static void LoadDatabase (FILE *infile,
                          sequenceMemoryBlockADT sequenceMemoryBlockObject)
{
  char nameBuffer[RECORD_LENGTH + 1], textSequenceBuffer[SEQUENCE_LENGTH + 1];
  unsigned long i, nameStart, sequenceStart, offset, charToBitMap['z' + 1];
  Scanner scannerObject;
  sequenceOffsetMapADT sequenceOffsetMapObject;

  InitializeCharToBitMap (charToBitMap, 'z' + 1);

  sequenceOffsetMapObject = (sequenceOffsetMapADT)
    malloc (sizeof (struct sequenceOffsetMapCDT));
  assert (sequenceOffsetMapObject != NULL);

  scannerObject = NewScannerFromFile (infile, "\n", TRUE);
  assert (scannerObject != NULL);

  nameStart = sequenceStart = offset = 0;
  ArrayAppend (sequenceMemoryBlockObject -> nameIndicesArray, &nameStart);
  ArrayAppend (sequenceMemoryBlockObject -> sequenceIndicesArray, 
	       &sequenceStart);
  ArrayAppend (sequenceMemoryBlockObject -> offsetIndicesArray, 
	       &offset);

  while (ReadNextToken (scannerObject, nameBuffer, RECORD_LENGTH + 1)) {
    nameStart += PushCharsOntoDArray ((char*) (nameBuffer + 1),
				      sequenceMemoryBlockObject -> namesArray);
    ArrayAppend (sequenceMemoryBlockObject -> nameIndicesArray, &nameStart);

    ReadNextToken (scannerObject, textSequenceBuffer, SEQUENCE_LENGTH + 1);

    sequenceStart += PushCharsOntoDArray (textSequenceBuffer,
					  sequenceMemoryBlockObject ->
					  textSequencesArray);
    ArrayAppend (sequenceMemoryBlockObject -> sequenceIndicesArray,
		 &sequenceStart);
    
    for (i = 0; i < NUM_LETTERS; i++) {
      sequenceOffsetMapObject -> map[i] = ArrayNew (sizeof (unsigned long),
						    SEQUENCE_LENGTH, NULL);
      assert (sequenceOffsetMapObject -> map[i] != NULL);
    }

    MapSequenceContent (textSequenceBuffer, charToBitMap,
			sequenceMemoryBlockObject ->
			bitSequencesArray, sequenceOffsetMapObject);

    MapSequenceOffsets (sequenceOffsetMapObject,
			sequenceMemoryBlockObject -> offsetsArray,
			sequenceMemoryBlockObject -> offsetIndicesArray);

    for (i = 0; i < NUM_LETTERS; i++) {
      ArrayFree (sequenceOffsetMapObject -> map[i]);
    }
  }
  
  free (sequenceOffsetMapObject);
  FreeScanner (scannerObject);
}


static unsigned long PushCharsOntoDArray (char *string, DArray charsArray)
{
  char character;
  unsigned i;

  for (i = 0; string[i]; i++) {
    character = string[i];
    ArrayAppend (charsArray, &character);
  }

  return (i);
}


/*
 * ----------
 * MapSequenceContent
 * ==========
 * The function MapSequence will convert the string of amino acid letters
 * in the dynamic array the char pointer textSequence points to into a
 * string of bits.  
 * ----------
 */

static void MapSequenceContent (char *textSequence,
				unsigned long *charToBitMap,
				DArray bitSequencesArray,
				sequenceOffsetMapADT sequenceOffsetMapObject)
{
  unsigned long i, bit;

  for (i = 0; textSequence[i]; i++) {
    bit = charToBitMap[(int) textSequence[i]];
    ArrayAppend (bitSequencesArray, &bit);

    if ((textSequence[i] - 'a') > 0) {
      ArrayAppend (sequenceOffsetMapObject -> 
		   map[textSequence[i] - 'a'], &i);
    } else {
      ArrayAppend (sequenceOffsetMapObject -> 
		   map[textSequence[i] - 'A'], &i);
    }
  }
}


static void MapSequenceOffsets (sequenceOffsetMapADT sequenceOffsetMapObject,
				DArray offsetsArray, DArray offsetIndicesArray)
{
  unsigned long i, j, offset, mapLength, offsetsArrayLength;

  for (i = 0; i < NUM_LETTERS; i++) {
    mapLength = ArrayLength (sequenceOffsetMapObject -> map[i]);

    for (j = 0; j < mapLength; j++) {
      offset = *(unsigned long*) ArrayNth (sequenceOffsetMapObject ->
					   map[i], j);
      ArrayAppend (offsetsArray, &offset);
    }

    offsetsArrayLength = ArrayLength (offsetsArray);
    ArrayAppend (offsetIndicesArray, &offsetsArrayLength);
  }
}


static void WriteMemoryBlocksToBinaryFiles (sequenceMemoryBlockADT
					    sequenceMemoryBlockObject,
					    char *fileRoot, int fileRootLength)
{
  char filename[FILENAME_LENGTH + 1];

  strcpy (filename, fileRoot);

  strcpy ((char*) (filename + fileRootLength), "-names.bin");
  WriteCharsBlockToBinaryFile (sequenceMemoryBlockObject -> namesArray,
			       filename);

  strcpy ((char*) (filename + fileRootLength), "-nameIndices.bin");
  WriteUnsignedLongsBlockToBinaryFile (sequenceMemoryBlockObject ->
				       nameIndicesArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-textSequences.bin");
  WriteCharsBlockToBinaryFile (sequenceMemoryBlockObject -> textSequencesArray,
			       filename);

  strcpy ((char*) (filename + fileRootLength), "-bitSequences.bin");
  WriteUnsignedLongsBlockToBinaryFile (sequenceMemoryBlockObject -> 
				       bitSequencesArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-sequenceIndices.bin");
  WriteUnsignedLongsBlockToBinaryFile (sequenceMemoryBlockObject -> 
				       sequenceIndicesArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-offsets.bin");
  WriteUnsignedLongsBlockToBinaryFile (sequenceMemoryBlockObject ->
				       offsetsArray, filename);

  strcpy ((char*) (filename + fileRootLength), "-offsetIndices.bin");
  WriteUnsignedLongsBlockToBinaryFile (sequenceMemoryBlockObject ->
				       offsetIndicesArray, filename);
}


static void FreeMemoryBlockObject (sequenceMemoryBlockADT 
				   sequenceMemoryBlockObject)
{ 
  ArrayFree (sequenceMemoryBlockObject -> namesArray);
  ArrayFree (sequenceMemoryBlockObject -> nameIndicesArray);
  ArrayFree (sequenceMemoryBlockObject -> textSequencesArray);
  ArrayFree (sequenceMemoryBlockObject -> bitSequencesArray);
  ArrayFree (sequenceMemoryBlockObject -> sequenceIndicesArray);
  ArrayFree (sequenceMemoryBlockObject -> offsetsArray);
  ArrayFree (sequenceMemoryBlockObject -> offsetIndicesArray);
}

