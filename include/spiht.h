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
 * Implementation of SPIHT algorithm.
 *
 * References:
 *
 * Amir Said, William A. Pearlman, "A New Fast and Efficient Image
 * Codec Based on Set Partitioning in Hierarchical Trees", IEEE Trans.
 * on Circuits and Systems for Video Technology, Vol. 6, June 1996.
 *
 * Shapiro, J. M. "Embedded image coding using zerotrees of wavelet
 * coefficients", IEEE Transactions on Signal Processing,
 * Vol. 41, No. 12 (1993), p. 3445-3462.
 *
 * C. Valens, "Embedded Zerotree Wavelet Encoding", 1999
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#ifndef SPIHT_H
#define SPIHT_H

#ifdef __cplusplus
extern "C" {
#endif

int SPIHTEncodeDWT(double *dwt_data,
                   int rows,
                   int cols,
                   int levels,
                   unsigned char *buffer,
                   int buffer_size,
                   int *stream_size);

int SPIHTDecodeDWT(double *dwt_data,
                   int rows,
                   int cols,
                   int levels,
                   unsigned char *buffer,
                   int buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SPIHT_H */
