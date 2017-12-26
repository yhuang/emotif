#include <assert.h>
#include <ctype.h>
#include "scanner.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define ALPHABET_SIZE 26
#define CUTOFF_BITS 20
#define FALSE 0
#define MAX_LENGTH 10000
#define MAX_WIDTH 70
#define NUM_AMINO_ACIDS 20
#define TRUE 1


static float CalculateAverageEntropy(char *alignment, int numSequences, 
				     int width, int *zeros);
static void CalculateBasedOnSharedEntropy(char *alignmentDatabase);


float LOG_2, MAX_ENTROPY;


int main (int argc, char **argv)
{
  Scanner trimmedBlocksScanner;
  char buffer[MAX_LENGTH], accession[MAX_LENGTH], description[MAX_LENGTH],
    motif[MAX_LENGTH], sensitivity[MAX_LENGTH],
    alignment[MAX_LENGTH * MAX_WIDTH];
  int i, j, numSequencesInBlock, width, zeros[ALPHABET_SIZE];
  float specificity, p, avgEntropy, maxEntropy;

  LOG_2 = log(2);

  p = (float) 1 / NUM_AMINO_ACIDS;
  MAX_ENTROPY = -p * (log(p) / LOG_2) * NUM_AMINO_ACIDS;

  trimmedBlocksScanner = NewScannerFromFile(stdin, ">|\n", TRUE);

  for (i = 0; i < ALPHABET_SIZE; i++) {
    zeros[i] = 0;
  }

  /* Block Accession */
  while (ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH)) {
    strcpy(accession, buffer);
    
    /* Block Description */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    strcpy(description, buffer);

    /* Motif Specificity */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    specificity = atof(buffer);

    /* Motif */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    strcpy(motif, buffer);

    /* Motif Sensitivity */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    strcpy(sensitivity, buffer);

    /* Block Size */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    numSequencesInBlock = atoi(buffer);

    /* Block Width */
    ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
    width = atoi(buffer);

    /* Sequence Alignment */
    for (i = 0; i < numSequencesInBlock; i++) {
      /* Protein Accession */
      ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);

      ReadNextToken(trimmedBlocksScanner, buffer, MAX_LENGTH);
      strncpy(&alignment[i * MAX_WIDTH], buffer, MAX_WIDTH);
    }

    avgEntropy = CalculateAverageEntropy(alignment, numSequencesInBlock,
					 width, zeros);

    if (width < NUM_AMINO_ACIDS) {
      p = (float) 1 / width;
      maxEntropy = -p * (log(p) / LOG_2) * width;

      if (avgEntropy / maxEntropy <= 0.60f) {
	continue;
      }
    } else {
      if (avgEntropy / MAX_ENTROPY <= 0.60f) {
	continue;
      }
    }

    fprintf(stdout, "%.3f\t%s\t%s\t%s\t%s\n", 
	    specificity, accession, description, motif, sensitivity);
  }

  FreeScanner(trimmedBlocksScanner);

  fclose(stdin);
  fclose(stdout);
}


static float CalculateAverageEntropy(char *alignment, int numSequences, 
				     int width, int *zeros)
{
  int i, j, aaCountsLookup[ALPHABET_SIZE];
  float p, rowEntropy, totalEntropy;

  totalEntropy = 0.0;

  for (i = 0; i < numSequences; i++) {
    rowEntropy = 0.0;
    memcpy(aaCountsLookup, zeros, ALPHABET_SIZE * sizeof(int));

    for (j = 0; j < width; j++) {
      aaCountsLookup[alignment[i * MAX_WIDTH + j] - 'A']++;
    }

    for (j = 0; j < ALPHABET_SIZE; j++) {
      if (aaCountsLookup[j]) {
	p = (float) aaCountsLookup[j] / width;
	rowEntropy += p * (log(p) / LOG_2);
      }
    }

    totalEntropy += rowEntropy;
  }

  return (-(totalEntropy / numSequences));
}

