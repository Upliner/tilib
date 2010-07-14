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

#ifndef BUTTERWORTH_H
#define BUTTERWORTH_H

#ifdef __cplusplus
extern "C" {
#endif

int ButterworthAnalysis2D(double *image, int width, int height, int levels);
int ButterworthSynthesis2D(double *image, int width, int height, int levels);

#ifdef __cplusplus
}
#endif

#endif /* BUTTERWORTH_H */
