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
 * Top-level compression & decompression functions.
 * 
 */

#ifndef TILIB_H
#define TILIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define GRAYSCALE (0)
#define TRUECOLOR (1)

#define BUTTERWORTH (0)
#define DAUB97      (1)

int TiCompress(unsigned char *image,
               unsigned char *stream,
               int img_width,
               int img_height,
               int wavelet,
               int img_type,
               int desired_size,
               int *actual_size,
               int lum_ratio,
               int cb_ratio,
               int cr_ratio,
               int scales);

int TiCheckHeader(unsigned char *stream,
                  int *img_width,
                  int *img_height,
                  int *img_type,
                  int stream_size);

int TiDecompress(unsigned char *stream,
                 unsigned char *image,
                 int img_width,
                 int img_height,
                 int img_type,
                 int stream_size);

#ifdef __cplusplus
}
#endif

#endif /* TILIB_H */
