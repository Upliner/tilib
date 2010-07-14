/*
 * pbm.c
 *
 * PGM/PPM image formats support routines.
 *
 * (C) ENTROPYWARE, Alexander Simakov.
 *
 * Our site: http://www.entropyware.info/
 * Support:  support@entropyware.info
 * Author:   xander@entropyware.info
 */

#include <stdio.h>
#include <stdlib.h>
#include "../include/pbm.h"

int file_size(FILE *f)
{
  int save_pos, size_of_file;

  save_pos = ftell(f);
  fseek(f, 0L, SEEK_END);
  size_of_file = ftell(f);
  fseek(f, save_pos, SEEK_SET);
  return size_of_file;
}

/* Read next char, skipping over any comments.
 * A comment/newline sequence is returned as a newline.
 */

int get_char(FILE *f)
{
  int ch;
  
  ch = getc(f);

  if (ch == '#') {
    do {
      ch = getc(f);
    } while (ch != '\n' && ch != EOF);
  }

  return ch;
}

/* Read an unsigned decimal integer from the PGM/PPM file.
 * This function swallows one trailing character after the integer.
 * Return a value if success or -1 if fail.
 */

int get_integer(FILE *f)
{
  int ch;
  int val;
  
  do {
    ch = get_char(f);
    if (ch == EOF) return -1;
  } while ((ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r'));
  
  if ((ch < '0') || (ch > '9')) return -1;

  val = ch - '0';
  
  while ((ch = get_char(f)) >= '0' && ch <= '9') {
    val *= 10;
    val += ch - '0';
  }

  return val;
}

/* Read the file header: width, height type and component count.
 * Return 0 if success or -1 if fail.
 */

int read_hdr(FILE *f, pbm_hdr *h)
{
  if (getc(f) != 'P') return -1;

  h->type = getc(f);

  /* Only PGM and PPM (P5 and P6) formats are supported */
  if ((h->type != PGM) && (h->type != PPM)) return -1;

  h->width = get_integer(f);
  h->height = get_integer(f);
  h->max_val = get_integer(f);

  /* (width * height * pixel_size) must be equal to the image size */
  if (file_size(f) - ftell(f) != h->width * h->height * (h->type == PGM ? 1 : 3)) return -1;

  if ((h->width < 0) || (h->height < 0) || (h->max_val < 0) || (h->max_val > 255)) return -1;

  return 0;
}
