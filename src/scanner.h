/*
 * File:  scanner.h
 * Date:  10-01-98
 * Course:  CS107
 * Grader:  Erik Haugen
 * Programmer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 * This file defines the interface for the Scanner ADT.  The Scanner ADT
 * is a simple token scanner that can parse a file token by token.
 * ------------------------------------------------------------
 */


#ifndef _SCANNER_H_
#define _SCANNER_H_


#include <stdio.h>


#define FALSE 0
#define TRUE 1


/*
 * Scanner type
 * ------------
 * The Scanner ADT is defined as a pointer to a struct that will be
 * implemented in the scanner.c file.  Variables of type Scanner must
 * be initialized with NewScannerFromFilename or NewScannerFromFile.
 * The Scanner is implemented with pointers, so all client copies in
 * variables or parameters will be "shallow," that is, they will all 
 * actually point to the same Scanner structure.
 */

typedef struct ScannerImplementation *Scanner; 


/*
 * NewScannerFromFilename
 * ----------------------
 * This function allocates and return a new Scanner.  The client-supplied
 * filename specifies which file the client wishes to scan.  If the
 * file does not exist or cannot be opened, a NULL pointer is returned.
 * The delimiter string lists the token-delimiting characters.  The
 * int variable discard controls whether the delimiters are returned
 * along with the tokens.  A copy of the delimiter string is made and kept
 * by the scanner.  No limit exists for the number of delimiting
 * characters that the client may give in the delimiter set.
 */

Scanner NewScannerFromFilename(const char *filename, const char *delimiters,
                               int discard);

/*
 * NewScannerFromFilename behaves similarly as the function above, but it
 * is used to scan an already opened file.  If a NULL pointer is passed,
 * a NULL scanner is returned.
 */

Scanner NewScannerFromFile(FILE *fp, const char *delimiters, 
                           int discard);


/*
 * FreeScanner
 * -----------
 * This function frees all the storage for the specified scanner.  If the
 * scanner was created via NewScannerFromFilename and not
 * NewScannerFromFile, this function would close the underlying file.
 * The client cannot continue to use a scanner after it has been passed to
 * FreeScanner.
 */

void FreeScanner(Scanner _scanner);
 

/* 
 * ReadNextToken
 * -------------
 * This function scans characters from the file associated with the scanner
 * and writes them in the client-supplied buffer.  The token written into
 * the buffer is null-terminated by this function.  ReadNextToken returns
 * an int result to indicate whether or not anything has been written
 * into the buffer.  If a valid (i.e. non-empty) token has been found
 * and written into the client's buffer, the function returns TRUE;
 * otherwise, the function returns FALSE.  If the scanner reads the last
 * token and stops at EOF, ReadNextToken will still return true because
 * the buffer now has a token; however, the next call to ReadNextToken
 * on the same scanner will return FALSE (since no token can be formed).
 *
 * How ReadNextToken forms tokens:
 * If the character under scrutiny is a member of the delimiter set,
 * ReadNextToken will form a single-character string of just that
 * delimiter.  When the int variable discard is FALSE, the function
 * will return this delimiter; when the same int variable is TRUE,
 * the delimiter token will be discarded.  If the characters under
 * scrutiny are not members of the delimiter set, they will be written
 * into the client's buffer until the first delimiter character (or EOF)
 * is seen.  The delimiter char remains in the input stream for next
 * call of ReadNextToken.
 *
 * What happens when client buffer is full:
 * The client supplies the buffer and indicates its size.  Should the
 * scanner exhausts the buffer before the delimiter, the token is
 * truncated to fit the client's buffer and still be NULL terminated.
 * The next call to ReadNextToken will pick up where the previous call
 * left off.  The chopping up of long tokens will generate no error
 * messages or warnings.  A scanner supplied with a 10-character buffer
 * will break "antidisestablishmentarianism" into "antidises", then
 * "tablishme", etc. 
 */

int ReadNextToken(Scanner _scanner, char buffer[], int bufLen);


/*
 * SkipOver
 * --------
 * This function skips over characters in the file, if they occur
 * in the set of characters defined by skips.  If the client wishes
 * the scanner to skip over common white-space characters, he or she
 * will define skips as " \n\r\t".  The SkipOver function will continue
 * reading characters from the file stream as long as they are
 * white-space.  As soon as the first non-white-space character is seen
 * by the scanner, the function stops reading.  The character responsible
 * for halting the function will remain in the input stream, so it will
 * be the very next character read.  This character will also be returned
 * in case the caller wishes to know about it.  Hitting the end of file
 * also causes the skipping; in such cases EOF is returned. 
 */

int SkipOver(Scanner _scanner, const char *skips);


/*
 * SkipUntil
 * ---------
 * This function also helps the scanner to skip parts of the file stream
 * as the SkipOver function does.  The SkipUntil function, however, will
 * skip UNTIL the scanner encounters a character in the set of stops.  
 * The SkipUntil function, for example, will jump to the end of a sentence
 * if it were given the set ".!?" as its stops.  As with SkipOver, the
 * halting character is left in the file stream and returned.  EOF will 
 * also cause skipping to come to a grinding halt.  
 */

int SkipUntil(Scanner _scanner, const char *stops);


/*
 * SkipOverDelimiters
 * ------------------
 * This function skips over characters in the file, if they occur
 * in the set of delimiters.
 */

int SkipOverDelimiters(Scanner _scanner);


/*
 * SkipUntilDelimiters
 * -------------------
 * This function also helps the scanner to skip parts of the file stream
 * as the SkipOverDelimiters function does.  The SkipUntilDelimiters 
 * function, however, will skip UNTIL the scanner encounters a character
 * in the set of delimiters.  
 */

int SkipUntilDelimiters(Scanner _scanner);

#endif

