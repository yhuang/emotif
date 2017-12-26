/*
 * File:  myppmchange.c
 * Date:  March 1, 2002
 * Package:  eMOTIF-3.6
 * Programmer:  Craig Nevill-Manning (http://craig.nevill-manning.com)
 * ------------------------------------------------------------
 * 
 * ------------------------------------------------------------
 * Copyright (c) 2002 
 * All Rights Reserved.
 * The Leland Stanford Junior University Board of Trustees
 */


#include <stdio.h>

int main (int argc, char *argv[]) {
  char line[200];
  int r, g, b;

  gets(line); puts(line);
  gets(line); puts(line);
  gets(line); puts("255");

  while (1) {
    r = getchar();
    g = getchar();
    b = getchar();

    if (feof(stdin)) break;

    if (r == 3 && g == 3 && b == 3) { r = g = b = 255; }
    if (r == 3 && g == 1 && b == 1) { r = g = 0; b = 102; }
    if (r == 1 && g == 1 && b == 3) { r = g = 102; b = 153; };
    if (r == 1 && g == 3 && b == 1) { r = g = 102; b = 153; };
    if (r == 1 && g == 3 && b == 3) { r = g = 0; b = 102; };

    putchar(r);
    putchar(g);
    putchar(b);
  }
  return 0;
}


