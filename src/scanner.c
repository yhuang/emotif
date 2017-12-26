/*
 * File:  scanner.c
 * Date:  10-01-98
 * Course:  CS107
 * Grader:  Erik Haugen
 * Programmer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 *
 * ------------------------------------------------------------
 * This file implements the Scanner ADT.
 */


#define MAX_CHARACTERS 256


#include <assert.h>
#include <ctype.h>
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Type:  modeT
 * ------------
 *
 */

typedef enum {
  OVER, UNTIL
} modeT;


/*
 * struct ScannerImplementation
 * ----------------------------
 * The ScannerImplementation contains five fields.  The variable
 * _delimitersLookup of type char* contains the set of delimiting
 * characters.  The variable _filename of type char* will have the
 * name of the file the client opened via NewScannerFromFilename.
 * The variable _dlength of type int will keep track of how many
 * characters there are in this set of delimiting characters.  The
 * variable _infile of type FILE* will point to the file from which
 * the scanner reads its characters.  The variable _discard of type
 * int will indicate whether or not the delimiters are to be
 * thrown out.
 */

struct ScannerImplementation {
  char _charHolder, _delimiterHolder, *_filename;
  char _delimitersLookup[MAX_CHARACTERS], _skipsLookup[MAX_CHARACTERS];
  FILE *_infile;
  int _discard;
};


/* Private Function Prototypes */

static void InitializeDelimitersLookup (Scanner _scanner,
					const char *delimiters);
static void InitializeSkipsLookup (Scanner _scanner);
static void ResetSkipsLookup (Scanner _scanner, const char *skips);
static void SetSkipsLookup (Scanner _scanner, const char *skips);
static int Skip (Scanner _scanner, const char *skipsLookup, modeT mode);


/* Exported Functions */

/*
 * NewScannerFromFile
 * ------------------
 * This function, when called by the client, will dynamically allocate
 * memory for the underlying representation of the scanner.  The scanner
 * will scan tokens from this file.
 */

Scanner NewScannerFromFile (FILE *fp, const char *delimiters, int discard)
{
  Scanner _scanner;

  if (fp == NULL) {
    return (NULL);
  }

  _scanner = (Scanner) malloc (sizeof (struct ScannerImplementation)); 
  assert (_scanner != NULL);


  /* 
   * ----------
   * The file has already been opened the client; the NULL pointer here
   * is a reminder to the FreeScanner function that the FreeScanner
   * function will not have the authority to close the input file.
   * ----------
   */

  _scanner -> _filename = NULL;

  _scanner -> _charHolder = '\0';
  _scanner -> _delimiterHolder = '\0';
  _scanner -> _discard = discard;
  _scanner -> _infile = fp;

  InitializeDelimitersLookup (_scanner, delimiters);
  InitializeSkipsLookup (_scanner);

  return (_scanner);
}


/*
 * NewScannerFromFilename
 * ----------------------
 * This function, when called by the client, will dynamically allocate
 * memory for the underlying representation of the scanner.  Unlike
 * the NewScannerFromFile function, the NewScannerFromFilename function
 * will open the file whose name is to be supplied by the client.  The
 * scanner will scan tokens from this file.
 */

Scanner NewScannerFromFilename (const char *filename, const char
				*delimiters, int discard)
{
  Scanner _scanner;

  if (filename == NULL) {
    return (NULL);
  }

  _scanner = (Scanner) malloc (sizeof (struct ScannerImplementation)); 
  assert (_scanner != NULL);


  /* 
   * ----------
   * As long as this field is not NULL, the FreeScanner function
   * will have the authority to close the file associated with this
   * particular instance of the scanner. 
   * ----------
   */

  _scanner -> _filename = strdup (filename);  

  _scanner -> _charHolder = '\0';
  _scanner -> _delimiterHolder = '\0';
  _scanner -> _discard = discard;
  _scanner -> _infile = fopen (_scanner -> _filename, "r"); 

  if (_scanner -> _infile == NULL) {
    FreeScanner (_scanner);


    /* 
     * ----------
     * If the file sought by the client does not exist, memory allocated
     * dynamically will be freed before the function returns a NULL
     * pointer.
     * ---------- 
     */

    return (NULL);
  }

  InitializeDelimitersLookup (_scanner, delimiters);
  InitializeSkipsLookup (_scanner);

  return (_scanner);
}


/*
 * FreeScanner
 * -----------
 * FreeScanner is responsible for freeing all the memory that has been
 * dynamically allocated in implementing the scanner without orphaning
 * any memory.  If the scanner was constructed using
 * NewScannerFromFilename, the field _filename of type char* will not
 * be NULL.  The FreeScanner will, therefore, close the file associated
 * with the scanner to be destroyed.
 */

void FreeScanner (Scanner _scanner)
{
  if (_scanner -> _filename != NULL) {
    free (_scanner -> _filename);
  }

  if (_scanner -> _infile != NULL) {
    fclose (_scanner -> _infile);
  }

  free (_scanner);
}


/*
 * ReadNextToken
 * -------------
 * The ReadNextToken reads in the next token from the file associated
 * with the given scanner and puts it into a client-supplied buffer.
 * The reading is accomplished character by character using a
 * while loop idiom modified from Eric Roberts' The Art and Science of
 * C.  The function will read characters from a file as long as:
 *    1.  The EOF character has not been reached;
 *    2.  The client-supplied buffer has enough room for a NULL
 *        character;
 *    3.  The character just scanned in is not a delimiter.
 */

int ReadNextToken (Scanner _scanner, char buffer[], int bufLen)
{
  int i, ch;

  i = 0;

  if (_scanner -> _charHolder) {
    buffer[i++] = _scanner -> _charHolder;
    _scanner -> _charHolder = '\0';
  } else if (_scanner -> _delimiterHolder) {
    if (!_scanner -> _discard) {
      buffer[i++] = _scanner -> _delimiterHolder;
      _scanner -> _delimiterHolder = '\0';
      
      buffer[i] = '\0';
    
      /* 
       * ----------
       * The return value TRUE indicates to the client that the client's
       * buffer now has more than just a NULL character.
       * ----------
       */
      
      return (TRUE);  
    }

    _scanner -> _delimiterHolder = '\0';
  } else {
  }

  while (i < bufLen - 1 && (ch = getc (_scanner -> _infile)) != EOF) {

    /* 
     * ----------
     * The variable i must be less than the buffer's length minus one
     * character, so the buffer will have one character left and may be
     * properly terminated.
     * ----------
     */
    
    if (_scanner -> _delimitersLookup[ch]) {

      /* 
       * ----------
       * If the delimiter is not to be discarded, the following code will
       * be executed.
       * ----------
       */
      
      if (!_scanner -> _discard) {
	if (i == 0) {
	  buffer[i++] = ch;
	} else {
	  _scanner -> _delimiterHolder = ch;
	}
	
	break;
      } else {

	/* 
	 * ----------
	 * If the delimiter is to be discarded, the following code will be
	 * executed.
	 * ----------
	 */
	
	if (i == 0) {

	  /*
	   * ----------
	   * If a delimiter is encountered before any character is read,
	   * call Skip to discard any other delimiter that may follow.
	   * ----------
	   */

	  buffer[i++] = Skip (_scanner, _scanner -> _delimitersLookup, OVER);
	  _scanner -> _charHolder = '\0';

	  while (i < bufLen - 1 &&
		 (ch = getc (_scanner -> _infile)) != EOF) {
	    if (!(_scanner -> _delimitersLookup[ch])) {
	      buffer[i++] = ch;
	    } else {
	      _scanner -> _delimiterHolder = ch;
	      break;
	    }
	  }
	} else {
	  _scanner -> _delimiterHolder = ch;
	}
	
	break;
      }
    }
    
    buffer[i++] = ch;
  } /* while (i < bufLen - 1 && (ch = getc (_scanner -> _infile)) != EOF) */

  buffer[i] = '\0';
  
  if (i > 0) {

      /* 
       * ----------
       * The return value TRUE indicates to the client that the client's
       * buffer now has more than just a NULL character.
       * ----------
       */

    return (TRUE);
  }

  /*
   * ----------
   * The return value of FALSE indicates to the client that the client's
   *  buffer has only a NULL character.
   * ----------
   */

  return (FALSE);  
}


/*
 * SkipOver
 * --------
 * This exported function will skip over only those characters defined
 * in the set of skips by the client.  As soon as a character not of
 * this special set is encountered, the skipping will be terminated.
 * Since the character that terminated the skipping does not belong
 * to the set of "to-be-skipped" characters supplied by the client,
 * this function will return a copy of the terminating character in
 * case the client wishes to use that information.
 */

int SkipOver (Scanner _scanner, const char *skips)
{
  int ch;

  SetSkipsLookup (_scanner, skips);

  ch = Skip (_scanner, _scanner -> _skipsLookup, OVER);

  ResetSkipsLookup(_scanner, skips);

  return (ch);
}


/*
 * SkipUntil
 * ---------
 * This exported function will skip over characters in a file associated
 * with the given scanner if the characters scanned do not belong to
 * the set of "stop" characters as defined by the client.
 */

int SkipUntil (Scanner _scanner, const char *skips)
{
  int ch;

  SetSkipsLookup (_scanner, skips);

  ch = Skip (_scanner, _scanner -> _skipsLookup, UNTIL);

  ResetSkipsLookup(_scanner, skips);

  return (ch);
}


/*
 * SkipOverDelimiters
 * ------------------
 */

int SkipOverDelimiters (Scanner _scanner)
{
  return (Skip (_scanner, _scanner -> _delimitersLookup, OVER));
}


/*
 * SkipUntilDelimiters
 * -------------------
 */

int SkipUntilDelimiters (Scanner _scanner)
{
  return (Skip (_scanner, _scanner -> _delimitersLookup, UNTIL));
}


/* Private Functions */

/*
 * InitializeDelimitersLookup
 * --------------------------
 *
 */

static void InitializeDelimitersLookup (Scanner _scanner,
					const char *delimiters)
{
  int i;

  for (i = 0; i < MAX_CHARACTERS; i++) {
    _scanner -> _delimitersLookup[i] = 0;
  }

  for (i = 0; delimiters[i]; i++) {
    _scanner -> _delimitersLookup[(int)(delimiters[i])] = 1;
  }
}


/*
 * InitializeSkipsLookup
 * ---------------------
 *
 */

static void InitializeSkipsLookup (Scanner _scanner)
{
  int i;

  for (i = 0; i < MAX_CHARACTERS; i++) {
    _scanner -> _skipsLookup[i] = 0;
  }
}


/*
 * SetSkipsLookup
 * --------------
 *
 */

static void SetSkipsLookup (Scanner _scanner, const char *skips)
{
  int i;

  for (i = 0; skips[i]; i++) {
    _scanner -> _skipsLookup[(int)(skips[i])] = 1;
  }
}


/*
 * ResetSkipsLookup
 * ----------------
 *
 */

static void ResetSkipsLookup (Scanner _scanner, const char *skips)
{
  int i;

  for (i = 0; skips[i]; i++) {
    _scanner -> _skipsLookup[(int)(skips[i])] = 0;
  }
}


/*
 * Skip
 * ----
 *
 */

static int Skip (Scanner _scanner, const char *skipsLookup, modeT mode)
{
  int ch = EOF;

  if (mode == OVER) {
    if (_scanner -> _charHolder) {
      if (!skipsLookup[(int) _scanner -> _charHolder]) {
	return (_scanner -> _charHolder);
      }
    } else if (_scanner -> _delimiterHolder) {
      if (!skipsLookup[(int) _scanner -> _delimiterHolder]) {
	return (_scanner -> _delimiterHolder);
      }      
    } else {
    }

    while ((ch = getc (_scanner -> _infile)) != EOF) {
      if (!skipsLookup[ch]) {
	if (_scanner -> _delimitersLookup[ch]) {
	  if (!_scanner -> _discard) {
	    _scanner -> _delimiterHolder = ch;
	  }
	} else {
	  _scanner -> _charHolder = ch;
	}

	return (ch);
      }
    }
  } else {
    if (_scanner -> _charHolder) {
      if (skipsLookup[(int) _scanner -> _charHolder]) {
	return (_scanner -> _charHolder);
      } 
    } else if (_scanner -> _delimiterHolder) {
      if (skipsLookup[(int) _scanner -> _delimiterHolder]) {
	return (_scanner -> _delimiterHolder);
      }
    } else {
    }

    while ((ch = getc (_scanner -> _infile)) != EOF) {
      if (skipsLookup[ch]) {
	if (_scanner -> _delimitersLookup[ch]) {
	  if (!_scanner -> _discard) {
	    _scanner -> _delimiterHolder = ch;
	  }
	} else {
	  _scanner -> _charHolder = ch;
	}

	return (ch);
      }
    }
  }

  return (ch);
}

