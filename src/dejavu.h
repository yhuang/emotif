/*
 * File:  dejavu.h
 * Date:  March 1, 2002
 * Package:  eMOTIF-3.6
 * Programmer:  Craig Nevill-Manning (http://craig.nevill-manning.com)
 * Comment Writer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 *
 * ------------------------------------------------------------
 * Copyright (c) 2002 
 * All Rights Reserved.
 * The Leland Stanford Junior University Board of Trustees
 */


#include <stdlib.h>
#include <stdio.h>

typedef unsigned long ul;
extern ul zero[];

// Should chunk be extern?  Are global variables extern by default? 1
// chunk = 32 sequences; it is defined in emotif-maker.cc:
// chunk = s / (sizeof (ul) * 8) + 1

int chunk;
int used = 0;
int total = 0;
int queries = 0;
int collisions = 0;

class dejavus {
  ul *p;  // pointer to the sequence list
  int size, occupied, traverse;
  // int number;

  // size of the hash table
  // occupied = how many sequences are in there
  
 public:

  // The int variable n is the number of sequences in a particular
  // subset of sequences in the local sequence alignment.
  dejavus (int n) {

    size = 101;
    occupied = 0;
    traverse = 0;
    //    number = n;
    
    // Since the int variable used is set to 0 initially, after this
    // line the value of used should be 101.
    
    // Comment this out...this was to remember the total memory used
    // for the hashtable.
    used += size;
    
    // chunk is the number of unsigned longs needed for the
    // hashtable p points to.
    p = (ul *) malloc (sizeof (ul) * size * chunk);
    
    // set everything in the hash to zero.
    memset (p, 0, sizeof (ul) * size * chunk);
  } // dejavus (int n)
  
  ~dejavus () {
    free (p);
  }  // ~dejavus ()
  
  int next_prime (int p) {

    // static here means that there is only one cache[100] for all the
    // dejavu objects constructed.
    static struct { 
      int current, next;
    } cache[100];  // 0 to 99

    // The static int variable index is set to 0.  Since index is
    // static, this line will be executed once only.

    static int index = 0;


    // first time through we don't go through this loop, because index is 0.
    for (int i = 0; i < index; i++) {

      // cache[i].current is set 
      if (cache[i].current == p) {
	return (cache[i].next);
      }  // if (cache[i].current == p)
    } // for (int i = 0; i < index; i++)
  
    int oldp = p;
    
    // first time through it starts at 203 (first odd number processed).
    for (p = p * 2 + 1; ; p += 2) {
      int d;
      
      // see if the number is divisable by 3, then 5, then 7, then 9...
      // you only need to go up to the square root of p.
      
      for (d = 3; d * d < p; d += 2) {
	if (p % d == 0) {
	  break;
	}  // if (p % d == 0)
      }  // for (d = 3; d * d < p; d += 2)
      
      // if you get here AND p is a prime number, d is guaranteed to
      // be bigger than the square root of p.  If such is the case,
      // break out of the for loop because you've found your prime
      // number.
      
      if (d * d > p) {
	break;
      } // if (d * d > p
    }  // for (p = p * 2 + 1; ; p += 2)

    // still the first time...index is 0.  cache[0].current = oldp,
    // which should be 101.
    cache[index].current = oldp;
    cache[index].next = p;
    
    // cache[1].current = p;
    // cache[1].next = (new p);
    index++;
    
    return (p);
  } // int next_prime (int p)
  
  
  inline int dejavus::exists (unsigned long *n) {
    ul hash = 0;
    int i;
    
    // The int variable chunk was set in the subroutine Initialize in
    // emotif-maker.cc.
    
    // just a hash function.  Craig is double-hashing.
    for (i = 0; i < chunk; i ++) {
      hash ^= n[i];
    } // for (i = 0; i < chunk; i ++)
    
    hash %= size;
    
    // The unsigned long variable hash2 can never be 0.  If a
    // collision occurs, i will be set to (i + hash2) % size; size
    // should be a prime number.
    
    ul hash2 = 17 - (n[0] % 17);

    queries++;
    
    for (i = hash; ; i = (i + hash2) % size) {
      
      // n here is not the same n as in the constructor.
      if (memcmp (&p[i * chunk], n, chunk * sizeof (ul)) == 0) {
	return (1);
      }  // if (memcmp (&p[i * chunk], n, chunk * sizeof (ul)) == 0)
      
      if (memcmp (zero, &p[i * chunk], chunk * sizeof (ul)) == 0) {
	memcpy (&p[i * chunk], n, chunk * sizeof (ul));
	occupied++;
	total++;
	break;
      }  // if (memcmp (zero, &p[i * chunk], chunk * sizeof (ul)) == 0)

      // A collision must have occurred.
      collisions++;
    } // for (i = hash; ; i = (i + hash2) % size)
    
    if (occupied > size / 2) {
      total -= occupied;
      occupied = 0;
      int old_size = size;
      ul *oldp = p;
      
      used -= size;


      // size is the first prime number; next_prime will generate the
      // next prime nuber that's at least 2 x size
      size = next_prime (size);
      
      used += size;
      
      p = (ul *) malloc (sizeof (ul) * size * chunk);
      memset (p, 0, sizeof (ul) * size * chunk);
      int c = collisions;
      int q = queries;

      // populate the new hash.  
      for (i = 0; i < old_size; i++) {

	// if oldp[i * chunk] is not zero, something else is there already.
	if (memcmp (zero, &oldp[i * chunk], chunk * sizeof (ul)) != 0) {

	  // exists handles collisions.
	  exists (&oldp[i * chunk]);
	}  // if (memcmp (zero, &oldp[i * chunk], chunk * sizeof (ul)) != 0)
      }  // for (i = 0; i < old_size; i++)

      collisions = c;
      queries = q;
      // fprintf (stderr, "%d: %d, total = %d, queries %d, collisions %d\n", used, used * sizeof (ul) * chunk, total, queries, collisions);
      
      free (oldp);
    }  // if (occupied > size / 2)
      
    return (0);
  }  // inline int dejavus::exists (unsigned long *n)


  // For debugging...goes to the next non-null entry in the hash
  // table.  traverse holds the next non-null entry.

  inline ul *dejavus::next () {
    for (; traverse < size; traverse++) {
      if (memcmp (zero, &p[traverse * chunk], chunk * sizeof (ul))) {
	return (&p[traverse ++ * chunk]);
      }  // if (memcmp (zero, &p[traverse * chunk], chunk * sizeof (ul)))
    }  // for (; traverse < size; traverse++)

    return (0);
  }  // inline ul *dejavus::next ()
};  // class dejavus... why does it end with a semicolon?


// The bit array included keeps track of every sequence in the local
// multiple sequence alignment.

#define QUERY(included, bit) ((included)[(bit) / (sizeof (ul) * 8)] & (1 << ((bit) % (sizeof (ul) * 8))))
#define SET(included, bit) ((included)[(bit) / (sizeof (ul) * 8)] |= (1 << ((bit) % (sizeof (ul) * 8))))
#define CLEAR(included, bit) ((included)[(bit) / (sizeof (ul) * 8)] &= ~(1 << ((bit) % (sizeof (ul) * 8))))

