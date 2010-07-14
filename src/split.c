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
 * Routines for spliting and merging bit streams for YCbCr channels.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#define MIN(x, y) (x < y ? x : y)

void MergeChannels(unsigned char *buf,
                   unsigned char *lum,
                   unsigned char *cb,
                   unsigned char *cr,
                   int n_lum,
                   int n_cb,
                   int n_cr)
{
  int lum_pkg, cb_pkg, cr_pkg;
  int lum_div, cb_div, cr_div;
  int lum_cur, cb_cur, cr_cur;
  int lum_rem, cb_rem, cr_rem;
  int lum_tr0, cb_tr0, cr_tr0;
  int lum_tr1, cb_tr1, cr_tr1;
  int i, pkg, min;

  min = MIN(n_lum, MIN(n_cb, n_cr));

  lum_div = n_lum / min;
  lum_cur = n_lum;
  lum_rem = 0;
  lum_tr0 = lum_div * min;
  lum_tr1 = lum_tr0 + min;

  cb_div = n_cb / min;
  cb_cur = n_cb;
  cb_rem = 0;
  cb_tr0 = cb_div * min;
  cb_tr1 = cb_tr0 + min;

  cr_div = n_cr / min;
  cr_cur = n_cr;
  cr_rem = 0;
  cr_tr0 = cr_div * min;
  cr_tr1 = cr_tr0 + min;

  for (pkg = 0; pkg < min; pkg++) {

    lum_cur = n_lum + lum_rem;

    if (lum_cur >= lum_tr1) {
      lum_pkg = lum_div + 1;
      lum_rem = lum_cur - lum_tr1;
    } else {
      lum_pkg = lum_div;
      lum_rem = lum_cur - lum_tr0;
    }

    cb_cur = n_cb + cb_rem;

    if (cb_cur >= cb_tr1) {
      cb_pkg = cb_div + 1;
      cb_rem = cb_cur - cb_tr1;
    } else {
      cb_pkg = cb_div;
      cb_rem = cb_cur - cb_tr0;
    }

    cr_cur = n_cr + cr_rem;

    if (cr_cur >= cr_tr1) {
      cr_pkg = cr_div + 1;
      cr_rem = cr_cur - cr_tr1;
    } else {
      cr_pkg = cr_div;
      cr_rem = cr_cur - cr_tr0;
    }

    for (i = 0; i < lum_pkg; i++) *buf++ = *lum++;
    for (i = 0; i < cb_pkg; i++) *buf++ = *cb++;
    for (i = 0; i < cr_pkg; i++) *buf++ = *cr++;
  }
}

void SplitChannels(unsigned char *buf,
                   unsigned char *lum,
                   unsigned char *cb,
                   unsigned char *cr,
                   int n_buf,
                   int n_lum,
                   int n_cb,
                   int n_cr,
                   int *count_lum,
                   int *count_cb,
                   int *count_cr)
{
  int lum_pkg, cb_pkg, cr_pkg;
  int lum_div, cb_div, cr_div;
  int lum_cur, cb_cur, cr_cur;
  int lum_rem, cb_rem, cr_rem;
  int lum_tr0, cb_tr0, cr_tr0;
  int lum_tr1, cb_tr1, cr_tr1;
  unsigned char *pn;
  int i, pkg, min;

  min = MIN(n_lum, MIN(n_cb, n_cr));

  pn = buf + n_buf;

  *count_lum = 0;
  *count_cb = 0;
  *count_cr = 0;

  lum_div = n_lum / min;
  lum_cur = n_lum;
  lum_rem = 0;
  lum_tr0 = lum_div * min;
  lum_tr1 = lum_tr0 + min;

  cb_div = n_cb / min;
  cb_cur = n_cb;
  cb_rem = 0;
  cb_tr0 = cb_div * min;
  cb_tr1 = cb_tr0 + min;

  cr_div = n_cr / min;
  cr_cur = n_cr;
  cr_rem = 0;
  cr_tr0 = cr_div * min;
  cr_tr1 = cr_tr0 + min;

  for (pkg = 0; pkg < min; pkg++) {

    lum_cur = n_lum + lum_rem;

    if (lum_cur >= lum_tr1) {
      lum_pkg = lum_div + 1;
      lum_rem = lum_cur - lum_tr1;
    } else {
      lum_pkg = lum_div;
      lum_rem = lum_cur - lum_tr0;
    }

    cb_cur = n_cb + cb_rem;

    if (cb_cur >= cb_tr1) {
      cb_pkg = cb_div + 1;
      cb_rem = cb_cur - cb_tr1;
    } else {
      cb_pkg = cb_div;
      cb_rem = cb_cur - cb_tr0;
    }

    cr_cur = n_cr + cr_rem;

    if (cr_cur >= cr_tr1) {
      cr_pkg = cr_div + 1;
      cr_rem = cr_cur - cr_tr1;
    } else {
      cr_pkg = cr_div;
      cr_rem = cr_cur - cr_tr0;
    }

    for (i = 0; (buf < pn) && (i < lum_pkg); i++, (*count_lum)++) *lum++ = *buf++;
    for (i = 0; (buf < pn) && (i < cb_pkg); i++, (*count_cb)++) *cb++ = *buf++;
    for (i = 0; (buf < pn) && (i < cr_pkg); i++, (*count_cr)++) *cr++ = *buf++;

    if (buf >= pn) return;
  }
}
