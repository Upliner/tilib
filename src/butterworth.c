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
 * A lifting implementation of the Butterworth biorthogonal wavelet filter.
 *
 * References:
 *
 * A.B. Pevnyi, V.A. Zheludev, "The Butterworth wavelet transform and its
 * implementation with the use of recursive filters," Computational mathematics
 * mathematical physics, vol. 42, no. 4, 2002, pp. 571-582.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#include <stdlib.h>
#include "../include/errcodes.h"

#define MAX(_x, _y) (_x > _y ? _x : _y)
#define MIN(_x, _y) (_x < _y ? _x : _y)
#define ROUND(_x) (((_x) < 0) ? (int) ((_x) - 0.5) : (int) ((_x) + 0.5))
#define UFIX(_x) ((_x) < 0 ? 0 : ((_x) > 255 ? 255 : (_x)))

#define GAMMA        (0.1715728752538099023966225515806)
#define ALPHA        (0.3333333333333333333333333333333)

#define NORM_FACTOR  (1.4142135623730950488016887242097)

#define LOOKAHEAD    (8)

static void F2(double *x, double *y, double *t, int len);
static void PHI3(double *x, double *y, double *t, int len);
static void filter_r3(double *x, double *y, double *t, int len);
static void filter_r2(double *x, double *y, double *t, int len);
static void decompose(double *x, double *y, int len);
static void reconstruct(double *x, double *y, int len);

static void filter_r3(double *x, double *y, double *t, int len)
{
  double init_val, pow_val;
  int i, lookahead;

  lookahead = MIN(len, LOOKAHEAD);

  init_val = x[0];
  pow_val = - ALPHA;

  for (i = 1; i <= lookahead; i++) {
    init_val += pow_val * x[i - 1];
    pow_val *= - ALPHA;
  }

  y[0] = init_val;

  for (i = 1; i < len; i++) y[i] = x[i - 1] - ALPHA * y[i - 1];

  init_val = x[len - 1];
  pow_val = - ALPHA;

  for (i = 1; i <= lookahead; i++) {
    init_val += pow_val * x[len - i];
    pow_val *= - ALPHA;
  }

  t[len - 1] = init_val;

  for (i = len - 2; i >= 0; i--) t[i] = x[i] - ALPHA * t[i + 1];

  for (i = 0; i < len - 1; i++) y[i] = (- 8.0 * t[i] - 8.0 / 9.0 * y[i] + x[i + 1] + 35.0 / 3.0 * x[i]) / 6.0;

  y[len - 1] = (- 8.0 * t[len - 1] - 8.0 / 9.0 * y[len - 1] + x[len - 1] + 35.0 / 3.0 * x[len - 1]) / 6.0;
}

static void filter_r2(double *x, double *y, double *t, int len)
{
  double init_val, pow_val;
  int i, lookahead;

  lookahead = MIN(len, LOOKAHEAD);

  init_val = x[0];
  pow_val = - GAMMA;

  for (i = 1; i <= lookahead; i++) {
    init_val += pow_val * x[i - 1];
    pow_val *= - GAMMA;
  }

  y[0] = init_val;

  for (i = 1; i < len; i++) y[i] = x[i] - GAMMA * y[i - 1];

  init_val = x[len - 1];
  pow_val = - GAMMA;

  for (i = 1; i <= lookahead; i++) {
    init_val += pow_val * x[len - i];
    pow_val *= - GAMMA;
  }

  t[len - 1] = init_val;

  for (i = len - 2; i >= 0; i--) t[i] = x[i + 1] - GAMMA * t[i + 1];

  for (i = 0; i < len; i++) y[i] = (4.0 * GAMMA / (1.0 + GAMMA)) * (y[i] + t[i]);  
}

static void F2(double *x, double *y, double *t, int len)
{
  filter_r2(x, y, t, len);
}

static void PHI3(double *x, double *y, double *t, int len)
{
  int i, n_unrol;

  filter_r3(x, y, t, len);

  n_unrol = (len - 1) & 0x00000007;

  for (i = len - 1; i > n_unrol; i -= 8) {

    y[i - 0] = 0.5 * y[i - 1];
    y[i - 1] = 0.5 * y[i - 2];
    y[i - 2] = 0.5 * y[i - 3];
    y[i - 3] = 0.5 * y[i - 4];
    y[i - 4] = 0.5 * y[i - 5];
    y[i - 5] = 0.5 * y[i - 6];
    y[i - 6] = 0.5 * y[i - 7];
    y[i - 7] = 0.5 * y[i - 8];
  }

  for (; i > 0; i--) y[i] = 0.5 * y[i - 1];

  y[0] *= 0.5;
}

static void decompose(double *x, double *y, int len)
{
  double *temp_1, *temp_2;
  double *even, *odd;
  int i, n_half, n_unrol;

  n_half = len >> 1;

  temp_1 = x;
  temp_2 = x + n_half;

  even = y;
  odd = y + n_half;

  n_unrol = n_half & 0xfffffff8;

  for (i = 0; i < n_unrol; i += 8) {

    even[i + 0] = x[(i + 0) << 1];
    even[i + 1] = x[(i + 1) << 1];
    even[i + 2] = x[(i + 2) << 1];
    even[i + 3] = x[(i + 3) << 1];
    even[i + 4] = x[(i + 4) << 1];
    even[i + 5] = x[(i + 5) << 1];
    even[i + 6] = x[(i + 6) << 1];
    even[i + 7] = x[(i + 7) << 1];

    odd[i + 0] = x[((i + 0) << 1) + 1];
    odd[i + 1] = x[((i + 1) << 1) + 1];
    odd[i + 2] = x[((i + 2) << 1) + 1];
    odd[i + 3] = x[((i + 3) << 1) + 1];
    odd[i + 4] = x[((i + 4) << 1) + 1];
    odd[i + 5] = x[((i + 5) << 1) + 1];
    odd[i + 6] = x[((i + 6) << 1) + 1];
    odd[i + 7] = x[((i + 7) << 1) + 1];
  }

  for (; i < n_half; i++) {
    even[i] = x[i << 1];
    odd[i] = x[(i << 1) + 1];
  }

  F2(even, temp_1, temp_2, n_half);

  for (i = 0; i < n_unrol; i += 8) {

    odd[i + 0] -= temp_1[i + 0];
    odd[i + 1] -= temp_1[i + 1];
    odd[i + 2] -= temp_1[i + 2];
    odd[i + 3] -= temp_1[i + 3];
    odd[i + 4] -= temp_1[i + 4];
    odd[i + 5] -= temp_1[i + 5];
    odd[i + 6] -= temp_1[i + 6];
    odd[i + 7] -= temp_1[i + 7];
  }

  for (; i < n_half; i++) odd[i] -= temp_1[i];

  PHI3(odd, temp_1, temp_2, n_half);

  for (i = 0; i < n_unrol; i += 8) {

    even[i + 0] += temp_1[i + 0];
    even[i + 1] += temp_1[i + 1];
    even[i + 2] += temp_1[i + 2];
    even[i + 3] += temp_1[i + 3];
    even[i + 4] += temp_1[i + 4];
    even[i + 5] += temp_1[i + 5];
    even[i + 6] += temp_1[i + 6];
    even[i + 7] += temp_1[i + 7];
  }

  for (; i < n_half; i++) even[i] += temp_1[i];

  for (i = 0; i < n_unrol; i += 8) {

    even[i + 0] *= NORM_FACTOR;
    even[i + 1] *= NORM_FACTOR;
    even[i + 2] *= NORM_FACTOR;
    even[i + 3] *= NORM_FACTOR;
    even[i + 4] *= NORM_FACTOR;
    even[i + 5] *= NORM_FACTOR;
    even[i + 6] *= NORM_FACTOR;
    even[i + 7] *= NORM_FACTOR;

    odd[i + 0] /= NORM_FACTOR;
    odd[i + 1] /= NORM_FACTOR;
    odd[i + 2] /= NORM_FACTOR;
    odd[i + 3] /= NORM_FACTOR;
    odd[i + 4] /= NORM_FACTOR;
    odd[i + 5] /= NORM_FACTOR;
    odd[i + 6] /= NORM_FACTOR;
    odd[i + 7] /= NORM_FACTOR;
  }

  for (; i < n_half; i++) {
    even[i] *= NORM_FACTOR;
    odd[i] /= NORM_FACTOR;
  }
}

static void reconstruct(double *x, double *y, int len)
{
  double *temp_1, *temp_2;
  double *even, *odd;
  int i, n_half, n_unrol;

  n_half = len >> 1;

  even = x;
  odd = x + n_half;

  temp_1 = y;
  temp_2 = y + n_half;

  n_unrol = n_half & 0xfffffff8;

  for (i = 0; i < n_unrol; i += 8) {

    even[i + 0] /= NORM_FACTOR;
    even[i + 1] /= NORM_FACTOR;
    even[i + 2] /= NORM_FACTOR;
    even[i + 3] /= NORM_FACTOR;
    even[i + 4] /= NORM_FACTOR;
    even[i + 5] /= NORM_FACTOR;
    even[i + 6] /= NORM_FACTOR;
    even[i + 7] /= NORM_FACTOR;

    odd[i + 0] *= NORM_FACTOR;
    odd[i + 1] *= NORM_FACTOR;
    odd[i + 2] *= NORM_FACTOR;
    odd[i + 3] *= NORM_FACTOR;
    odd[i + 4] *= NORM_FACTOR;
    odd[i + 5] *= NORM_FACTOR;
    odd[i + 6] *= NORM_FACTOR;
    odd[i + 7] *= NORM_FACTOR;
  }

  for (; i < n_half; i++) {
    even[i] /= NORM_FACTOR;
    odd[i] *= NORM_FACTOR;
  }

  PHI3(odd, temp_1, temp_2, n_half);

  for (i = 0; i < n_unrol; i += 8) {

    even[i + 0] -= temp_1[i + 0];
    even[i + 1] -= temp_1[i + 1];
    even[i + 2] -= temp_1[i + 2];
    even[i + 3] -= temp_1[i + 3];
    even[i + 4] -= temp_1[i + 4];
    even[i + 5] -= temp_1[i + 5];
    even[i + 6] -= temp_1[i + 6];
    even[i + 7] -= temp_1[i + 7];
  }

  for (; i < n_half; i++) even[i] -= temp_1[i];

  F2(even, temp_1, temp_2, n_half);

  for (i = 0; i < n_unrol; i += 8) {

    odd[i + 0] += temp_1[i + 0];
    odd[i + 1] += temp_1[i + 1];
    odd[i + 2] += temp_1[i + 2];
    odd[i + 3] += temp_1[i + 3];
    odd[i + 4] += temp_1[i + 4];
    odd[i + 5] += temp_1[i + 5];
    odd[i + 6] += temp_1[i + 6];
    odd[i + 7] += temp_1[i + 7];
  }

  for (; i < n_half; i++) odd[i] += temp_1[i];

  for (i = 0; i < n_unrol; i += 8) {

    y[((i + 0) << 1)] = even[i + 0];
    y[((i + 1) << 1)] = even[i + 1];
    y[((i + 2) << 1)] = even[i + 2];
    y[((i + 3) << 1)] = even[i + 3];
    y[((i + 4) << 1)] = even[i + 4];
    y[((i + 5) << 1)] = even[i + 5];
    y[((i + 6) << 1)] = even[i + 6];
    y[((i + 7) << 1)] = even[i + 7];

    y[((i + 0) << 1) + 1] = odd[i + 0];
    y[((i + 1) << 1) + 1] = odd[i + 1];
    y[((i + 2) << 1) + 1] = odd[i + 2];
    y[((i + 3) << 1) + 1] = odd[i + 3];
    y[((i + 4) << 1) + 1] = odd[i + 4];
    y[((i + 5) << 1) + 1] = odd[i + 5];
    y[((i + 6) << 1) + 1] = odd[i + 6];
    y[((i + 7) << 1) + 1] = odd[i + 7];
  }

  for (; i < n_half; i++) {
    y[(i << 1)] = even[i];
    y[(i << 1) + 1] = odd[i];
  }
}

int ButterworthAnalysis2D(double *image, int width, int height, int levels)
{
  double *signal_in, *signal_out, *base;
  int cur_level, cur_width, cur_height;
  int i, j, max, offs, n_samples, n_unrol;
  int result;

  max = MAX(width, height);

  signal_in = signal_out = NULL;

  signal_in = (double *) malloc(max * sizeof(double));
  signal_out = (double *) malloc(max * sizeof(double));

  if (signal_in == NULL || signal_out == NULL) {
    result =  MEMORY_ERROR;
    goto error;
  }

  n_samples = width * height;
  n_unrol = n_samples & 0xfffffff8;

  for (i = 0; i < n_unrol; i += 8) {

    image[i + 0] -= 128.0;
    image[i + 1] -= 128.0;
    image[i + 2] -= 128.0;
    image[i + 3] -= 128.0;
    image[i + 4] -= 128.0;
    image[i + 5] -= 128.0;
    image[i + 6] -= 128.0;
    image[i + 7] -= 128.0;
  }

  for (; i < n_samples; i++) image[i] -= 128.0;

  cur_width = width;
  cur_height = height;

  for (cur_level = 1; cur_level <= levels; cur_level++) {

    n_unrol = cur_width & 0xfffffff8;
    offs = 1;

    for (i = 0; i < cur_height; i++) {

      base = image + i * width;

      for (j = 0; j < n_unrol; j += 8) {

        signal_in[j + 0] = *base;
        base += offs;

        signal_in[j + 1] = *base;
        base += offs;

        signal_in[j + 2] = *base;
        base += offs;

        signal_in[j + 3] = *base;
        base += offs;

        signal_in[j + 4] = *base;
        base += offs;

        signal_in[j + 5] = *base;
        base += offs;

        signal_in[j + 6] = *base;
        base += offs;

        signal_in[j + 7] = *base;
        base += offs;
      }

      for (; j < cur_width; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      decompose(signal_in, signal_out, cur_width);

      base = image + i * width;

      for (j = 0; j < n_unrol; j += 8) {

        *base = signal_out[j + 0];
        base += offs;

        *base = signal_out[j + 1];
        base += offs;

        *base = signal_out[j + 2];
        base += offs;

        *base = signal_out[j + 3];
        base += offs;

        *base = signal_out[j + 4];
        base += offs;

        *base = signal_out[j + 5];
        base += offs;

        *base = signal_out[j + 6];
        base += offs;

        *base = signal_out[j + 7];
        base += offs;
      }

      for (; j < cur_width; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    n_unrol = cur_height & 0xfffffff8;
    offs = width;

    for (i = 0; i < cur_width; i++) {

      base = image + i;

      for (j = 0; j < n_unrol; j += 8) {

        signal_in[j + 0] = *base;
        base += offs;

        signal_in[j + 1] = *base;
        base += offs;

        signal_in[j + 2] = *base;
        base += offs;

        signal_in[j + 3] = *base;
        base += offs;

        signal_in[j + 4] = *base;
        base += offs;

        signal_in[j + 5] = *base;
        base += offs;

        signal_in[j + 6] = *base;
        base += offs;

        signal_in[j + 7] = *base;
        base += offs;
      }

      for (; j < cur_height; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      decompose(signal_in, signal_out, cur_height);

      base = image + i;

      for (j = 0; j < n_unrol; j += 8) {

        *base = signal_out[j + 0];
        base += offs;

        *base = signal_out[j + 1];
        base += offs;

        *base = signal_out[j + 2];
        base += offs;

        *base = signal_out[j + 3];
        base += offs;

        *base = signal_out[j + 4];
        base += offs;

        *base = signal_out[j + 5];
        base += offs;

        *base = signal_out[j + 6];
        base += offs;

        *base = signal_out[j + 7];
        base += offs;
      }

      for (; j < cur_height; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    cur_width >>= 1;
    cur_height >>= 1;
  }

  n_unrol = n_samples & 0xfffffff8;

  for (i = 0; i < n_unrol; i += 8) {

    image[i + 0] = ROUND(image[i + 0]);
    image[i + 1] = ROUND(image[i + 1]);
    image[i + 2] = ROUND(image[i + 2]);
    image[i + 3] = ROUND(image[i + 3]);
    image[i + 4] = ROUND(image[i + 4]);
    image[i + 5] = ROUND(image[i + 5]);
    image[i + 6] = ROUND(image[i + 6]);
    image[i + 7] = ROUND(image[i + 7]);
  }

  for (; i < n_samples; i++) image[i] = ROUND(image[i]);

  result = OK;

  error:

  free(signal_in);
  free(signal_out);

  return result;
}

int ButterworthSynthesis2D(double *image, int width, int height, int levels)
{
  double *signal_in, *signal_out, *base;
  int cur_level, cur_width, cur_height;
  int i, j, max, offs, n_samples, n_unrol;
  int result;

  max = MAX(width, height);

  signal_in = signal_out = NULL;

  signal_in = (double *) malloc(max * sizeof(double));
  signal_out = (double *) malloc(max * sizeof(double));

  if (signal_in == NULL || signal_out == NULL) {
    result = MEMORY_ERROR;
    goto error;
  }

  cur_width = width >> (levels - 1);
  cur_height = height >> (levels - 1);

  for (cur_level = 1; cur_level <= levels; cur_level++) {

    n_unrol = cur_width & 0xfffffff8;
    offs = 1;

    for (i = 0; i < cur_height; i++) {

      base = image + i * width;

      for (j = 0; j < n_unrol; j += 8) {

        signal_in[j + 0] = *base;
        base += offs;

        signal_in[j + 1] = *base;
        base += offs;

        signal_in[j + 2] = *base;
        base += offs;

        signal_in[j + 3] = *base;
        base += offs;

        signal_in[j + 4] = *base;
        base += offs;

        signal_in[j + 5] = *base;
        base += offs;

        signal_in[j + 6] = *base;
        base += offs;

        signal_in[j + 7] = *base;
        base += offs;
      }

      for (; j < cur_width; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      reconstruct(signal_in, signal_out, cur_width);

      base = image + i * width;

      for (j = 0; j < n_unrol; j += 8) {

        *base = signal_out[j + 0];
        base += offs;

        *base = signal_out[j + 1];
        base += offs;

        *base = signal_out[j + 2];
        base += offs;

        *base = signal_out[j + 3];
        base += offs;

        *base = signal_out[j + 4];
        base += offs;

        *base = signal_out[j + 5];
        base += offs;

        *base = signal_out[j + 6];
        base += offs;

        *base = signal_out[j + 7];
        base += offs;
      }

      for (; j < cur_width; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    n_unrol = cur_height & 0xfffffff8;
    offs = width;

    for (i = 0; i < cur_width; i++) {

      base = image + i;

      for (j = 0; j < n_unrol; j += 8) {

        signal_in[j + 0] = *base;
        base += offs;

        signal_in[j + 1] = *base;
        base += offs;

        signal_in[j + 2] = *base;
        base += offs;

        signal_in[j + 3] = *base;
        base += offs;

        signal_in[j + 4] = *base;
        base += offs;

        signal_in[j + 5] = *base;
        base += offs;

        signal_in[j + 6] = *base;
        base += offs;

        signal_in[j + 7] = *base;
        base += offs;
      }

      for (; j < cur_height; j++) {
        signal_in[j] = *base;
        base += offs;
      }

      reconstruct(signal_in, signal_out, cur_height);

      base = image + i;

      for (j = 0; j < n_unrol; j += 8) {

        *base = signal_out[j + 0];
        base += offs;

        *base = signal_out[j + 1];
        base += offs;

        *base = signal_out[j + 2];
        base += offs;

        *base = signal_out[j + 3];
        base += offs;

        *base = signal_out[j + 4];
        base += offs;

        *base = signal_out[j + 5];
        base += offs;

        *base = signal_out[j + 6];
        base += offs;

        *base = signal_out[j + 7];
        base += offs;
      }

      for (; j < cur_height; j++) {
        *base = signal_out[j];
        base += offs;
      }
    }

    cur_width <<= 1;
    cur_height <<= 1;
  }

  n_samples = width * height;
  n_unrol = n_samples & 0xfffffff8;

  for (i = 0; i < n_unrol; i += 8) {

    image[i + 0] = UFIX(ROUND(image[i + 0] + 128.0));
    image[i + 1] = UFIX(ROUND(image[i + 1] + 128.0));
    image[i + 2] = UFIX(ROUND(image[i + 2] + 128.0));
    image[i + 3] = UFIX(ROUND(image[i + 3] + 128.0));
    image[i + 4] = UFIX(ROUND(image[i + 4] + 128.0));
    image[i + 5] = UFIX(ROUND(image[i + 5] + 128.0));
    image[i + 6] = UFIX(ROUND(image[i + 6] + 128.0));
    image[i + 7] = UFIX(ROUND(image[i + 7] + 128.0));
  }

  for (; i < n_samples; i++) image[i] = UFIX(ROUND(image[i] + 128.0));

  result = OK;

  error:

  free(signal_in);
  free(signal_out);

  return result;
}
