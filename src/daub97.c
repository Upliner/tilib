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
 * A lifting implementation of Daubechies 9/7 biorthogonal wavelet filter.
 *
 * References:
 *
 * A. Cohen, I. Daubechies, J. C. Feauveau, "Biorthogonal Bases of
 * Compactly Supported Wavelets," Communications on Pure and
 * Applied Mathematics, vol. 45, no. 5, pp. 485-560, May 1992.
 *
 * I. Daubechies and W. Sweldens, "Factoring Wavelet Transforms Into
 * Lifting Steps," J. Fourier Anal. Appl., vol. 4, no. 3, 
 * pp. 245-267, 1998.
 *
 * M. Antonini, M. Barlaud, P. Mathieu, I. Daubechies, "Image Coding Using
 * Wavelet Transform," IEEE Transactions on Image Processing, vol. 1,
 * no. 2, pp. 205-220, April 1992.
 * 
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include "../include/daub97.h"
#include "../include/errcodes.h"

#define MAX(_x, _y) (_x > _y ? _x : _y)
#define ROUND(_x) (((_x) < 0) ? (int) ((_x) - 0.5) : (int) ((_x) + 0.5))
#define FIX(_x) ((_x) < 0 ? 0 : ((_x) > 255 ? 255 : (_x)))

#define ALPHA     -1.58615986717275
#define BETA      -0.05297864003258
#define GAMMA      0.88293362717904
#define DELTA      0.44350482244527
#define EPSILON    1.14960430535816

static void Daub97Analysis1D(double *signal_in, double *signal_out, int signal_length)
{
  double *even, *odd;
  int i, half;

  for (i = 1; i < signal_length - 2; i += 2)
  signal_in[i] += ALPHA * (signal_in[i - 1] + signal_in[i + 1]);
  signal_in[signal_length - 1] += 2 * ALPHA * signal_in[signal_length - 2];

  signal_in[0] += 2 * BETA * signal_in[1];
  for (i = 2; i < signal_length; i += 2)
  signal_in[i] += BETA * (signal_in[i + 1] + signal_in[i - 1]);

  for (i = 1; i < signal_length - 2; i += 2)
  signal_in[i] += GAMMA * (signal_in[i - 1] + signal_in[i + 1]);
  signal_in[signal_length - 1] += 2 * GAMMA * signal_in[signal_length - 2];

  signal_in[0] = EPSILON * (signal_in[0] + 2 * DELTA * signal_in[1]);
  for (i = 2; i < signal_length; i += 2)
  signal_in[i] = EPSILON * (signal_in[i] + DELTA * (signal_in[i + 1] + signal_in[i - 1]));

  for (i = 1; i < signal_length; i += 2) signal_in[i] /= (-EPSILON);

  half = signal_length >> 1;

  even = signal_out;
  odd = signal_out + half;

  for (i = 0; i < half; i++) {
    even[i] = signal_in[i << 1];
    odd[i] = signal_in[(i << 1) + 1];
  }
}

static void Daub97Synthesis1D(double *signal_in, double *signal_out, int signal_length)
{
  double *even, *odd;
  int i, half;

  half = signal_length >> 1;

  even = signal_in;
  odd = signal_in + half;

  for (i = 0; i < half; i++) {
    signal_out[i << 1] = even[i];
    signal_out[(i << 1) + 1] = odd[i];
  }
 
  for (i = 1; i < signal_length; i += 2) signal_out[i] *= (-EPSILON);

  signal_out[0] = signal_out[0] / EPSILON - 2 * DELTA * signal_out[1];
  for (i = 2; i < signal_length; i += 2)
  signal_out[i] = signal_out[i] / EPSILON - DELTA * (signal_out[i + 1] + signal_out[i - 1]);

  for (i = 1; i < signal_length - 2; i += 2)
  signal_out[i] -= GAMMA * (signal_out[i - 1] + signal_out[i + 1]);
  signal_out[signal_length - 1] -= 2 * GAMMA * signal_out[signal_length - 2];

  signal_out[0] -= 2 * BETA * signal_out[1];
  for (i = 2; i < signal_length; i += 2)
  signal_out[i] -= BETA * (signal_out[i + 1] + signal_out[i - 1]);

  for (i = 1; i < signal_length - 2; i += 2)
  signal_out[i] -= ALPHA * (signal_out[i - 1] + signal_out[i + 1]);
  signal_out[signal_length - 1] -= 2 * ALPHA * signal_out[signal_length - 2];
}

int Daub97Analysis2D(double *image, int rows, int cols, int levels)
{
  double *signal_in, *signal_out, *base;
  int cur_level, cur_cols, cur_rows;
  int i, j, max, offs, n_samples;
  int err_code;

  max = MAX(cols, rows);

  signal_in = signal_out = NULL;

  signal_in = (double *) malloc(max * sizeof(double));
  signal_out = (double *) malloc(max * sizeof(double));

  if (signal_in == NULL || signal_out == NULL) {
    err_code = MEMORY_ERROR;
    goto memory_error;
  }

  n_samples = cols * rows;

  /* DC level shift */
  for (i = 0; i < n_samples; i++) image[i] -= 128.0;

  cur_cols = cols;
  cur_rows = rows;

  for (cur_level = 1; cur_level <= levels; cur_level++) {

    offs = cols;

    /* transform all columns */
    for (i = 0; i < cur_cols; i++) {

      base = image + i;

      /* load data */
      for (j = 0; j < cur_rows; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      Daub97Analysis1D(signal_in, signal_out, cur_rows);

      base = image + i;

      /* save data */
      for (j = 0; j < cur_rows; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    offs = 1;

    /* transform all rows */
    for (i = 0; i < cur_rows; i++) {

      base = image + i * cols;

      /* load data */
      for (j = 0; j < cur_cols; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      Daub97Analysis1D(signal_in, signal_out, cur_cols);

      base = image + i * cols;

      /* save data */
      for (j = 0; j < cur_cols; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    /* next scale */
    cur_cols >>= 1;
    cur_rows >>= 1;
  }

  /* uniform scalar quantinization */
  for (i = 0; i < n_samples; i++) image[i] = ROUND(image[i]);

  err_code = OK;

  memory_error:

  free(signal_in);
  free(signal_out);

  return err_code;
}

int Daub97Synthesis2D(double *image, int rows, int cols, int levels)
{
  double *signal_in, *signal_out, *base;
  int cur_level, cur_cols, cur_rows;
  int i, j, max, offs, n_samples;
  int err_code;

  max = MAX(cols, rows);

  signal_in = signal_out = NULL;

  signal_in = (double *) malloc(max * sizeof(double));
  signal_out = (double *) malloc(max * sizeof(double));

  if (signal_in == NULL || signal_out == NULL) {
    err_code = MEMORY_ERROR;
    goto memory_error;
  }

  cur_cols = cols >> (levels - 1);
  cur_rows = rows >> (levels - 1);

  for (cur_level = 1; cur_level <= levels; cur_level++) {

    offs = 1;

    /* transform all rows */
    for (i = 0; i < cur_rows; i++) {

      base = image + i * cols;

      /* load data */
      for (j = 0; j < cur_cols; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      Daub97Synthesis1D(signal_in, signal_out, cur_cols);

      base = image + i * cols;

      /* save data */
      for (j = 0; j < cur_cols; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    offs = cols;

    /* transform all columns */
    for (i = 0; i < cur_cols; i++) {

      base = image + i;

      /* load data */
      for (j = 0; j < cur_rows; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      Daub97Synthesis1D(signal_in, signal_out, cur_rows);

      base = image + i;

      /* save data */
      for (j = 0; j < cur_rows; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    /* next scale */
    cur_cols <<= 1;
    cur_rows <<= 1;
  }

  n_samples = cols * rows;

  /* undo DC level shift */
  for (i = 0; i < n_samples; i++) image[i] = FIX(ROUND(image[i] + 128.0));

  err_code = OK;

  memory_error:

  free(signal_in);
  free(signal_out);

  return err_code;
}
