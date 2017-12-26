/*
 * File:  gradient.cc
 * Date:  March 1, 2002
 * Package:  eMOTIF-3.6
 * Programmers:  Craig Nevill-Manning (http://craig.nevill-manning.com)
 *               Jimmy Y. Huang (yhuang@leland.stanford.edu)
 * ------------------------------------------------------------
 * 
 * ------------------------------------------------------------
 * Copyright (c) 2002 
 * All Rights Reserved.
 * The Leland Stanford Junior University Board of Trustees
 */


#include <stream.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

main(int argc, char **argv)
{
  assert(argc == 7);

  int x = atoi(argv[1]);
  int y = atoi(argv[2]);

  int minx = atoi(argv[3]) - 1;
  int maxx = atoi(argv[4]) - 1;
  int maxy = atoi(argv[5]);
  int miny = atoi(argv[6]);

  int nmaxx = maxx * x / (maxx - minx);
  int nminx = minx * x / (maxx - minx);
  int nmaxy = maxy * y / (maxy - miny);
  int nminy = miny * y / (maxy - miny);

  //  fprintf(stderr, "%d %d %d %d\n", nmaxx, nminx, nmaxy, nminy);

  printf("P5\n%d %d\n255\n", x, y);

  for (int i = 0; i < y; i ++)
    for (int j = 0; j < x; j ++)
      putchar(((nminy + i) * (nminx + j - 1) * 256) / (nmaxx * nmaxy));
}

