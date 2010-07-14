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

#ifndef SPLIT_H
#define SPLIT_H

#ifdef __cplusplus
extern "C" {
#endif

void MergeChannels(unsigned char *buf,
                   unsigned char *lum,
                   unsigned char *cb,
                   unsigned char *cr,
                   int n_lum,
                   int n_cb,
                   int n_cr);

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
                   int *count_cr);

#ifdef __cplusplus
}
#endif

#endif /* SPLIT_H */
