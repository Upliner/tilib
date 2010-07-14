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

#ifndef DAUB97_H
#define DAUB97_H

#ifdef __cplusplus
extern "C" {
#endif

int Daub97Analysis2D(double *image, int rows, int cols, int levels);
int Daub97Synthesis2D(double *image, int rows, int cols, int levels);

#ifdef __cplusplus
}
#endif

#endif /* DAUB97_H */
