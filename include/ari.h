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
 * Arithmetic coder for SPIHT.
 *
 * References:
 *
 * I.H. Witten, R.M. Neal, and J.H. Cleary, "Arithmetic coding for data
 * compression" CACM, vol. 30, pp. 520-540, Jun. 1987.
 *
 * E-mail: xander@entropyware.info
 * Internet: http://www.entropyware.info
 */

#ifndef ARI_H
#define ARI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bitio.h"

#define CODE_BITS  (16)

#define TOP_VALUE  ((1 << CODE_BITS) - 1)

#define FIRST_QTR  (TOP_VALUE / 4 + 1)
#define HALF       (2 * FIRST_QTR)
#define THIRD_QTR  (3 * FIRST_QTR)

#define ALPHA_SIZE (2)

#define MAX_FREQ   (128)

typedef struct
{
  int low;
  int high;
  int value;

  int underflow_bits;

  int *cum_freq;

} ArithCoder;

ArithCoder *AllocArithCoder();
void FreeArithCoder(ArithCoder *arith_coder);
int BitPlusFolow(ArithCoder *arith_coder, BitStream *bit_stream, int bit);
void InitModel(ArithCoder *arith_coder);
void UpdateModel(ArithCoder *arith_coder, int symbol);
void InitEncoder(ArithCoder *arith_coder);
int EncodeSymbol(ArithCoder *a, BitStream *b, int symbol);
int DoneEncoder(ArithCoder *arith_coder, BitStream *bit_stream);
int InitDecoder(ArithCoder *arith_coder, BitStream *bit_stream);
int DecodeSymbol(ArithCoder *a, BitStream *b, int *symbol);

#ifdef __cplusplus
}
#endif

#endif /* ARI_H */
