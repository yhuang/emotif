/*
 * File:  darray.c
 * Date:  10-14-98
 * Course:  CS 107
 * Grader:  Erik Haugen
 * Programmer:  Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 * This file implements the DArray ADT.  Because DArray is really a
 * pointer to structure, and each function depends on DArray's not being
 * NULL, every exported function, except for ArrayNew, begins with an
 * assertion that the DArray cannot be NULL.  The completion of this
 * code would not have been possible without the invaluable help of Erik
 * Haugen and David Nagy-Farkas.
 * ------------------------------------------------------------
 */


#include <assert.h>
#include <ctype.h>
#include "darray.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DEFAULT_MAX 8  /*  A specification described in darray.h
			   for the default value of ArrayNew */


/*
 * struct DArrayImplementation
 * ---------------------------
 * This struct defines the underlying representation of the exported
 * type DArray.  The comments for ArrayNew explains the purpose for
 * each of struct DArrayImplementation's fields.
 */

struct DArrayImplementation {
  void *_array;
  unsigned long _elementSize, _numElements, _currentMax, _originalMax;
  ArrayElementFreeFn _liberator;
};

  
/* Private Function Prototypes */

static void DynamicExpand (DArray _arrayObject);


/* Exported Functions */

/*
 * ArrayNew
 * --------
 * The ArrayNew function creates a DArray object for the client.
 * The underlying structures of DArray are dynamically allocated on
 * the heap, and this function returns a pointer to these structures.
 * The DArray has four integer fields to keep track of the client-
 * specified parameters:  _elementSize, _numElements, _currentMax,
 * _originalMax.  The field _elementSize dictates how large each
 * element in the dynamic array will be; the field _currentMax tells
 * ArrayNew how many empty elements to allocate first; the field
 * _originalMax will be set to the value of _currentMax and not be
 * changed throughout the lifetime of the DArray object; and the field
 * _numElements is the number of elements the client has stored in the
 * DArray object.  The field _originalMax is necessary because the dynamic
 * array will be expanding by this value whenever it becomes full.  The
 * product of elementSize and currentMax is the size in bytes of the
 * initial dynamic array.  The ArrayNew function will dynamically allocate
 * this number of bytes on the heap for the client's array, but it has
 * no idea what these bytes will hold.  Finally the structure of DArrayCDT
 * also contains a pointer to a call-back free function should the
 * client's data structure require additional freeing that ArrayFree has
 * no way of knowing.
 */

DArray ArrayNew (unsigned long elementSize, unsigned long currentMax,
		 ArrayElementFreeFn liberator)
{
  DArray _arrayObject;

  assert (elementSize > 0);  /* An assert was raised as dictated by
				the darray.h file. */

  _arrayObject = (DArray) malloc (sizeof (struct DArrayImplementation));
  assert (_arrayObject != NULL);

  /* Should the client set the currentMax number of elements in the
     DArray to zero, ArrayNew will create an array with the capacity to
     handle DEFAULT_MAX number of elements (8).  Although the darray.h
     didn't specify how negative values of currentMax should be handled, 
     it did emphasize that the default capacity to be 8. Should the user
     supply a currentMax value of 0 or less, ArrayNew will set both
     the integer field _originalMax and the integer field _currentMax
     within DArray to 8. */

  if (currentMax > 0) {
    _arrayObject -> _originalMax = currentMax;
    _arrayObject -> _currentMax = currentMax;
  } else {
    _arrayObject -> _originalMax = DEFAULT_MAX;
    _arrayObject -> _currentMax = DEFAULT_MAX;
  }

  /* Every byte within this dynamic array will be initialized to zero. */

  _arrayObject -> _array = calloc (_arrayObject -> _originalMax, elementSize);
  assert (_arrayObject -> _array != NULL);

  _arrayObject -> _elementSize = elementSize;
  _arrayObject -> _numElements = 0;  /* The client has not stored anything
					in the DArray yet. */
  _arrayObject -> _liberator = liberator;  /* Client-supplied free function */

  return (_arrayObject);
}


/* 
 * ArrayClear
 * ----------
 * 
 */

void ArrayClear (DArray _arrayObject)
{
  assert (_arrayObject != NULL);

  _arrayObject -> _numElements = 0;
}


/* 
 * ArrayFree
 * ---------
 * The function ArrayFree will free every byte of memory it has allocated
 * on the heap in ArrayNew.  Because of the client-implementor barrier,
 * the ArrayFree function has no way of freeing memory whose pointers are
 * stored on the dynamic array, ArrayFree must rely on the client-supplied
 * free function to liberate the memory whose location has been stored on
 * this dynamic array.  This client-supplied free function essentially
 * provides ArrayFree with a means to interpret properly the information
 * stored in the array of void*'s.
 */

void ArrayFree (DArray _arrayObject)
{
  unsigned long i, limit, size;

  assert (_arrayObject != NULL);

  size = _arrayObject -> _elementSize;
  limit = _arrayObject -> _numElements * size;

  /* The field for storing the pointer to the client-supplied free
     function may be NULL, because the client may simply have elements
     that do not require freeing in the dynamic array. */

  if (_arrayObject -> _liberator != NULL) {
    for (i = 0; i < limit; i += size) {
      _arrayObject -> _liberator ((char*) _arrayObject -> _array + i);
    }
  }

  free (_arrayObject -> _array);
  free (_arrayObject);
}


/*
 * ArrayLength
 * -----------
 * The ArrayLength function returns the number of elements in the dynamic
 * array the client has used.  Constant time efficiency was achieved by
 * simply having a field within the underlying structure of DArray to keep
 * track of the number of used elements at any given moment in time.
 */

unsigned long ArrayLength (const DArray _arrayObject)
{
  assert (_arrayObject != NULL);

  return (_arrayObject -> _numElements);
}


/*
 * ArrayNth
 * --------
 * Because DArray keeps track of the individual element's size in the
 * dynamic array, it can return elements from the array to the client
 * without knowing anything about the type of the client's elements by
 * performing simple pointer arithmetics.  The client must remember to
 * cast the returned void* pointer to a pointer of the right type, if
 * the client wishes to retrieve data properly.  In addition the pointer
 * returned by ArrayNth becomes invalid after any calls involving
 * insertion, deletion or sorting the array, as all of these may
 * rearrange the element storage.
 */

void* ArrayNth (DArray _arrayObject, unsigned long n)
{
  assert (_arrayObject != NULL);
  assert (n >= 0 && n < _arrayObject -> _numElements);

  return ((char*) _arrayObject -> _array + n * _arrayObject -> _elementSize);
}


/*
 * ArrayAppend
 * -----------
 * ArrayAppend adds the client-supplied element to the end of the current
 * array.  Should the dynamic array become full, the private function
 * DynamicExpand will be called to increase the current length of the
 * dynamic array by the original length of the dynamic array.  After
 * the user's element has been faithfully copied onto the heap with the
 * help of the memcpy function from string.h, the field _numElements
 * within DArray will be incremented by one.
 */

void ArrayAppend (DArray _arrayObject, const void *element)
{
  unsigned long numElements, size;

  assert (_arrayObject != NULL);

  if (_arrayObject -> _numElements == _arrayObject -> _currentMax) {
    DynamicExpand (_arrayObject);
  }

  numElements = _arrayObject -> _numElements;
  size = _arrayObject -> _elementSize;

  /* The memcpy function doesn't care about the content of the element;
     all memcpy cares about is how many bytes after the base address it
     must copy onto the heap. */

  memcpy ((char*) _arrayObject -> _array + numElements * size, element, size);

  _arrayObject -> _numElements ++;
}


/*
 * ArrayInsertAt
 * -------------
 * ArrayInsert adds the client-supplied element to the position specified
 * by the client in the current array.  The original element in that
 * position, along with everything else that comes after it, will be
 * shifted down the array by one element's size in bytes.  Should the 
 * dynamic array become full, the private function DynamicExpand will
 * be called to increase the current length of the dynamic array by the
 * original length of the dynamic array.  The field _currentMax will be
 * set to the new number of elements the dynamic array can now hold.
 * After the user's element has been faithfully copied onto the heap with
 * the help of the memcpy function from string.h, the field _numElements
 * within DArray will be incremented by one.  Insertion can occur at the
 * end of the array.
 */

void ArrayInsertAt (DArray _arrayObject, const void *element,
	            unsigned long n)
{
  unsigned long size, chunkSize;

  assert (_arrayObject != NULL);
  assert (n >= 0 && n <= _arrayObject -> _numElements);

  if (_arrayObject -> _numElements == _arrayObject -> _currentMax) {
    DynamicExpand (_arrayObject);
  }

  size = _arrayObject -> _elementSize;

  /* We will be shift the entire block of elements to be moved in one
     operation using the function memmove.  The size of this block in
     bytes is calculated, and the result is stored in chunkSize. */
  
  chunkSize = (_arrayObject -> _numElements - n) * size;

  /* As long as the insertion is NOT occuring at the end of the dynamic
     array, the memmove function will be called to move memory that is
     to be pushed down the array.  If the insertion is to happen at the
     end of the current array, the ArrayInsertion function will jump to
     memcpy and faithfully copy the client's element into the array. */ 

  if (n < _arrayObject -> _numElements) {
    memmove ((char*) _arrayObject -> _array + (n + 1) * size,
	     (char*) _arrayObject -> _array + n * size, chunkSize);
  }

  memcpy ((char*) _arrayObject -> _array + n * size, element, size);

  _arrayObject -> _numElements ++;
}


/*
 * ArrayDeleteAt
 * -------------
 * The function ArrayDeleteAt will delete the client's element at the
 * specified position in the array.  If the client's element requires
 * additional freeing, the client's free function will be called.
 * After the element has been deleted, the elements that come after the
 * deleted element will be moved forward in the array by one element's
 * size in bytes.  The array will stay overallocated after this move.
 */

void ArrayDeleteAt (DArray _arrayObject, unsigned long n)
{
  unsigned long size, chunkSize;

  assert (_arrayObject != NULL);
  assert (n >= 0 && n < _arrayObject -> _numElements);

  size = _arrayObject -> _elementSize;

  if (_arrayObject -> _liberator != NULL) {
      _arrayObject -> _liberator ((char*) _arrayObject -> _array +
				  n * size);
  }

  if (n < _arrayObject -> _numElements - 1) {
    chunkSize = (_arrayObject -> _numElements - n - 1) * size;
    memmove ((char*) _arrayObject -> _array + n * size,
	     (char*) _arrayObject -> _array + (n + 1) * size, chunkSize);
  }

  _arrayObject -> _numElements --;
}


/*
 * ArrayReplaceAt
 * --------------
 * The function ArrayReplaceAt will replace one of the client's element
 * with another.  If the client's element contains pointers to the heap,
 * the client's free function will free them before the element is
 * replaced.
 */

void ArrayReplaceAt (DArray _arrayObject, const void *element,
                     unsigned long n)
{
  unsigned long size;

  assert (_arrayObject != NULL);
  assert (n >= 0 && n < _arrayObject -> _numElements);

  size = _arrayObject -> _elementSize;

  if (_arrayObject -> _liberator != NULL) {
      _arrayObject -> _liberator ((char*) _arrayObject -> _array +
				  n * size);
  }

  memcpy ((char*) _arrayObject -> _array + n * size, element, size);
}


/*
 * ArraySort
 * ---------
 * The function ArraySort will call the function qsort on the dynamic
 * array with the client-supplied comparison function.
 */

void ArraySort (DArray _arrayObject, ArrayCompareFn comparator)
{
  unsigned long numElements, size;

  assert (_arrayObject != NULL);
  assert (comparator != NULL);

  numElements = _arrayObject -> _numElements;
  size = _arrayObject -> _elementSize;

  qsort (_arrayObject -> _array, (size_t) numElements,
	 (size_t) size, comparator);
}


/*
 * ArraySearch
 * -----------
 * The function ArraySearch can search the dynamic array in two ways.
 * If the dynamic array is not sorted, ArraySearch will perform a
 * linear search for the requested element; if the dynamic array is
 * sorted, it can perform the more efficient binary tree search for the
 * requested element.  Like the ArraySort function, the ArraySearch
 * function relies on the client-supplied comparison function to tell
 * it whether or not the right element has been found.  The client may
 * specify where in the dynamic array the search is to begin.  A value
 * of zero for the parameter from means that the client wishes to search
 * the entire array.
 */

unsigned long ArraySearch (DArray _arrayObject, const void *key,
		           ArrayCompareFn comparator, unsigned long from, 
                           int isSorted)
{
  unsigned long i, size, limit, base, numElements;
  void *resultPtr;

  assert (_arrayObject != NULL);
  assert (from >= 0 && from <= _arrayObject -> _numElements);
  assert (comparator != NULL);

  size = _arrayObject -> _elementSize;
  limit = _arrayObject -> _numElements * size;
  base = from * size;
  numElements = _arrayObject -> _numElements - from;
  
  if (isSorted) {
    resultPtr = bsearch (key, (char*) _arrayObject -> _array + base,
			 (size_t) numElements, (size_t) size, comparator);

    if (resultPtr != NULL) {
      return (((char*) resultPtr - (char*) _arrayObject -> _array) / size);
    }

    return (NOT_FOUND);
  }
  
  for (i = base; i < limit; i += size) {
    if (comparator (key, (char*) _arrayObject -> _array + i) == 0) {
      return (i / size);
    }
  }

  return (NOT_FOUND);
}


/*
 * ArrayMap
 * --------
 * The ArrayMap function will call the client's mapping function on each
 * of the client's element without knowing the element's type.  It does
 * so by simply passing the base address of each element to the mapping
 * function, and the mapping function can be calculated easily, since
 * the DArray object knows how large each element in the array is.
 * Getting the base address of each element, with the size of the each
 * element in hand, becomes a simple pointer arithmetic exercise.
 */

void ArrayMap (DArray _arrayObject, ArrayMapFn mapper, void *clientData)
{
  unsigned long i, size, limit;
  char *base;

  assert (_arrayObject != NULL);
  assert (mapper != NULL);

  size = _arrayObject -> _elementSize;
  limit = _arrayObject -> _numElements * size;

  for (i = 0; i < limit; i += size) {
    base = (char*) _arrayObject -> _array;
    mapper (base + i, clientData);
  }
}


/* Private Functions */

/*
 * DynamicExpand
 * -------------
 * The private function DynamicExpand will extend the dynamic array
 * by the original length of the array, if the dynamic array ever
 * becomes filled.
 */

static void DynamicExpand (DArray _arrayObject)
{
  unsigned long size;

  size = _arrayObject -> _elementSize;
  _arrayObject -> _currentMax = _arrayObject -> _currentMax +
                                _arrayObject -> _originalMax;
  
  _arrayObject -> _array = realloc (_arrayObject -> _array, 
				    _arrayObject -> _currentMax * size);

  assert (_arrayObject -> _array != NULL);
}

