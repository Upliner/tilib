/*
 * The TiLib: wavelet based lossy image compression library
 * Copyright (C) 1998-2004 Alexander Simakov
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * QuikInfo:
 *
 * Image extending routines for wavelet transform.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

void ExtendImage(unsigned char *src,
                 double *dst,
                 int rows,
                 int cols,
                 int align_rows,
                 int align_cols)

{
  int pad_top, pad_bottom, pad_left, pad_right;
  unsigned char *ps;
  double *pd, *p1, *p2;
  int i, j;

  pad_top = (align_rows - rows) >> 1;
  pad_bottom = align_rows - rows - pad_top;
  pad_left = (align_cols - cols) >> 1;
  pad_right = align_cols - cols - pad_left;

  /* transfer image */
  ps = src;
  pd = dst + pad_top * align_cols + pad_left;

  for (i = 0; i < rows; i++) {

    for (j = 0; j < cols; j++) *pd++ = *ps++;
    pd += pad_right + pad_left;
  }

  /* pad left */
  p1 = dst + pad_top * align_cols + pad_left - 1;
  p2 = dst + pad_top * align_cols + pad_left;

  for (i = 0; i < rows; i++) {

    for (j = 0; j < pad_left; j++) {
      *p1-- = *p2;
      if (j < cols - 1) p2++; else p2--;
    }

    p1 += align_cols + pad_left;
    p2 = p1 + 1;
  }

  /* pad right */
  p1 = dst + pad_top * align_cols + pad_left + cols;
  p2 = dst + pad_top * align_cols + pad_left + cols - 1;

  for (i = 0; i < rows; i++) {

    for (j = 0; j < pad_right; j++) {
      *p1++ = *p2;
      if (j < cols - 1) p2--; else p2++;
    }

    p1 += pad_left + cols;
    p2 = p1 - 1;
  }

  /* pad top */
  p1 = dst + (pad_top - 1) * align_cols;
  p2 = dst + pad_top * align_cols;

  for (i = 0; i < align_cols; i++) {

    for (j = 0; j < pad_top; j++) {
      *p1 = *p2;
      p1 -= align_cols;
      if (j < rows - 1) p2 += align_cols; else p2 -= align_cols;
    }

    p1 += pad_top * align_cols + 1;
    p2 = p1 + align_cols;
  }

  /* pad bottom */
  p1 = dst + (pad_top + rows) * align_cols;
  p2 = p1 - align_cols;

  for (i = 0; i < align_cols; i++) {

    for (j = 0; j < pad_bottom; j++) {
      *p1 = *p2;
      p1 += align_cols;
      if (j < rows - 1) p2 -= align_cols; else p2 += align_cols;
    }

    p1 -= pad_bottom * align_cols - 1;
    p2 = p1 - align_cols;
  }

}

void ExtractImage(double *src,
                  unsigned char *dst,
                  int align_rows,
                  int align_cols,
                  int rows,
                  int cols)
{
  int pad_top, pad_bottom, pad_left, pad_right;
  unsigned char *pd;
  double *ps;
  int i, j;

  pad_top = (align_rows - rows) >> 1;
  pad_bottom = align_rows - rows - pad_top;
  pad_left = (align_cols - cols) >> 1;
  pad_right = align_cols - cols - pad_left;

  /* transfer image */
  ps = src + pad_top * align_cols + pad_left;
  pd = dst;

  for (i = 0; i < rows; i++) {

    for (j = 0; j < cols; j++) *pd++ = (unsigned char) *ps++;
    ps += pad_right + pad_left;
  }
}
