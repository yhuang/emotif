/*
 * File:  emotif-maker.cc
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


#include <assert.h>
#include <ctype.h>
#include <fstream.h>
#include <math.h>
#include <string.h>
#include <iostream.h>

/*
 * ----------
 * The file unistd.h exports the function getopt.  The function getopt can
 * get option letters from an argument vector.
 * ----------
 */

#include <unistd.h> 


/*
 * ----------
 * Original Comment:  [The file dejavu.h] implements the hash table that
 *                    records whether a set of sequences has been visited
 *                    before.
 * ==========
 * 
 * ----------
 */

#include "dejavu.h"


/*
 * ----------
 * Original Comment:  Hard limits; should really remove at some point
 * ==========
 *
 * ----------
 */

#define SEQUENCES 5000
#define SLENGTH 5000
#define ALPHABETS 100


/*
 * ----------
 * The type dejavus is defined in the file dejavu.h.  The following line of
 * code declares a global array of pointers of type dejavus called dejavu.  The
 * variable dejavu now holds the address of this global array.  The constant
 * SEQUENCES has been set to 1000.
 * ----------
 */

dejavus *dejavu[SEQUENCES];


/*
 * ----------
 * The modifier ul is just a short-hand for the modifier unsigned long.  The
 * global array that the pointer called zero points to, therefore, has a
 * constant SEQUENCES number of unsigned long elements.  The constant
 * SEQUENCES has been set to 1000.
 * ----------
 */

ul zero[SEQUENCES];


/*
 * ----------
 * As declared below, the variable pp is essentially a pointer to some yet-to-
 * be-defined constant object of type double.  While whatever pp points to
 * may not change because the object pp points to must remain constant, the
 * variable pp itself may change.  The variable pp, in fact, may be changed to
 * point to a different variable of type double, but the value of the objects
 * the variable pp points to can never be modified.  The qualifier extern 
 * in the declaration below pledges to the program that the definition of the
 * object the pointer pp points to will be defined elsewhere.  In this case
 * the definition is located at the end of this file.
 * ----------
 */

extern const double pp[];


/*
 * ----------
 * The variable sequence is a pointer to a global 2D integer array.  The
 * modifier long, when used back-to-back as shown below, declares each element
 * in the 2D integer array to be of 64-bit, instead of the usual 32-bit with a
 * single long modifier.  The variable alphabet is a pointer to a global
 * integer array, in which each element, as explained, is a 64-bit integer.
 * The constant SEQUENCES has been set 1000; the constant SLENGTH has been set
 * to 2500; and the constant ALPHABETS has been set to 100.
 * ----------
 */

static unsigned long long 
  sequence[SEQUENCES][SLENGTH],  // each sequence expressed as a bit-vector
  alphabet[ALPHABETS];           // the alphabets expressed as a bit-vector


static char as[SEQUENCES][20],  // alphabets as text
  ss[SEQUENCES][SLENGTH];       // sequences as text


/*
 * ----------
 * Original Comment:  [The global 2D integer array count keeps track of the]
 *                    occurrences of group in each column.  this gets modified
 *                    (decremented) going down the tree, and gets restored
 *                    coming back up; it's pretty dynamic.
 * ==========
 * 
 */

static int count[SLENGTH][ALPHABETS]; 


/*
 * ----------
 * Original Comment:  [The following global variables keep track of the]
 *                    statistics for the sequences.
 * ==========
 *
 */

static double p[ALPHABETS],
  l[ALPHABETS];


/*
 * ----------
 * Original Comment:  [The following global variables keep track of] which
 *                    sequences are currently matched.
 * ==========
 *
 */

static ul included[SEQUENCES];
static int num_included;



int a,       // number of groups
    s,       // number of sequences
    length;  // length of sequences


double probability = 1;
int stringency = 0;
int minimum = 30;

static int match[300][300], mi[300];
double min_probability[1000];


/* Function Prototypes */

void Initialize (int argc, char **argv);
void FormMotifs (int left);



main (int argc, char **argv)
{
  Initialize (argc, argv);
  FormMotifs (0);  // left is 0 initially

  return (0);  // This line is added to let the Sun machines know that everything is okay.
}


/*
 * ----------
 * Original Comment:  [The subroutine initialize reads] the alphabet and
 *                    sequence files.
 * ==========
 * 
 */

void Initialize (int argc, char **argv)
{
  int i, j, c;


  /*
   * ----------
   * The local variable alphabet_f of type char* points to the constant string
   * "default".  The constant string "default" is the filename of a file
   * Craig's emotif needs.  The purpose this file default serves will be
   * explained later.
   * ----------
   */

  char *alphabet_f = "default";


  /*
   * ----------
   * The function getopt is a function the file unistd.h exports.  The
   * prototype of the getopt function is shown below:
   *
   *      int getopt (int argc, char * const argv[], const char *optstring);
   *
   * The getopt function returns the next option letter in the command-line
   * string the pointer argv points to that matches a letter in optstring.
   * The variable optstring is, again, a pointer to a constant character
   * object.  While the value of optstring may change, the value of whatever 
   * optstring points to may not.  The pointer opstring must point to a string
   * that contains the option letters the command using getopt will recognize.
   * If a letter is followed by a colon, the option is expected to have an
   * argument, which may be separated from it by a white space.  The extern
   * variable optarg of type char* points to the argument expected by an
   * option.  The function getopt is essentially responsible for directing the
   * exported pointer optarg of type char to the user-supplied argument.  The
   * function getopt returns -1 when all command line options have been parsed.
   * ----------
   */
  
  while ((c = getopt (argc, argv, "c:s:a:")) != -1) {


    /*
     * ----------
     * The program emotif has 3 different options associated with it.  If
     * an option does not expect any argument, it will not be followed by a
     * semicolon.  The function atoi is a function exported by the file 
     * stdlib.h.  The atoi function takes a pointer to a constant character
     * object and returns the integer value represented by that constant
     * character object.  The following line of code is an example of the
     * function atoi's usage:
     *
     *      int integerValue = atoi ("10");
     *
     * The value of integerValue will be 10.
     * ----------
     */

    switch (c) {
    case 'c': minimum = atoi (optarg);
      break;

      
      /*
       * ----------
       * When the script emotif-3.6/build/emotif-build.pl calls the
       * emotif program, the only option the script activates is -s.
       * The option -s is followed by an integer value that reflects
       * the specificity of emotifs to be
       * generated.
       * ----------
       */

    case 's': stringency = atoi (optarg);
      break;


      /*
       * ----------
       * When option -a is turned on, Craig's emotif expects the name of the
       * alphabet file to follow.  The local variable alphabet_f, which is a
       * pointer of type char, will be set to what optarg is pointing to.  The
       * pointer optarg of type char is exported by the file unistd.h; the
       * pointer optarg points to the name of the alphabet file.
       * ----------
       */

    case 'a': alphabet_f = optarg;
      break;
    }  /* switch (c) */
  }  /* while ((c = getopt (argc, argv, "qt:g:a:e:")) != -1) */


  /*
   * ----------
   * The variable alphbet_file is a pointer to a local array of 500 char
   * elements.  This pointer points to a global char array each of whose
   * elements will contain a letter from the complete name of the alphabet
   * file used by Craig's emotif program.  The complete name of this alphabet
   * file,  therefore, must not exceed 500 characters.  The alphabet file
   * essentially contains all the amino acids and all the statistically
   * significant substitution groups to be used by Craig's emotif program in
   * constructing the emotif regular expressions.
   * ----------
   */

  char alphabet_file[500];


  /* 
   * ----------
   * The variable alphabet_f is a pointer of type char.  At the beginning of
   * this subroutine (initialize), th pointer alphabet_f is initialized to
   * where the constant string "default" resides.  If option -a has been
   * turned on, the pointer alphabet_f will point to the filename of a
   * user-specified alphabet file instead of the default one.  If the string
   * alphabet_f points to contains a '/', the function strchr exported by the
   * string.h file will return a non-zero value, and the first char element of
   * the local array the pointer alphabet_file points to will be set to an
   * ASCII value of zero.  Setting the first element of the local array to an
   * ASCII value of zero is effectively the same as setting the local pointer
   * alphabet_file of type char to NULL.  If no '/' character is detected by
   * the function strchr, each letter in the constant string
   * "/usr/people/cnevill/emotif/alphabets/" will be copied to the local
   * character array the pointer alphabet_file points to.  Remember the strchr
   * function automatically NULL (\0) terminates the string it puts into
   * the local array the pointer alphabet_file points to.
   * ----------
   */

  if (strchr (alphabet_f, '/')) {
    alphabet_file[0] = 0;
  } else /* if (strchr (alphabet_f, '/')) */ {
    strcpy (alphabet_file, "/usr/share/emotif/");
  }  /* if (strchr (alphabet_f, '/')) */


  /*
   * ----------
   * Whatever the local array the pointer alphabet_f points to may hold, the
   * elements of type char in that local array will be concatenated to the
   * content of the local array the pointer alphabet_file points to by the
   * string.h-exported function strcat.  If the user gives the complete name
   * of the desired alphabet file at the command line (which must contain '/'
   * characters), the local array the pointer alphabet_file points to will
   * simply be the NULL (\0) character, whose ASCII value is 0; if the user
   * only gives the name of the alphabet file but no directory information,
   * Craig's emotif program will assume that the file exists in the directory
   * /usr/people/cnevill/emotif/alphabets/.  The strcat function will also
   * automatically NULL (\0) terminate the final string it puts into the local
   * array the pointer alphabet_file points to.
   * ----------
   */

  strcat (alphabet_file, alphabet_f);


  /*
   * ----------
   * When doing I/O to disk files, the file version of the stream classes must
   * be used.  These classes are ifstream, ofstream,and fstream; they are
   * exported by the file fstream.h.  C++ allows the open call to be combined
   * with the constructor.  For example, instead of writing:
   *
   *      ifstream data_file;  // input file
   *      data_file.open ("/usr/people/cnevill/emotif/alphabets/default");
   *
   * The two lines of code may be condensed into one by writing:
   *
   *      ifstream data_file ("/usr/people/cnevill/emotif/alphabets/default");
   *
   * The local array the pointer alphabet_file points to now contains the
   * complete name of the alphabet file.  The variable A of class ifstream
   * provides the interface between the alphabet file and Craig's emotif
   * program.
   * ----------
   */

  ifstream A (alphabet_file);


  /*
   * ----------
   * In the interest of speed Craig's emotif program does not check to see if
   * problems are encountered during file opening.  A safer version would
   * probably include the following code snippet:
   *
   *      if (A.bad ()) {
   *        cerr << "Error:  Could not open <alphabet_file>\n";
   *        exit (8);
   *      }
   * ----------
   */


  /*
   * ----------
   * The global variable mi points to a global integer array with 300
   * elements.  Each of the 300 elements is initialized to 0 in the following
   * for loop.
   * ----------
   */

  for (i = 0; i < 300; i ++) {
    mi[i] = 0;
  }  /* for (i = 0; i < 300; i ++) */


  /*
   * ----------
   * The for loop below:
   *
   *      for (a = 0; ; a++) {
   *                 .
   *                 .
   *                 .
   *      }
   *
   * is essentially a while loop:
   *
   *      a = 0;
   *
   *      while (1) {
   *        if (A.eof ()) {
   *          break;
   *        }    
   *             .
   *             .
   *             .
   *        a++;
   *      } 
   *
   * The variable a of type int is, as Craig explained after the variable a's
   * declaration, keeps track of the number of groups.  The alphabet file,
   * /usr/local/cnevill/emotif/alphabets/default, looks like the following:
   *
   * A
   * C
   * D
   * .
   * .
   * .
   * FILMV
   * FILVY
   * FILMVY
   *
   * As shown above, each line contains either a single amino acid or a
   * substitution group.  Since the integer variable a keeps track of the
   * number of lines in the alphabet file, the value the integer variable a
   * holds, after the end-of-file character has been encountered, is the
   * total number of groups, alphabets, or single amino acids plus
   * substitution groups, used by Craig's emotif program.
   * ----------
   */
  
  for (a = 0; ; a++) {


    /*
     * ----------
     * The member function getline is defined as:
     *
     *      ifstream &getline (char* as[s], int len, char delim = '\n');
     *
     * As shown above, the getline member function takes three arguments.
     * Since the line of code below only has two parameters, the third
     * parameter defaults to the newline character '\n'.  The getline
     * function is essentially responsible for taking one line from the
     * alphabet file and putting it in the global array the pointer as[a]
     * points to.  The delimiting character '\n' will not go into the global
     * array the pointer as[a] points to; the character element the newline
     * character would have gone gets a NULL (\0) character instead.  The
     * global variable as, as Craig explained after its declaration,
     * is for the alphabets as text.  The global 2D array the pointer as of
     * type char* points to is statically allocated 1000 (the value of the
     * constant SEQUENCES) elements one dimension and 20 (the number of amino
     * acids) elements the other direction.  The number of amino acids plus
     * the number of substitution groups in the file 
     * /usr/local/cnevill/emotif/alphabets/default is 40, which is clearly
     * less than 1000, and the number of amino acids in a substitution group
     * cannot possibly be more than 20.  The member function getline will
     * automatically terminate the string it puts into the global array of size
     * 20 with a NULL (\0) character.
     * ----------
     */

    A.getline (as[a], 20);


    /*
     * ----------
     * Once the end-of-file character has been reached, A.eof will return true,
     * and the program will exit the for (a = 0; ; a++) loop.
     * ----------
     */

    if (A.eof ()) {
      break;
    }  /* if (A.eof ()) */

    p[a] = 0;


    /*
     * ----------
     * The global array of type unsigned long long (64-bit integer) the
     * pointer alphabet points to has been declared statically to be of size
     * 100 (the value of the constant ALPHABETS).  The file
     * /usr/local/cnevill/emotif/alphabets/default, for example, has a total of
     * 40 lines of single amino acids and substitution groups; only the
     * first 40 elements, element 0 through 39, of the global 64-bit integer
     * array will, therefore, be set to the proper bit-pattern that reflects
     * the identity of the single amino acid or the identify of the
     * substitution group.  Before each 64-bit integer in that global array,
     * alphabet[a], is set to the appropriate value, alphabet[a] is intialized
     * to 0.
     * ----------
     */

    alphabet[a] = 0;


    /*
     * ----------
     * The following for loop is responsible for setting each of the 64-bit bit
     * arrays in the global array the pointer alphabet points to its proper
     * bit-pattern as dictated by the identity of the associated single amino
     * acid or the identity of the associated substitution group.  The for
     * loop iterates through each amino acid letter in the global char array
     * the pointer as[a] points to (Note:  The value of the integer variable a
     * will not change during the tenure of the for loop below.).  Since the
     * getline member function used to read in a line from the alphabet file
     * always puts a NULL (\0) character after the last amino acid character in
     * a substitution group, the encounter of the NULL (\0) character will kick
     * Craig's emotif program out of the for loop (as[a][i] == '\0').
     * ----------
     */

    for (i = 0; as[a][i]; i++) {


      /*
       * ----------
       * The character variable of the ith element in the global array the
       * pointer as[a] points to is initially in upper case, since all the
       * single amino acids and the substitution groups in the alphabet file
       * are in capital letters.  Because Craig chose to represent the emotif
       * regular expressions using lower case letters, the ctype.h exported
       * function tolower is called on the character variable the pointer
       * as[a][i] points to from upper case to lower case.
       * ----------
       */

      as[a][i] = tolower (as[a][i]);


      /*
       * ----------
       * The following two lines are basically responsible for "turning on all
       * of the bits" in the 64-bit bit array for a particular line from the
       * alphabet file.  If the line from the alphabet file contains only one
       * letter, the for loop in progress, for (i = 0; as[a][i]; i++), will
       * stop after only one iteration, and the bit associated with that amino
       * acid letter in the 64-bit bit array will be turned on; if the line
       * from the alphabet file contains more than one letter, the for loop
       * will iterate through however many times there are amino acid letters,
       * and the bits associated with all the amino acid letters in the 64-bit
       * bit array will all be turned on at the end of the for loop.  The
       * integer variable n holds the result of the expression 
       * (as[a][i] - 'a'), which is the difference between the ASCII value
       * associated with the character the pointer as[a][i] points to and the
       * ASCII value of the character 'a'.  The possible values for this
       * difference ranges from 0, as in the case when as[a][i] == 'a', to 24,
       * as in the case when as[a][i] == 'y'.  Once the value of the
       * expression (as[a][i] - 'a') is stored in the integer variable n, the
       * integer variable n can be processed by the shift (<<) operator and
       * the |= operator.  The |= operator and the shift (<<) operator means
       * the same as its C counterpart.  The value of the difference, stored 
       * in the integer variable n, basically dictates which element in the
       * 64-bit global bit array the pointer alphabet[a] points to is turned
       * on.  If the character the pointer as[a][i] points to is 'a', the
       * first bit, or bit 0, will be initialized to 1; if the character the
       * pointer as[a][i] points to is 'y', the 25th bit, or bit 24, will be
       * initialized to 1.  The 64-bit bit array, as explained earlier, is
       * created by declaring variables of type unsigned long long.  The
       * resulting bit patterns of 0s and 1s, therefore, do correspond to
       * concrete integer values.  The |= operator basically ensures that
       * whatever bits were turned on in the previous iteration of the for
       * loop remain on.
       * ----------
       */

      int n = as[a][i] - 'a';
      alphabet[a] |= 1 << n;


      /*
       * ----------
       * The variable p is a pointer to a global integer array of size
       * ALPHABETS.  The constant ALPHABETS has been set to 100.  As the for
       * loop, for (i = 0; as[a][i]; i ++), iterates from 0 to however many
       * amino acids are in a particular alphabet, the occupancy probability
       * of each alphabet is calculated.  The integer variable n, which,
       * again, can have values anywhere from 0 to 24, indicates which
       * element in the global double array that the pointer pp points to. 
       * Every element, except for element 1, element 9, element 14, element
       * 20, and element 23, in that global double array corresponds to the
       * occupancy probability of a particular amino acid in the Swiss-Prot
       * database.  The global array that the pointer pp points to is declared
       * and initialized at the end of emotif.cc.  If a particular alphabet
       * comprises only one amino acid, that alphabet's occupancy probability
       * will be the same as the occupancy probability of that one amino acid
       * in the current release of the Swiss-Prot database; if a particular
       * alphabet comprises two or more amino acids, that alphabet's
       * occupancy probability will be the sum of all the individual
       * occupancy probability of each amino acid.
       * ----------
       */

      p[a] += pp[n];


      /*
       * ----------
       * The variable mi is a pointer to a global integer array of size 300.
       * The value of each element in the array the pointer mi points to,
       * initialized to 0, keeps track of the total number of different
       * alphabets a particular amino acid appears in.  The value of each
       * element in the array the pointer mi points to, in addition, is the
       * column index for the 2D array the pointer match points to.  The
       * integer variable a is a number that corresponds to a particular
       * alphabet.  The integer variable n is a number that corresponds to
       * a particular amino acid.  (Not done).
       * ----------
       */

      match[n][mi[n]++] = a;
    }  /* for (i = 0; as[a][i]; i++) */


    /*
     * ----------
     * Element a in the global double array the pointer p points to now holds
     * the occupancy probability of alphabet a.   Exported by the file
     * math.h, the log10 function returns the log base 10 value of alphabet
     * a's occupancy probability.
     * ----------
     */

    l[a] = log10 (p[a]);
  }  /* for (a = 0; ; a++) */


  /*
   * ----------
   * Although the integer variable a, which, again, keeps track the number of
   * lines in the alphabet file, can be anywhere legally from 0 to 99 (The
   * value of the integer constant SEQUENCES has been set to 100.), Craig
   * demands the value of the integer variable a to be less than 64.  The
   * size of each bit-vector is 8 bytes, or 64 bit.
   * ----------
   */

  assert (a < 64);


  /*
   * ----------
   * The for loop below:
   *
   *      for (s = 0; ; s++) {
   *                 .
   *                 .
   *                 .
   *      }
   *
   * is essentially a while loop:
   *
   *      s = 0;
   *
   *      while (1) {
   *        if (cin.eof ()) {
   *          break;
   *        }
   *             .
   *             .
   *             .
   *        s++;
   *      } 
   *
   * The variable s of type int is, as Craig explained after the variable s's
   * declaration, keeps track of the number of sequences.  The file containing
   * the locally aligned sequences is fed to the emotif program by the
   * emotif-3.6/build/emotif-build.pl script.  The name of that file is
   * /tmp/$$.block ($$ is special Perl variable that refers to the process
   * number assigned to the script emotif-3.6/build/emotif-build.pl), and
   * its content is available to the emotif program through the STANDARD INPUT.
   * ----------
   */

  for (s = 0; ; s++) { 


    /*
     * ----------
     * The member function getline is defined as:
     *
     *      ifstream &getline (char* ss[s], int len, char delim = '\n');
     *
     * As shown above, the getline member function takes three arguments.
     * Since the line of code below only has two parameters, the third
     * parameter defaults to the newline character '\n'.  The getline
     * function is essentially responsible for taking one line from the
     * alphabet file and putting it in the global array the pointer ss[s]
     * points to.  The delimiting character '\n' will not go into the global
     * array the pointer ss[s] points to; the character element the newline
     * character would have gone gets a NULL (\0) character instead.  The
     * global variable as, as Craig explained after its declaration,
     * is for the alphabets as text.  The global 2D array the pointer as of
     * type char* points to is statically allocated 1000 (the value of the
     * constant SEQUENCES) elements one dimension and 2500 (the length of each
     * sequence) elements the other direction.  A block of locally aligned
     * sequences must, therefore, be no longer than 2500 amino-acid long, and
     * the number of the aligned sequences must not exceed 1000.  The member
     * function getline will automatically terminate the string it puts into
     * the global array of size 2500 with a NULL (\0) character.
     * ----------
     */

    cin.getline (ss[s], SLENGTH);


    /*
     * ----------
     * Once the end-of-file character has been reached, cin.eof will return
     * true, and the program will exit the for (s = 0; ; s++) loop.
     * ----------
     */

    if (cin.eof ()) {
      break;
    }  /* if (cin.eof ()) */

    for (i = 0; ss[s][i]; i++) {


      /*
       * ----------
       * The element (s, i) in the 2D global array the pointer ss points to is
       * initially in upper case.  Because Craig chose to represent the emotif
       * regular expressions using lower case letters, the ctype.h exported
       * function tolower is called on the character variable the pointer
       * ss[s][i] points to from upper case to lower case.
       * ----------
       */

      ss[s][i] = tolower (ss[s][i]);


      /*
       * ----------
       * The global array of type unsigned long long (64-bit integer) the
       * pointer sequence points to has been declared statically to be of size
       * 1000 (the value of the constant SEQUENCES).  The number of locally
       * aligned sequences in a given block must, as a result, not exceed
       * 1000.  The row index s specifies which sequence in the block of
       * locally aligned sequences is being processed; the column index i,
       * on the other hand, indicates which column in that block of locally
       * aligned sequences is being considered.  The 64-bit integer
       * sequence[s][i] is a concrete integer value whose bit representation
       * reflects the identity of the amino acid letter at position i
       * for sequence s.
       *
       * The following line of code is basically responsible for "turning on
       * all of the bits" in the 64-bit bit array for a particular sequence s
       * from the block in the /tmp/$$.block file ($$ is a special Perl
       * variable that refers to the process number assigned to the script
       * emotif-3.6/build/emotif-build.pl.  The for loop in progress,
       * for (i = 0; ss[s][i]; i++), will iterate through each position i in
       * the sequence and the bit associated with a particular amino acid
       * letter at position i in the 64-bit bit array will be turned on.
       * The for loop will iterate through however many times there are amino
       * acid letters remaining in the sequence.  The result of the
       * expression (ss[s][i] - 'a'), which is the difference between the
       * ASCII value associated with the character the pointer ss[s][i]
       * points to and the ASCII value of the character 'a'.  The possible
       * values for this difference ranges from 0, as in the case when
       * ss[s][i] == 'a', to 24, as in the case when ss[s][i] == 'y'.  The
       * value of the difference basically dictates which element in the
       * 64-bit global bit array the pointer sequence[s][i] points to is
       * turned on by the shift (<<) operator.  If the character the pointer
       * ss[s][i] points to is 'a', the first bit, or bit 0, will be
       * initialized to 1; if the character the pointer as[a][i] points to is
       * 'y', the 25th bit, or bit 24, will be initialized to 1.  The 64-bit
       * bit array, as explained earlier, is created by declaring variables
       * of type unsigned long long.  The resulting bit patterns of 0s and
       * 1s, again, do correspond to concrete integer values.
       * ----------
       */

      sequence[s][i] = 1 << (ss[s][i] - 'a');


      /*
       * ----------
       * The following for loop, for (j = 0; j < a; j++), iterates through
       * each alphabet found in the alphabet file.  Since the script
       * emotif-3.6/build/emotif-build.pl did not specify a preferred
       * alphabet file, the alphabet file chosen is
       * emotif-3.6/share/default.  The amino acid methionine, for
       * example, will have a j value of 10, because it is the 11th amino acid
       * in emotif-3.6/share/default.  Methionine, however, appears in
       * other alphabets as well; it shows up in the substitution groups
       * [ILMV], [FILMV], and [FILMVY].  The j index value that corresponds to
       * those three alphabets are 35, 37, and 39, respectively.
       * ----------
       */

      for (j = 0; j < a; j++) {


	/*
	 * ----------
	 * The variable sequence points to a global 2D array of type unsigned
	 * long long (64-bit integer).  The row index s specifies which
	 * sequence in the block of locally aligned sequences is being
	 * processed; the column index i, on the other hand, indicates which
	 * column in that block of locally aligned sequences is being
	 * considered.  The 64-bit integer sequence[s][i] is a concrete
	 * integer value whose bit representation reflects the identity
	 * of the amino acid letter at position i for sequence s.
	 * The global array of type unsigned long long (64-bit integer) the
	 * pointer alphabet points to has been declared statically to be of
	 * size 100 (the value of the constant ALPHABETS).  The file
	 * emotif-3.6/share/default, for example, has a
	 * total of 40 lines of single amino acids and substitution groups;
	 * only the first 40 elements, element 0 through 39, of the global
	 * 64-bit integer array will, therefore, be set to the proper
	 * bit-pattern that reflects the identity of the single amino acid
	 * or the identify of the substitution group.  The bit-AND (&)
	 * operator compares two bits.  If they are both 1, the result is 1.
	 * When two 64-bit variables (unsigned long long) are "bit-ANDed"
	 * together, as shown in the expression
	 * (sequence[s][i] & alphabet[j]), the bit-AND operator works on
	 * each bit independently.  The result of all 64 bit-AND operations
	 * is a 64-bit integer in its own right, and this 64-bit integer is
	 * processed by the if statement.  The only way for the if statement
	 * to be true is if the 64-bit integer that is result of all 64
	 * bit-AND operations is non-zero.
	 * ----------
	 */

	if (sequence[s][i] & alphabet[j]) {


	  /*
	   * ----------
	   * The pointer count points to a global 2D integer array.  The row
	   * index i indicates which column in the block of locally aligned
	   * sequence is being processed; the column index j, on the other
	   * hand, specifies which alphabet is under investigation.  If the
	   * amino acid found in sequence s and column i belongs to the
	   * alphabet j, count[i][j] will be incremented by one to reflect
	   * the fact that one more sequence has been found to contain the
	   * alphabet j.
	   * ----------
	   */

	  count[i][j]++;
	}  /* if (sequence[s][i] & alphabet[j]) */
      }  /* for (j = 0; j < a; j++) */
    }  /* for (i = 0; ss[s][i]; i++) */


    /*
     * ----------
     * At the end of the for loop, for (i = 0; ss[s][i]; i++), the integer
     * variable i will be the block width of the locally aligned sequences.
     * ----------
     */

    length = i;
  }  /* for (s = 0; ; s++) */


  /*
   * ----------
   * At this point of the emotif program, all the locally aligned sequences
   * in a block have been read, and the amino acid composition of each column
   * in the block has been properly recorded.
   * ----------
   */


  /*
   * ----------
   * The variable minimum of type int is initially set to 30.  After the
   * following line of code, the value of minimum will be 30% of whatever 
   * value the variable s of type int has.  The following lines of code
   * illustrate a common C/C++ pitfall:
   *
   *      int minimum = 30;
   *      int s = 75;
   *
   *      minimum = (75) * (30) / 100;
   *
   * Since minimum is of type int and s is of type int as well, the final
   * value of minimum will be 22, not 22.5.
   * ----------
   */

  minimum = s * minimum / 100;

  
  /*
   * ----------
   * The size of ul, or unsigned long, is 4 bytes on the SGI Indigo2 (dna). 
   * One chunk therefore, has 32 bytes.  The block of locally aligned
   * sequences will be divided into chunks.  If a block contains fewer than
   * 32 sequences, the variable chunk of type int will be set to 1; if a
   * block contains 32 sequences or more but fewer than 64 sequences, chunk
   * will be set to 2.
   * ----------
   */

  chunk = s / (sizeof (ul) * 8) + 1;


  /*
   * ----------
   * The for loop below, for (i = 0; i < chunk; i++), will iterate through
   * all the chunks a given block of locally aligned sequences can be
   * repartitioned into.
   * ----------
   */

  for (i = 0; i < chunk; i++) {


    /*
     * ----------
     * The pointer zero points to a global unsigned long array of size 1000
     * (the value of the constant SEQUENCES).  For a chunk number of chunks,
     * zero[0] through zero[chunk - 1] will be initialized to 0.  The element
     * zero[0] is a variable of type unsigned long.  Since an unsigned long
     * variable has 4 bytes, initializing zero[0] to 0 is the same as setting
     * each bit of zero[0]'s 32 bits to 0s.  The size of this global integer
     * array's being 1000 sets an implicit limit on the number of chunks,
     * which, in turn, sets the maximum number of sequences the emotif program
     * can accept.  Since each chunk contains 32 sequences, the maximum number
     * of sequences the emotif program can take is 32,000.
     * ----------
     */

    zero[i] = 0;
  }  /* for (i = 0; i < chunk; i++) */


  /*
   * ----------
   * The prototype of the memcpy function, exported by the string.h file, is
   * shown below:
   *
   *      void *memcpy (void *s, const void *ct, size_t n);
   *
   * The pointer included points to a global unsigned long array of size 1000
   * (the value of constant SEQUENCES), and the pointer zero points to a
   * global unsigned long array of size 1000 (the value of constant
   * SEQUENCES).  The parameter n in the prototype is the number of bytes. 
   * The number of bytes the memcpy function will copy from the global
   * unsigned long array zero points to to the global unsigned long array
   * included points to is (chunk * sizeof (ul)).  The bit patterns for those
   * (chunk * sizeof (ul)) bytes will be copied efficiently such that the
   * first (chunk * sizeof (ul) * 8) bits in the global unsigned long array
   * included points to are all 0.
   * ----------
   */

  memcpy (included, zero, chunk * sizeof (ul));


  /*
   * ----------
   * The following for loop, for (i = 0; i < s; i++), iterates from 0 to s,
   * which is the number of the number of sequences in the file
   * /tmp/$$.block passed in by the emotif-3.6/build/emotif-build.pl
   * script via STANDARD INPUT ($$ is a special Perl variable that refers to
   * the process number assigned to the script
   * emotif-3.6/build/emotif-build.pl).
   * ----------
   */

  for (i = 0; i < s; i++) {


    /*
     * ----------
     * The integer variable minimum has already been set 30% of s, the total
     * number of locally aligned sequences in a block.  The if statement
     * below, if (i >= minimum), is Craig's way of making sure that dejavus
     * objects are created only for subsets with at least minimum sequences.
     * The pointer dejavu points to a global dejavus* array of size 1000
     * (the value of constant SEQUENCES).   The dejavus objects are
     * essentially hashtables that allow the emotif program to check whether
     * or not a particular subset of sequences has already been processed.
     * Not all of the elements in the global dejavus* array are initialized;
     * dejavu[0] through dejavu[minimum - 1] will never get initialized.
     * The integer index value i, in the context of the global dejavus* array
     * that the pointer dejavu points to, does NOT refer to the order of a
     * particular sequence in the block; rather the variable i of type int
     * reflects the number of sequences in a particular subset.  Unless a
     * subset has minimum or more sequences, the emotif program will not
     * devote time to create the dejavus hashtables for those subsets.
     * ----------
     */

    if (i >= minimum) {
      dejavu[i] = new dejavus(i);
    }  /* if (i >= minimum) */


    /*
     * ----------
     * The pseudofuncton SET is #defined in the dejavu.h file.  The 
     * preprocessor will basically replace the line:
     *
     *      SET(included, i);
     *
     * with:
     *
     *      included[i / (sizeof(ul) * 8)] |= 1 << (i % sizeof(ul) * 8);
     *
     * The size of ul, or unsigned long, is 4 bytes on the SGI Indigo2 (dna),
     * and the global unsigned long array that the pointer included points to
     * is of size 1000 (the value of the constant SEQUENCES).  The integer 
     * index i, in the context of the global unsigned long array, indicates
     * which chunk a particular sequence in the block resides in.  The 33th
     * sequence, for example, would reside in chunk 1 on an SGI Indigo2.  The
     * 33th sequence would have an index number of 32.  The quotient of
     * 32 / (4 * 8) is 1.  To include a sequence in a subset of sequences,
     * the bit corresponding to that sequence in chunk 1, or included[1],
     * must be turned on.  To include the 33th sequence, the 0th bit of
     * included[1] should be set to 1 with the help of the shift operator
     * (<<), since the expression (32 % sizeof(ul) * 8) equals to 0. 
     * Initially every sequence in the file /tmp/$$.block is included ($$
     * is a special Perl variable that refers to the process number assigned
     * to the script emotif-3.6/build/emotif-build.pl).
     * ----------
     */

    SET(included, i);
  }  /* for (i = 0; i < s; i++) */


  dejavu[s] = new dejavus(s);


  /*
   * ----------
   * The global variable num_included of type int is initialized to the value
   * of s, anoter global variable of type int.  While the value of s will
   * remain the same throughout the duration of the emotif program, the
   * value of num_included is expected to change.  The variable s exists to
   * keep track of the number of sequences in the file /tmp/$$.block passed
   * in by the emotif-3.6/build/emotif-build.pl script via STANDARD INPUT
   * ($$ is a special Perl variable that refers to the process number
   * assigned to the script emotif-3.6/build/emotif-build.pl).  The
   * variable num_included, on the other hand, exists to keep track of the
   * number of sequences in a particular subset of sequences with repect to
   * all the sequences in the /tmp/$$.block file that are used to construct an
   * emotif.  Every sequence in the /tmp/$$.block file is initially included
   * before the emotif program's very first call to recursive function
   * FormMotifs.
   * ----------
   */

  num_included = s;
}


void FormMotifs (int left)
{
  int i, j, k;


  /*
   * ----------
   * Original Comment:  [The following line of code prints]
   *                    out the most specific motif for this subset.
   * ==========
   *
   */


  /*
   * ----------
   * In the script emotif-3.6/build/emotif-build.pl, the line:
   *
   *      ($k, $motif, $p) = split (/\t/, $_);
   *
   * initializes the variables $k, $motif, and $p with the results the emotif
   * program makes available via the STANDARD OUTPUT.  The variable $k refers
   * to the number of sequences used to make an emotif.  In the emotif program
   * that number is stored in the integer variable num_included.  
   * ----------
   */

  cout << num_included << '\t';


  double probability = 0;

  char motif[SEQUENCES];
  int motifi = 0;


  /*
   * ----------
   * The for loop below, for (i = 0; i < length; i++), iterates from the
   * leftmost column (0) in a block to the rightmost column (length - 1)
   * in a block.  The width of a block is stored in the variable length of
   * type int.
   * ----------
   */

  for (i = 0; i < length; i++) {


    /*
     * ----------
     * The following for loop, for (j = 0; j < a; j++), iterates from the
     * index number of the first alphabet (0) in the alphabet file to the
     * index number of the last alphabet (a - 1) in the alphabet file. 
     * The alphabet file the emotif-3.6/build/emotif-build.pl script
     * uses is emotif-3.6/share/default.
     * ----------
     */

    for (j = 0; j < a; j++) {


      /*
       * ----------
       * If every amino acid in column i is alphabet j, the following lines of
       * code will be executed.  Whenever the emotif program encounters an
       * alphabet j such that all num_included sequences have in a given
       * column i, this alphabet j is guaranteed to be the most specific
       * alphabet by virtue of the way the alphabets in the alphabet file is
       * supposed to be arranged.  The alphabet file
       * emotif-3.6/share/default, for example, lists the 20 amino acid
       * letters in alphabetical order first.  Those amino acid letters are
       * then followed by 20 substitution groups, starting with the more
       * specific ones ([IV], [FY], and [HY]) to the less specific ones
       * ([FILMV], [FILVY], AND [FILMVY]).
       * ----------
       */

      if (count[i][j] == num_included) {


	/*
	 * ----------
	 * Element j in the global double array the pointer l points to holds
	 * the log occupancy probability of alphabet j.  The variable
	 * probability of type double will hold the log occupancy probability
	 * of a particular motif.  A motif's ocupancy probability is
	 * calculated by adding up the log occupancy probability of each
	 * position's alphabet in the motif.
	 * ----------
	 */

	probability += l[j];


	/*
	 * ----------
	 * If an alphabet contains more than one amino acid letter, as[j][1]
	 * will not be a NULL (\0) character.  This alphabet, therefore, must
	 * be a substitution group.  To indicate that a particular column is
	 * occupied by a substitution group, the amino acid letters that make
	 * up the substitution group must be enclosed by brackets:
	 *
	 *      [ST], [KQR], [FILMV], and etc.
	 * ----------
	 */

	if (as[j][1]) {
	  motif[motifi++] = '[';


	  /*
	   * ----------
	   * The for loop below, for (int l = 0; as[j][l]; l++), iterates
	   * through each amino acid letter in the global char array
	   * the pointer as[a] points to (Note:  The value of the integer
	   * variable j will not change during the tenure of the for loop
	   * below.).  Since the getline member function used to read in a
	   * line from the alphabet file always puts a NULL (\0) character
	   * after the last amino acid character in a substitution group,
	   * the encounter of the NULL (\0) character will kick Craig's
	   * emotif program out of the for loop (as[j][l] == '\0').
	   * ----------
	   */

	  for (int l = 0; as[j][l]; l++) {
	    motif[motifi++] = as[j][l];
	  }  /* for (int l = 0; as[j][l]; l++) */

	  motif[motifi++] = ']';
	} else /* if (as[j][1]) */ {


	  /* 
	   * ----------
	   * If alphabet j is composed of only one amino acid, as[j][0], or
	   * the ASCII value of that amino acid letter, will be assigned to
	   * element motifi in the local char array of size 1000 the pointer
	   * motif points to.  The value of the integer index motifi will be
	   * incremented by 1.
	   * ----------
	   */

	  motif[motifi++] = as[j][0];
	}  /* if (as[j][l]) */


	/*
	 * ----------
	 * Having found the most specific alphabet capable of representing the
	 * amino acid composition of column i in the block of locally aligned
	 * sequences, the emotif program breaks out of the
	 * for (j = 0; j < a; j++) loop.  The most specific alphabet is
	 * guaranteed to be picked over the less specific alphabets because of
	 * t he way the alphabets in the alphabet file is arranged.  The
	 * alphabet file emotif-3.6/share/default, for example, lists
         * the 20 amino acid letters in alphabetical order first.  Those amino
         * acid letters are then followed by 20 substitution groups, starting
         * with the more specific ones ([IV], [FY], and [HY]) to the less
         * specific ones ([FILMV], [FILVY], AND [FILMVY]).
	 * ----------
	 */

	break;
      }  /* if (count[i][j] == num_included) */
    }  /* for (j = 0; j < a; j++) */


    /*
     * ----------
     * If none of the alphabets can fully cover all num_included sequences,
     * the index variable j becomes equal to a.  The wild card character (.),
     * in such a case, is the best character for describing the amino acid
     * composition in column i of the locally aligned sequence block.
     * ----------
     */
  
    if (j == a) {
      motif[motifi++] = '.';
    }  /* if (j == a) */
  }  /* for (i = 0; i < length; i ++) */

  motif[motifi] = 0;  // NULL (\0) terminate that string.


  // In the line ($k, $motif, $p) = split (/\t/, $_); in
  // emotif-3.6/build/emotif-build.pl,
  // the variable $motif is the string of emotif regular expression, and $p is
  // the specificity associated with this emotif.  Basically, we're sending out
  // all the data emotif-3.6/build/emotif-build.pl needs.

  cout << motif << '\t' << probability;

  cout << endl;


  // The variable stringency indicates specificity.  If the option -s is not turned
  // on, stringency would be set to 0, and we would have to skip the following line
  // of code.  On the other hand if stringency is some non-zero integer value, one
  // of the three requirements would be satisfied.

  // Now probability is actually the log probability of the emotif currently
  // being processed.  Before the probability is logged, it was some decimal
  // value; after logging, probability is a negative value.

  // One last guard condition is that the number of sequences included must be
  // more than the minimum (sensitivity issue), but num_included is always
  // going to be more than minimum...I think.  Well, not necessarily after the
  // first call to the subroutine FormMotifs.

  // minimum is now 30% of whatever.
  // but we want to make sure the motif has max coverage.  Now that we've
  // found one with num_included coverage, we're not interested in a motif
  // that has less coverage than that.  This speeds up things a lot.  

  if (stringency && probability < -stringency && num_included > minimum) {
    minimum = num_included;  // Now minimum is the same as num_included.
  } // if (stringency && probability < -stringency && num_included > minimum)


  /*
   * ----------
   * Original Comment:  [The following lines of code] enumerate groups in this
   *                    position ... could get rid of this for statement by
   *                    allowing '.' in column left (?!).
   * ==========
   *
   */

  for (i = left; i < length; i ++) {  // Iterate through the width of the
    // block.

    for (j = 0; j < a; j ++) {  // Iterate through all the alphabets.


      if (count[i][j] < num_included && count[i][j] >= minimum) {  
	// left is not guaranteed to be zero.  left is passed in, called
	// recursively.  If a subs group does not reduce the num of sequences
	// at all, then we don't consider it.  At each point we're looking for
	// a smaller subset.  Does not trim so much that it goes below the
	// minimum threshold (percentage coverage).  One of the main pruning
	// things.  
	// I guess I don't see how satisfying the two conditions above excludes
	// some of the sequences.  Wait, the counting is done back in the
	// subroutine initialize, so if count[i][j] < num_included, the
	// alphabet j is guaranteed to be only in a subset of sequences.
	// The count[i][j] should also be at least above the minimum number
	// of sequences that corresponds to 30% of the sequences.  The global
	// 2D array count does not tell you which sequences are included and
	// which are excluded; the only thing it tells you is the number of
	// includes, and you can figure out the number of excludes.

	/*
	 * ----------
	 * Original Comment:  [W]e've found a group that excludes some of the
	 *                    sequences, but not too many. We'll use this group
	 *                    to exclude some sequences.
	 * ==========
	 *
	 */
	
	ul save[SEQUENCES];


	memcpy (save, included, chunk * sizeof(ul));  // whatever the previous call the FormMotifs produced gets saved here to the array save.
	

	/*
	 * ----------
	 * Original Comment:  [The following lines of code] check which
	 *                    sequences don't match now.
	 * ==========
	 *
	 */

	for (k = 0; k < s; k ++) {
	  // included[k / (sizeof(ul) * 8)] & (1 << (bit % (sizeof(ul) * 8)));
	  if (QUERY(included, k) && !(sequence[k][i] & alphabet[j])) {
	    // if it's already included, and this position in this sequence
	    // does not match whatever alpha you're looking at, which is j,
	    // clear the bit.  This clears up all the sequences that don't
	    // have the subs group in question.

	    // If the bit at the position QUERY inquires about is turned on,
	    // and if the alphabet j we're currently on is different from the
	    // amino acid at position i of the kth sequence.  The exclusion
	    // is about to come up in the next line of code.

// included[k / (sizeof(ul) * 8)] &= ~(1 << (bit % (sizeof(ul) * 8)));
	    CLEAR(included, k);
	  }  // if (QUERY(included, k) && !(sequence[k][i] & alphabet[j])
	}  // for (k = 0; k < s; k ++)


	/*
	 * ----------
	 * Original Comment:  [I]f we've seen this subset before, reset the
	 *                    included list and try the next group.
	 * ==========
	 *
	 */

	if (dejavu[count[i][j]] -> exists(included)) {

	  // The value stored in count[i][j] reflects the number of sequences
	  // that have a particular alphabet j.  The function exists basically
	  // check to see if the subset of sequences that have the alphabet j
	  // at position i has already been seen.  The way Craig checked for
	  // this possibility is with the help of the dejavu object.  I will
	  // get to the dejavu.h later.  If the exists function returns true,
	  // whatever is in save, which is the previous never-before-seen
	  // subset of sequences, will get copied over to included.
	  // back out of here!  Perhaps going to the next possibility of the
	  // alphabets.

	  memcpy(included, save, chunk * sizeof(ul));
	  continue;  // So we move onto the next alpha.
	}  // if (dejavu[count[i][j]] -> exists(included))


	/*
	 * ----------
	 * Original Comment:  [The following lines of code] update the counts 
	 *                    based on excluding the sequences.
	 * ==========
	 *
	 */
	

	// At this point, you're guaranteed to have excluded some sequences.  

	for (k = 0; k < s; k++) {  // going through all the sequences.

	  // quick check to see which sequences have just been excluded.

	  if (!QUERY(included, k) && QUERY(save, k)) {  
	    // if a sequence is not already included but is already part of
	    // the save bit
	    // included[k / (sizeof(ul) * 8)] & (1 << (bit % (sizeof(ul) * 8)));
	    // save[k / (sizeof(ul) * 8)] & (1 << (bit % (sizeof(ul) * 8)));

	    num_included--;  // demonstrate that coverage is a bit less.
	    
	    for (int l = 0; l < length; l++) {

	      // The line below gives a little clue as to just what this
	      // array called mi is all about.  The amino acid ASCII value
	      // of sequence k at position l minus the ASCII value of 'a' is
	      // the ... hmm, the name mi can mean match index.  

	      // match is all the subs groups this amino acid appears in.

	      for (int m = 0; m < mi[ss[k][l] - 'a']; m++) {
		count[l][match[ss[k][l] - 'a'][m]]--;  
		// looking at a particular AA (0-25).  The column index for
		// match indicates all the subs groups that contain AA in.

		// what is count?  
		// This is the number of sequences that would match if we
		// enforce the subs group in the second index [m].  Count
		// will be updated to the current number of sequences in the
		// subset.  We're trying to remove once sequence in this
		// count array.  And so we have to go through every position
		// in the sequence and consider every subs group that the
		// amino acid in this position belongs to, and decrement the
		// count.  

	   
		// quickly look up all the subs group that amino acid which
		// is ss[k][l] appears in.
	      } // for (int m = 0; m < mi[ss[k][l] - 'a']; m++)
	    } // for (int l = 0; l < length; l++)
	  } // if (!QUERY(included, k) && QUERY(save, k))
	} // for (k = 0; k < s; k++)

	// now we've decided on a subs group for this column, we're ready to
	// move onto the next column.

	/*
	 * ----------
	 * Original Comment:  [The following lines of code] call [the
	 *                    subroutine FormMotifs] recursively to do the
	 *                    other columns.
	 * ==========
	 *
	 */
	
	FormMotifs (i + 1);  // next column.
	
	if (used * chunk * sizeof (ul) > 128000000) {
          exit (1);
        }

	/*
	 * ----------
	 * Original Comment:  [The following lines of code] put everything
	 *                    back the way it was.
	 * ==========
	 *
	 */


	// Before it goes to the next alphabet, it puts back the counts,
	// the included bit map, and num_included.

	for (k = 0; k < s; k++) {
	  if (!QUERY(included, k) && QUERY(save, k)) {
	    for (int l = 0; l < length; l++) {

	      for (int m = 0; m < mi[ss[k][l] - 'a']; m++) {
		count[l][match[ss[k][l] - 'a'][m]]++;
	      }  // for (int m = 0; m < mi[ss[k][l] - 'a']; m++)
	    }  // for (int l = 0; l < length; l++)

	    SET(included, k);
	    num_included++;  
	    // Because you excluded some for this call to FormMotifs, to exit,
	    // you have to put it back.
	  }  // if (!QUERY(included, k) && QUERY(save, k))
	}  // for (k = 0; k < s; k++)

      }  // if (count[i][j] < num_included && count[i][j] >= minimum)

    }  // for (j = 0; j < a; j++)

  }  // for (i = left; i < length; i++)
}


/*
 * ----------
 * The variable pp is a pointer to a global double array of size 25.  This
 * global array is essentially a lookup table for the occupancy probability of
 * a particular amino acid in the latest release of the Swiss-Prot database.
 * The following probability values are from Swiss-Prot release 39.22.  Every
 * element, except for element 1, element 9, element 14, element 20, and 
 * element 23, corresponds to an amino acid.  The double value associated
 * with each element is the corresponding amino acid's occupation probability
 * in the Swiss-Prot database.
 * ----------
 */

const double pp[] = {
0.07621, // A
0.0,     // B 
0.01634, // C 
0.05252, // D
0.06461, // E
0.04103, // F
0.06858, // G
0.02250, // H
0.05858, // I
0.0,     // J
0.05976, // K
0.09528, // L
0.02372, // M
0.04365, // N
0.0,     // O
0.04898, // P
0.03942, // Q
0.05196, // R 
0.07082, // S
0.05590, // T
0.0,     // U
0.06620, // V
0.01212, // W
0.0,     // X
0.03163  // Y
};

